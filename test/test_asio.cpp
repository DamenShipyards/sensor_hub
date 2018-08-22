#define BOOST_TEST_MODULE asio_test
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING 1
#include <boost/test/unit_test.hpp>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace asio = boost::asio;


BOOST_AUTO_TEST_CASE(buffer_test) {
  asio::streambuf b;
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
      : io_ctx_(io_ctx) {}
  typedef asio::io_context::executor_type executor_type;
  executor_type get_executor() {
    return io_ctx_.get_executor();
  }
  template <typename HandlerType>
  void thread_fun(HandlerType handler) {
    std::cout << "Entering thread_fun" << std::endl;
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
    std::cout << "Posting result" << std::endl;
    asio::post(io_ctx_, [handler]() mutable {
          boost::system::error_code ec;
          std::size_t bytes_transferred = 0;
          handler(ec, bytes_transferred);
        }
    );
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
          &Test_io::thread_fun<completion_handler_t>, 
          this, 
          init.completion_handler));

    std::cout << "Returning async_completion" << std::endl;
    return init.result.get();
  }
private:
  asio::io_context& io_ctx_;
};


void read_data(Test_io& io, asio::yield_context yield) {
  asio::streambuf b;
  std::cout << "Yielding from coro" << std::endl;
  size_t bytes_read = asio::async_read(io, b, yield);
  std::cout << "Returning to coro" << std::endl;
  asio::streambuf::const_buffers_type bufs = b.data();
  std::string str(asio::buffers_begin(bufs),
                  asio::buffers_begin(bufs) + b.size());
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
