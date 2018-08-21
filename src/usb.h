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
#include <boost/asio.hpp>
#include <libusb-1.0/libusb.h>

struct Usb {
  Usb() = delete;
  Usb(boost::asio::io_context& io_context);
  ~Usb();
  bool open(int vendor_id, int product_id, int seq=0);
  bool open(const std::string& device_str, int seq=0);
  void close();
  bool read();

  typedef boost::asio::io_context::executor_type executor_type;
  executor_type get_executor() BOOST_ASIO_NOEXCEPT {
    return io_ctx_.get_executor();
  }

  template <typename MutableBufferSequence, typename ReadHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
      void (boost::system::error_code, std::size_t))
  async_read_some(const MutableBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(ReadHandler) handler) {
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a ReadHandler.
    BOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

    boost::asio::async_completion<ReadHandler,
      void (boost::system::error_code, std::size_t)> init(handler);

    //this->get_service().async_read_some(
    //this->get_implementation(), buffers, init.completion_handler);

    return init.result.get();
  }


private:
  boost::asio::io_context& io_ctx_;
  libusb_context* ctx_;
  libusb_device_handle* device_;
  struct Usb_descriptors;
  std::unique_ptr<Usb_descriptors> descriptors_;
};

#endif
