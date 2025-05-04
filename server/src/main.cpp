#include "server.hpp"
#include <string>
#include <memory>


int main(int argc, char* argv[]) {
    try {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << " <address> <port> <doc_root> <threads>\n";
            return 1;
        }

        auto const address = net::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
        std::string doc_root = argv[3];
        auto const threads = std::max<int>(1, std::atoi(argv[4]));

        // SSL контекст
        ssl::context ctx{ ssl::context::tlsv12 };
        ctx.set_options(
            ssl::context::default_workarounds |
            ssl::context::no_sslv2 |
            ssl::context::single_dh_use);

        // Загружаем сертификат и приватный ключ
        ctx.use_certificate_chain_file("server.crt");
        ctx.use_private_key_file("server.key", ssl::context::pem);

        // IO контекст
        net::io_context ioc{ threads };

        // Создаем и запускаем сервер
        https_server::http_server server{ ioc, ctx, tcp::endpoint{address, port}, doc_root , static_cast<std::size_t>(threads) };
        std::cout << "HTTPS server started on port " << port << " with " << threads << " threads\n";
        server.start();

        // Запускаем pool потоков для обработки IO
        std::vector<std::thread> v;
        v.reserve(threads - 1);
        for (auto i = threads - 1; i > 0; --i) {
            v.emplace_back([&ioc] { ioc.run(); });
        }
        ioc.run();

        // Ждем завершения всех потоков
        for (auto& t : v) {
            t.join();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}