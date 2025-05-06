#include "client.hpp"


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;


namespace https_client {

    HttpsClient::HttpsClient(net::io_context& ioc, ssl::context& ctx) : resolver_(ioc), stream_(ioc, ctx) {}

    void HttpsClient::request(const std::string& host, const std::string& port, const std::string& target) {
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host.c_str())) {
            beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
            throw beast::system_error{ ec };
        }

        req_.version(11);
        req_.method(http::verb::get);
        req_.target(target);
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        resolver_.async_resolve(host, port,
            beast::bind_front_handler(&HttpsClient::on_resolve, this));
    }
    
    void HttpsClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) {
            std::cerr << "Resolve failed: " << ec.message() << "\n";
            return;
        }

        beast::get_lowest_layer(stream_).async_connect(results,
            beast::bind_front_handler(&HttpsClient::on_connect, this));
    }

    void HttpsClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if (ec) {
            std::cerr << "Connect failed: " << ec.message() << "\n";
            return;
        }

        stream_.async_handshake(ssl::stream_base::client,
            beast::bind_front_handler(&HttpsClient::on_handshake, this));
    }

    void HttpsClient::HttpsClient::on_handshake(beast::error_code ec) {
        if (ec) {
            std::cerr << "Handshake failed: " << ec.message() << "\n";
            return;
        }

        http::async_write(stream_, req_,
            beast::bind_front_handler(&HttpsClient::on_write, this));
    }

    void HttpsClient::on_write(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            std::cerr << "Write failed: " << ec.message() << "\n";
            return;
        }

        http::async_read(stream_, buffer_, res_,
            beast::bind_front_handler(&HttpsClient::on_read, this));
    }

    void HttpsClient::on_read(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            std::cerr << "Read failed: " << ec.message() << "\n";
            return;
        }

        std::cout << res_ << "\n";

        stream_.async_shutdown(
            beast::bind_front_handler(&HttpsClient::on_shutdown, this));
    }

    void HttpsClient::on_shutdown(beast::error_code ec) {
        if (ec == net::error::eof || ec == beast::errc::not_connected) {
            ec = {};
        }
        if (ec) {
            std::cerr << "Shutdown failed: " << ec.message() << "\n";
        }
    }

    std::string HttpsClient::get_response() {
        return res_.body().c_str();
    }
} // namespace https_client
