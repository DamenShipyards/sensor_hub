/**
 * \file usb.h
 * \brief Provide interface to libusb-1.0
 *
 * \author J.R. Versteegh <j.r.versteegh@orca-st.com>
 * \copyright Copyright (C) 2019 Damen Shipyards
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef USB_H_
#define USB_H_

#include <string>
#include <memory>
#include <iostream>
#include <deque>
#include <exception>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

// Avoid warnings from libusb.h which we won't fix
#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif
#include <libusb-1.0/libusb.h>
#ifdef _MSC_VER
#pragma warning(default:4200)
#endif
#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif

#include "log.h"

using std::runtime_error;

class Usb_exception: public runtime_error {
  using runtime_error::runtime_error;
};


struct Usb_transfer {
  Usb_transfer(): transfer_(libusb_alloc_transfer(0)) {
    if (transfer_ == nullptr) {
      log(level::error, "Failed to allocate USB transfer buffer");
      throw Usb_exception("Failed to allocate USB transfer buffer");
    }
  }
  ~Usb_transfer() {
    libusb_free_transfer(transfer_);
  }
  libusb_transfer* get_transfer() {
    return transfer_;
  }
private:
  libusb_transfer* transfer_;
};

using Usb_transfer_ptr = std::unique_ptr<Usb_transfer>;


struct Transfer_queue {
  libusb_transfer* new_transfer() {
    transfers_.emplace_back(new Usb_transfer);
    return transfers_.back()->get_transfer();
  }

  void delete_transfer(libusb_transfer* usb_transfer) {
    for (auto&& transfer: transfers_) {
      if (transfer != nullptr) {
        if (usb_transfer == transfer->get_transfer()) {
          transfer = nullptr;
        }
      }
    }
    while (!transfers_.empty()) {
      if (transfers_.front() == nullptr) {
        transfers_.pop_front();
      }
      else
        break;
    }
  }

  void cancel() {
    for (auto&& transfer: transfers_) {
      if (transfer != nullptr) {
        auto tr = transfer->get_transfer();
        log(level::debug, "Cancelling transfer: %", tr);
        libusb_cancel_transfer(tr);
      }
    }
  }
private:
  std::deque<Usb_transfer_ptr> transfers_;
};


template <typename BufferSequence, typename Handler>
struct Operation_context {
  Operation_context() = delete;
  Operation_context(
      boost::asio::io_context::strand& strand,
      const BufferSequence& buffers,
      Handler& handler,
      size_t packet_size,
      Transfer_queue* transfers)
      : strand_(strand),
        work_guard_(boost::asio::make_work_guard(strand.context())),
        buffers_(buffers),
        handler_(handler),
        data_(((boost::asio::buffer_size(buffers) - 1) / packet_size + 1) * packet_size),
        transfers_(transfers) {
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
  consume_read_data(size_t) {
  }

  void post(boost::system::error_code& ec, size_t bytes_transferred, libusb_transfer* transfer) {
    bytes_transferred = std::min(bytes_transferred, boost::asio::buffer_size(buffers_));
    consume_read_data(bytes_transferred);
    Handler handler(handler_);
    Transfer_queue* transfers = transfers_;
    DEBUGLOG("Posting handler with: % on thread %", ec, boost::this_thread::get_id());
    boost::asio::post(strand_,
        [handler{std::move(handler)}, ec, bytes_transferred, transfers, transfer]() mutable {
           transfers->delete_transfer(transfer);
           DEBUGLOG("Calling handler with: % on thread %", ec, boost::this_thread::get_id());
           handler(ec, bytes_transferred);
        }
    );
  }
  typedef std::vector<unsigned char> Data_type;
  Data_type& get_data() {
    return data_;
  }
private:
  boost::asio::io_context::strand& strand_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  BufferSequence buffers_;
  Handler handler_;
  Data_type data_;
  Transfer_queue* transfers_;
};


template <typename OperationContext>
static void handle_transfer(libusb_transfer* transfer) {
  OperationContext* operation_context = static_cast<OperationContext*>(transfer->user_data);
  boost::system::error_code ec;
  size_t bytes_transferred = static_cast<size_t>(transfer->actual_length);

  switch(transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
      ec = boost::system::errc::make_error_code(boost::system::errc::success);
      DEBUGLOG("USB transferred % bytes", bytes_transferred);
      break;
    case LIBUSB_TRANSFER_CANCELLED:
      DEBUGLOG("USB transfer cancelled");
      ec = boost::system::errc::make_error_code(boost::system::errc::operation_canceled);
      break;
    case LIBUSB_TRANSFER_NO_DEVICE:
      log(level::warning, "USB no device");
      ec = boost::system::errc::make_error_code(boost::system::errc::bad_file_descriptor);
      break;
    case LIBUSB_TRANSFER_TIMED_OUT:
      log(level::info, "USB timeout");
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
    DEBUGLOG("Resubmitting transfer: %", transfer);
    int r = libusb_submit_transfer(transfer);
    if (r != 0) {
      log(level::error, "Failed to re-submit USB transfer, error %", r);
    }
  }
  else {
    DEBUGLOG("Posting transfer result: %, %", transfer, ec);
    operation_context->post(ec, bytes_transferred, transfer);
    delete operation_context;
  }
}


struct Lib_usb {
  Lib_usb();
  ~Lib_usb();
  bool open(int vendor_id, int product_id, int seq=0);
  bool open(const std::string& device_str, int seq);
  void open(const std::string& device_str);
  void close();
protected:
  auto& get_context() {
    return ctx_;
  }
  auto& get_handle() {
    return device_;
  }
  virtual void setup_endpoints() {};
  virtual void setup_events() {};
private:
  libusb_context* ctx_;
  libusb_device_handle* device_;
  struct Usb_descriptors;
  std::unique_ptr<Usb_descriptors> descriptors_;
protected:
  std::unique_ptr<Usb_descriptors>& get_descriptors();
};


struct Usb: public Lib_usb {
  Usb() = delete;
  Usb(boost::asio::io_context& io_context);
  ~Usb();

  template<typename OperationContext>
  void submit_operation(OperationContext* operation_context, int endpoint) {
    libusb_transfer* transfer = transfers_.new_transfer();

    libusb_fill_bulk_transfer(
        transfer,
        this->get_handle(),
        static_cast<uint8_t>(endpoint),
        operation_context->get_data().data(),
        static_cast<int>(operation_context->get_data().size()),
        handle_transfer<OperationContext>,
        operation_context,
        2000U);

    log(level::debug, "Submitting USB transfer: %", transfer);
    int r = libusb_submit_transfer(transfer);
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
    log(level::debug, "Doing Usb::async_read_some for % bytes", 
        boost::asio::buffer_size(buffers));

    typedef boost::asio::async_completion<ReadHandler, void (boost::system::error_code, std::size_t)> Init;
    Init init(handler);

    typedef Operation_context<MutableBufferSequence, typename Init::completion_handler_type> Read_context;

    Read_context* read_context = new Read_context{strand_, buffers, init.completion_handler, read_packet_size_, &transfers_};
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
    log(level::debug, "Doing Usb::async_write_some for % bytes", 
        boost::asio::buffer_size(buffers));

    typedef boost::asio::async_completion<WriteHandler, void (boost::system::error_code, std::size_t)> Init;
    Init init(handler);

    typedef Operation_context<ConstBufferSequence, typename Init::completion_handler_type> Write_context;

    Write_context* write_context = new Write_context{strand_, buffers, init.completion_handler, 1, &transfers_};
    submit_operation(write_context, write_endpoint_);

    return init.result.get();
  }
  void close();
  void cancel();
protected:
  void setup_endpoints() override;
  void setup_events() override;
private:
  boost::asio::io_context& io_ctx_;
  boost::asio::io_context::strand strand_;
  struct Usb_event_handler;
  std::unique_ptr<Usb_event_handler> event_handler_;
  int read_endpoint_, write_endpoint_;
  size_t read_packet_size_;
  Transfer_queue transfers_;
};


inline std::string get_usb_connection_string(const std::string& vendor_product) {
  Lib_usb usb;
  for (int i = 0; i < 4; ++i) {
    try {
       usb.open(vendor_product, i);
       usb.close();
       return fmt::format("{:s},{:d}", vendor_product, i);
    }
    catch (const Usb_exception& e){
      log(level::info, "USB device %,% not found or already connected: %", vendor_product, i, e.what());
    }
  }
  return "usb_connection_string_not_found";
}

#endif
// vim: autoindent syntax=cpp expandtab tabstop=2 softtabstop=2 shiftwidth=2
