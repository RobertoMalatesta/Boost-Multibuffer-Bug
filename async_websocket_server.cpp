//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast

// Major Doble changes are marked with DOBLE_ENGINEERING macro. Minor changes (such as adding a smbZsock parameter to the session class constructor a method)


//------------------------------------------------------------------------------
// beast/example/websocket/server/async/websocket_server_async.cpp
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------


#include "async_websocket_server.h" // includes threads.h
#include <iostream>

// Report a failure
void fail(boost::system::error_code ec, char const* what)
{
    assert(0);
}

// Echoes back all received WebSocket messages
session::session(tcp::socket socket, boost::asio::io_context& ioc)
        : ws_(std::move(socket))
        , strand_(ws_.get_executor())
        
    {
    }

// Start the asynchronous operation
void session::run()
{
    // Accept the websocket handshake
    ws_.async_accept(
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &session::on_accept,
                shared_from_this(),
                std::placeholders::_1)));
}

void session::on_accept(boost::system::error_code ec)
{
    if(ec)
    {
        return fail(ec, __FUNCTION__);
    }

    // Read a message
    do_read();
}

void session::do_read()
{

    // Read a message into our buffer
    ws_.async_read(
        buffer_,
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &session::handle_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2)));
}

void session::handle_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
{
    // bytes_transferred should be 0
    //  it is int rc from zsock_send_op
    boost::ignore_unused(bytes_transferred);

    if (ec) {
            fail(ec, __FUNCTION__);
    }

    //send_op <boost::beast::basic_multi_buffer<std::allocator<char>>::const_buffers_type> op(buffer_.data());
    //op();
    checkValidity(buffer_.data());
    on_read(ec, bytes_transferred);

}

template <typename ConstBufferSequence>
void checkValidity (const ConstBufferSequence & buffer)
{
    //int buf_size = boost::asio::buffer_size(buffer.data());
    const char* buf = boost::asio::buffer_cast<const char*>(boost::beast::buffers_front(buffer));
    for (int i = 0; i < 50; i++)
    {
        std::cout << i << " -> " << static_cast<int>(*(buf+i));
        printf(" mem: %p", (void*)(buf+i));
        std::cout << std::endl;
        if(i != *(buf + i)) assert(0);
    }
}


/*
* @param: args contains all of the flatbuffer information
*/
void session::on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        fail(ec, __FUNCTION__);
    }

    // Echo the message
    ws_.text(ws_.got_text());
    ws_.async_write(
        buffer_.data(),
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &session::on_write,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2)));
}


void session::on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        fail(ec, __FUNCTION__);
    }

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();

}


//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions

listener::listener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint)
        : acceptor_(ioc)
        , socket_(ioc)
        , ioc_(ioc)
        
{
    boost::system::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "acceptor_.open");
        return;
    }

    // Allow acceptor to reuse address, prevents assert(0)
    //  Otherwise, long wait time before reconnect
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "acceptor_.bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        boost::asio::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "acceptor_.listen");
        return;
    }


}

// Start accepting incoming connections
void listener::run()
{
    if(! acceptor_.is_open())
        {
            assert(0);
            return;
        }
    do_accept();
}

void listener::do_accept()
{
    acceptor_.async_accept(
        socket_,
        std::bind(
            &listener::on_accept,
            shared_from_this(),
            std::placeholders::_1));
}

void listener::on_accept(boost::system::error_code ec)
{
    if(ec)
    {
        fail(ec, __FUNCTION__);
    }else
    {
        // Create the session and run it
        std::make_shared<session>(std::move(socket_), std::ref(ioc_))->run();

    }

    // Accept another connection
    do_accept();
}


int main()
{

    auto const address = boost::asio::ip::make_address("0.0.0.0"); // Bind to all interfaces
    auto const port = static_cast<unsigned short>(5500); // WebSocket port 5500
    auto const threads = 1; // one thread;

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // Create and launch a listening port, pass a copy of the zmq socket 
    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads
    // With one thread, this will not execute
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
    {
        v.emplace_back( [&ioc]{ ioc.run(); });
    }

    ioc.run();

}

