#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <thread>
#include <vector>


// This Boost::ASIO - ZMQ implementation was developed
//  in part by Iyed Bennour
// https://github.com/iyedb/boost_asio_zeromq/blob/master/asio_zsock.cxx

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

// Echoes back all received WebSocket messages
// The Session object does the communication and
//   closes when there is no more activity
class session : public std::enable_shared_from_this<session>
{
    websocket::stream<tcp::socket> ws_;
    boost::beast::multi_buffer buffer_;
	boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;

public:

    explicit session(tcp::socket socket, boost::asio::io_context& ioc);
    void run();
    void on_accept(boost::system::error_code ec);
    void do_read();
	void handle_read(boost::system::error_code ec, std::size_t bytes_transferred);
    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);
    void on_write( boost::system::error_code ec, std::size_t bytes_transferred);

};

// Accepts websocket upgrade requests and creates
//  session objects to handle activity
// Object is not destroyed until webGW is shut down
class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;
	boost::asio::io_context& ioc_;

public:

    listener(boost::asio::io_context& ioc, tcp::endpoint endpoint);
    void run();
    void do_accept();
    void on_accept(boost::system::error_code ec);

};

template <typename ConstBufferSequence>
void checkValidity (const ConstBufferSequence & buffer);
void fail(boost::system::error_code ec, char const* what);
