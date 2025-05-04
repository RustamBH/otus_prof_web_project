#include "server.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;


namespace https_server {
    
    http_connection::http_connection(tcp::socket socket, ssl::context& ctx, const std::string& doc_root, ThreadPool& thread_pool)
        : stream_(std::move(socket), ctx), doc_root_(doc_root), thread_pool_(thread_pool) {};

    void http_connection::start() {
        // Асинхронное handshake SSL
        stream_.async_handshake(
            ssl::stream_base::server,
            [self = shared_from_this()](const boost::system::error_code& error) {
                if (!error) {
                    self->read_request();
                }
            });
    }


    void http_connection::read_request() {
        auto self = shared_from_this();

        http::async_read(
            stream_,
            buffer_,
            request_,
            [this, self](beast::error_code ec, std::size_t bytes_transferred) {
                if (!ec) {
                    // Передача обработки запроса в пул потоков
                    thread_pool_.enqueue([self = shared_from_this()] {
                        self->process_request();
                        });
                }
            });
    }

    void http_connection::process_request() {
        try {
            response_.version(request_.version());
            response_.keep_alive(false);

            // Проверяем метод (поддерживаем только GET)
            if (request_.method() != http::verb::get) {
                response_.result(http::status::bad_request);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "Invalid method\n";
                return write_response();
            }

            // Безопасное формирование пути к файлу
            std::string request_path = std::string{ request_.target() };
            if (request_path.empty() || request_path[0] != '/' ||
                request_path.find("..") != std::string::npos) {
                response_.result(http::status::bad_request);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "Invalid request\n";
                return write_response();
            }

            // Если запрос заканчивается на /, добавляем index.html
            if (request_path.back() == '/') {
                request_path += "index.html";
            }

            // Формируем полный путь к файлу
            std::string full_path = doc_root_ + request_path;

            // Пытаемся открыть файл
            std::ifstream file(full_path, std::ios::in | std::ios::binary);
            if (!file) {
                response_.result(http::status::not_found);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "File not found\n";
                return write_response();
            }

            // Читаем файл и формируем ответ
            std::ostringstream file_content;
            file_content << file.rdbuf();
            response_.result(http::status::ok);

            // Определяем Content-Type по расширению файла
            std::string content_type = "text/plain";
            if (full_path.find(".html") != std::string::npos) {
                content_type = "text/html";
            }
            else if (full_path.find(".css") != std::string::npos) {
                content_type = "text/css";
            }
            else if (full_path.find(".js") != std::string::npos) {
                content_type = "application/javascript";
            }
            else if (full_path.find(".png") != std::string::npos) {
                content_type = "image/png";
            }
            else if (full_path.find(".jpg") != std::string::npos ||
                full_path.find(".jpeg") != std::string::npos) {
                content_type = "image/jpeg";
            }

            response_.set(http::field::content_type, content_type);
            beast::ostream(response_.body()) << file_content.str();
            write_response();
        }
        catch (const std::exception& e) {
            // Если файл не найден, отправляем 404            
            response_.set(http::field::server, "Boost.Asio HTTPS Server");
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body()) << "404 Not Found\n";
            write_response();
        }
    }

    void http_connection::write_response() {
        auto self = shared_from_this();

        response_.content_length(response_.body().size());

        http::async_write(
            stream_,
            response_,
            [self](beast::error_code ec, std::size_t) {
                // После отправки ответа закрываем соединение
                self->stream_.async_shutdown(
                    [self](beast::error_code ec) {
                        // Игнорируем ошибку shutdown
                        if (ec == net::error::eof || ec == ssl::error::stream_truncated) {
                            ec = {};
                        }

                        if (ec) {
                            std::cerr << "Shutdown error: " << ec.message() << std::endl;
                        }
                    });
            });
    }

    http_server::http_server(net::io_context& ioc, ssl::context& ctx, const tcp::endpoint& endpoint, const std::string& doc_root, size_t thread_pool_size)
    : acceptor_(ioc), ctx_(ctx), doc_root_(doc_root), thread_pool_(thread_pool_size) {
    beast::error_code ec;

    // Открываем acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "open: " << ec.message() << "\n";
        return;
    }

    // Устанавливаем опцию reuse address
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
            std::cerr << "set_option: " << ec.message() << "\n";
            return;
        }

        // Bind
        acceptor_.bind(endpoint, ec);
        if (ec) {
            std::cerr << "bind: " << ec.message() << "\n";
            return;
        }

        // Начинаем слушать
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) {
            std::cerr << "listen: " << ec.message() << "\n";
            return;
        }
    };

    void http_server::start() {
        if (!acceptor_.is_open()) return;
        accept();
    }

    void http_server::accept() {
        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<http_connection>(std::move(socket), ctx_, doc_root_, thread_pool_)->start();
                }
                accept();
            });
    }

} // namespace https_server