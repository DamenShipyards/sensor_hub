/**
 * \file usb.h
 * \brief Provide interface to libusb-1.0
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright
 * (C) 2018 Damen Shipyards. All rights reserved.
 * \license
 * This software is proprietary. Any use without written
 * permission from the copyright holder is strictly 
 * forbidden.
 */

#ifndef USB_H_
#define USB_H_

#include <string>
#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include <libusb-1.0/libusb.h>

#include "log.h"

template <typename BufferSequence, typename Handler>
struct Operation_context {
  Operation_context() = delete;
  Operation_context(
      boost::asio::io_context& ctx, 
      const BufferSequence& buffers, 
      Handler& handler,
      size_t packet_size)
      : ctx_(ctx), 
        work_guard_(boost::asio::make_work_guard(ctx)),
        buffers_(buffers), 
        handler_(handler), 
        data_(((boost::asio::buffer_size(buffers) - 1) / packet_size + 1) * packet_size) {
    commit_write_data();
  }

  template<typename B = BufferSequence>
  typename std::enable_if<!boost::asio::is_mutable_buffer_sequence<B>::value, void>::type
  commit_write_data() {
    boost::asio::buffer_copy(boost::asio::buffer(data_), buffers_);
  }

  template<typename B = BufferSequence>
  typename std::enable_if<boost::asio::is_mutable_buffer_sequence<B>::value, void>::type
  commit_write_data() {
  }

  template<typename B = BufferSequence>
  typename std::enable_if<boost::asio::is_mutable_buffer_sequence<B>::value, void>::type
  consume_read_data(size_t len) {
    boost::asio::buffer_copy(buffers_, boost::asio::buffer(data_), len);
  }

  template<typename B = BufferSequence>
  typename std::enable_if<!boost::asio::is_mutable_buffer_sequence<B>::value, void>::type
  consume_read_data(size_t len) {
  }

  void post(boost::system::error_code& ec, size_t bytes_transferred) {
    bytes_transferred = std::min(bytes_transferred, boost::asio::buffer_size(buffers_));
    consume_read_data(bytes_transferred);
    Handler handler(handler_);
    boost::asio::post(ctx_,
        [handler{std::move(handler)}, ec, bytes_transferred]() mutable {
           handler(ec, bytes_transferred);
        }
    );
  }
  typedef std::vector<unsigned char> data_type;
  data_type& get_data() {
    return data_;
  }
private:
  boost::asio::io_context& ctx_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  data_type data_;
  BufferSequence buffers_;
  Handler handler_;
};


template <typename OperationContext>
static void handle_transfer(libusb_transfer* trnsfr) {
  OperationContext* operation_context = static_cast<OperationContext*>(trnsfr->user_data);
  boost::system::error_code ec;
  size_t bytes_transferred = static_cast<size_t>(trnsfr->actual_length);

  switch(trnsfr->status) {
    case LIBUSB_TRANSFER_COMPLETED: 
      ec = boost::system::errc::make_error_code(boost::system::errc::success);
      log(level::debug, "USB transferred % bytes", bytes_transferred);
      break;
    case LIBUSB_TRANSFER_CANCELLED:
      log(level::warning, "USB transfer cancelled");
      ec = boost::system::errc::make_error_code(boost::system::errc::operation_canceled);
      break;
    case LIBUSB_TRANSFER_NO_DEVICE:
      log(level::warning, "USB no device");
      ec = boost::system::errc::make_error_code(boost::system::errc::bad_file_descriptor);
      break;
    case LIBUSB_TRANSFER_TIMED_OUT:
      log(level::warning, "USB timeout");
      ec = boost::system::errc::make_error_code(boost::system::errc::timed_out);
      break;
    case LIBUSB_TRANSFER_ERROR:
      log(level::error, "USB transfer error");
      ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
      break;
    case LIBUSB_TRANSFER_OVERFLOW:
      log(level::error, "USB tranfer overflow");
      ec = boost::system::errc::make_error_code(boost::system::errc::value_too_large);
      break;
    case LIBUSB_TRANSFER_STALL:
      log(level::error, "USB transfer stall");
      ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
      break;
    default:
      ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
      log(level::error, "Unexpected USB error");
      break;
  }
  if (!ec && bytes_transferred == 0) {
    int r = libusb_submit_transfer(trnsfr);
    if (r != 0) {
      log(level::error, "Failed to re-submit USB transfer, error %", r);
    }
  }
  else {
    operation_context->post(ec, bytes_transferred);
    delete operation_context;
    libusb_free_transfer(trnsfr);
  }
}

struct Usb {
  Usb() = delete;
  Usb(boost::asio::io_context& io_context);
  ~Usb();
  bool open(int vendor_id, int product_id, int seq=0);
  bool open(const std::string& device_str, int seq=0);
  void close();
   
  template<typename OperationContext>
  void submit_operation(OperationContext* operation_context, int endpoint) {
    libusb_transfer* trnsfr = libusb_alloc_transfer(0);
    if (trnsfr == nullptr) {
      log(level::error, "Failed to allocate USB transfer buffer");
    }

    libusb_fill_bulk_transfer(
        trnsfr, 
        device_,
        static_cast<uint8_t>(endpoint),
        operation_context->get_data().data(),
        operation_context->get_data().size(),
        handle_transfer<OperationContext>,
        operation_context,
        500);

    int r = libusb_submit_transfer(trnsfr);
    if (r != 0) {
      log(level::error, "Failed to submit USB transfer, error %", r);
    }
  }

  // asio basic_io_object interface
  typedef boost::asio::io_context::executor_type executor_type;
  executor_type get_executor() BOOST_ASIO_NOEXCEPT {
    return io_ctx_.get_executor();
  }

  template <typename MutableBufferSequence, typename ReadHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
      void (boost::system::error_code, std::size_t))
  async_read_some(const MutableBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(ReadHandler) handler) {
    BOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;
    log(level::debug, "Doing Usb::async_read_some for % bytes", boost::asio::buffer_size(buffers));

    typedef boost::asio::async_completion<ReadHandler, void (boost::system::error_code, std::size_t)> Init;
    Init init(handler);

    typedef Operation_context<MutableBufferSequence, typename Init::completion_handler_type> Read_context;

    Read_context* read_context = new Read_context{io_ctx_, buffers, init.completion_handler, read_packet_size_};
    submit_operation(read_context, read_endpoint_);

    return init.result.get();
  }

  template <typename ConstBufferSequence, typename WriteHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler,
      void (boost::system::error_code, std::size_t))
  async_write_some(const ConstBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
  {
    BOOST_ASIO_WRITE_HANDLER_CHECK(WriteHandler, handler) type_check;
    log(level::debug, "Doing Usb::async_write_some for % bytes", boost::asio::buffer_size(buffers));

    typedef boost::asio::async_completion<WriteHandler, void (boost::system::error_code, std::size_t)> Init;
    Init init(handler);

    typedef Operation_context<ConstBufferSequence, typename Init::completion_handler_type> Write_context;

    Write_context* write_context = new Write_context{io_ctx_, buffers, init.completion_handler, 1};
    submit_operation(write_context, write_endpoint_);

    return init.result.get();
  }

private:
  boost::asio::io_context& io_ctx_;
  libusb_context* ctx_;
  libusb_device_handle* device_;
  struct Usb_descriptors;
  std::unique_ptr<Usb_descriptors> descriptors_;
  struct Usb_event_handler;
  std::unique_ptr<Usb_event_handler> event_handler_;
  int read_endpoint_, write_endpoint_;
  size_t read_packet_size_;
};

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
