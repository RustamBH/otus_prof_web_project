#pragma once

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace https_client {

    class HttpsClient {
    public:
        explicit HttpsClient(net::io_context& ioc, ssl::context& ctx);
        void request(const std::string& host, const std::string& port, const std::string& target);
        std::string get_response();

    private:
        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
        void on_handshake(beast::error_code ec);
        void on_write(beast::error_code ec, std::size_t bytes_transferred);
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void on_shutdown(beast::error_code ec);

        tcp::resolver resolver_;
        beast::ssl_stream<beast::tcp_stream> stream_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        http::response<http::string_body> res_;
    };

} // namespace https_client
