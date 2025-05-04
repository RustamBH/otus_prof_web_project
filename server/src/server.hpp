#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

namespace https_server {
    // Пул потоков для обработки соединений
    class ThreadPool {
    public:
        explicit ThreadPool(size_t threads) : stop(false) {
            for (size_t i = 0; i < threads; ++i) {
                workers.emplace_back([this] {
                    for (;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                    });
            }
        }

        template<class F>
        void enqueue(F&& f) {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (stop)
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                tasks.emplace(std::forward<F>(f));
            }
            condition.notify_one();
        }

        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                stop = true;
            }
            condition.notify_all();
            for (std::thread& worker : workers)
                worker.join();
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
    };

    // Класс для обработки HTTP соединений
    class http_connection : public std::enable_shared_from_this<http_connection> {
    public:
        http_connection(tcp::socket socket, ssl::context& ctx, const std::string& doc_root, ThreadPool& thread_pool);
        void start();
    private:
        beast::ssl_stream<tcp::socket> stream_;
        beast::flat_buffer buffer_{ 8192 };
        http::request<http::dynamic_body> request_;
        http::response<http::dynamic_body> response_;
        std::string doc_root_;
        ThreadPool& thread_pool_;

        void read_request();
        void process_request();
        void write_response();
    };

    // Класс сервера
    class http_server {
    public:
        http_server(net::io_context& ioc, ssl::context& ctx,
            const tcp::endpoint& endpoint, const std::string& doc_root, size_t thread_pool_size);
        void start();

    private:
        tcp::acceptor acceptor_;
        ssl::context& ctx_;
        std::string doc_root_;
        ThreadPool thread_pool_;

        void accept();
    };

} // namespace https_server