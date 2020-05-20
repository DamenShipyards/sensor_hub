#ifndef SIGNALK_SERVER_H_
#define SIGNALK_SERVER_H_

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
class tcp_server;
class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_context& io_context,tcp_server* server)
  {
    return pointer(new tcp_connection(io_context, server));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start();
  void send(std::string message){
     boost::asio::async_write(socket_, boost::asio::buffer(message),
        boost::bind(&tcp_connection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  void close() {
    socket_.close();
  }

private:
  tcp_connection(boost::asio::io_context& io_context, tcp_server* server)
    : socket_(io_context), server_(server)
  {
  }

  void handle_write(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/)
  {
  }

  tcp::socket socket_;
  tcp_server* server_;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 4123)),
      stopped_(false)
  {
    start_accept();

  }
  void add_connection(boost::shared_ptr<tcp_connection> connection){
    connections_.push_back(connection);
  }
  void send(std::string delta){
    for (boost::shared_ptr<tcp_connection> connection : connections_)
    {
      connection->send(delta);
    }
  }
  void stop(){
    stopped_ = true;
    for (boost::shared_ptr<tcp_connection> connection : connections_) {
      connection->close();
    }
    acceptor_.cancel();
    acceptor_.close();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(io_context_,this);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }


  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error)
  {
        log(level::debug, "connection received");
    if (!error)
    {
      new_connection->start();
    }
    if(!stopped_) {
      start_accept();
    }
  }

  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  std::list<boost::shared_ptr<tcp_connection> > connections_;
  bool stopped_;
};


#endif // SIGNALK_SERVER_H_