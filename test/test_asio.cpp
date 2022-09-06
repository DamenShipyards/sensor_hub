#define BOOST_TEST_MODULE asio_test
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#include <boost/test/unit_test.hpp>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace asio = boost::asio;
using namespace boost::placeholders;

BOOST_AUTO_TEST_CASE(buffer_test) {
  asio::streambuf b;
  auto mut = b.prepare(13);
  BOOST_TEST(mut.size() == 13U);
  b.commit(13);
  BOOST_TEST(b.size() == 13U);
  b.consume(13);
  BOOST_TEST(b.size() == 0U);
  std::ostream os(&b);
  os << "Hello, World!\nThis is me.";
  b.consume(7);
  std::string str;
  std::istream is(&b);
  is >> str;
  BOOST_TEST(str == "World!");
  std::stringbuf sb;
  is >> std::skipws >> &sb;
  BOOST_TEST(sb.str() == "This is me.");
}


struct Test_io {
  Test_io() = delete;
  Test_io(asio::io_context& io_ctx)
      : io_ctx_(io_ctx), work_guard_(asio::make_work_guard(io_ctx_)) {
  }
  typedef asio::io_context::executor_type executor_type;
  executor_type get_executor() {
    return io_ctx_.get_executor();
  }
  template <typename HandlerType, typename MutableBufferSequence>
  void thread_fun(HandlerType handler, const MutableBufferSequence& buffers) {
    std::cout << "Entering thread_fun" << std::endl;
    std::cout << "Thread fun thread id: " << boost::this_thread::get_id() << std::endl;
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
    std::cout << "Posting result" << std::endl;
    // This is our fake input data that we 'got' after one second of waiting
    asio::buffer_copy(buffers, asio::buffer("Hello, World!"));
    auto bufsize = buffers.size();
    asio::post(io_ctx_, [handler, bufsize]() mutable {
          boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::success);
          std::cout << "Completion handler thread id: " << boost::this_thread::get_id() << std::endl;
          handler(ec, bufsize);
        }
    );
    work_guard_.reset();
  }
  template <typename MutableBufferSequence, typename ReadHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
      void (boost::system::error_code, std::size_t))
  async_read_some(const MutableBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(ReadHandler) handler) {
    std::cout << "async_read_some called!" << std::endl;
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a ReadHandler.
    BOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

    boost::asio::async_completion<ReadHandler,
      void (boost::system::error_code, std::size_t)> init(handler);

    typedef typename boost::asio::async_completion<ReadHandler,
            void (boost::system::error_code, std::size_t)>::completion_handler_type completion_handler_t;

    std::cout << "Starting completion thread" << std::endl;
    boost::thread thr(boost::bind(
          &Test_io::thread_fun<completion_handler_t, MutableBufferSequence>,
          this,
          init.completion_handler,
          buffers));

    std::cout << "Returning async_completion" << std::endl;
    return init.result.get();
  }
private:
  asio::io_context& io_ctx_;
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
};


void read_data(Test_io& io, asio::yield_context yield) {
  asio::streambuf buffer;
  std::string expected = "Hello, World!";
  std::cout << "Yielding from coro" << std::endl;
  std::cout << "Coro thread id: " << boost::this_thread::get_id() << std::endl;
  size_t bytes_read = asio::async_read(io, buffer.prepare(expected.size()), yield);
  std::cout << "Returning to coro with: " << bytes_read << std::endl;
  std::cout << "Coro thread id: " << boost::this_thread::get_id() << std::endl;
  BOOST_TEST(bytes_read == expected.size());
  buffer.commit(bytes_read);
  asio::streambuf::const_buffers_type bufs = buffer.data();
  std::string str(asio::buffers_begin(bufs),
                  asio::buffers_begin(bufs) + buffer.size());
  std::cout << str << std::endl;
}


BOOST_AUTO_TEST_CASE(completion_from_thread_test) {
  asio::io_context io_ctx;
  Test_io test_io{io_ctx};
  asio::spawn(io_ctx, boost::bind(read_data, boost::ref(test_io), _1));
  std::cout << "Running io context" << std::endl;

  io_ctx.run();
  std::cout << "Done running io context" << std::endl;
}
