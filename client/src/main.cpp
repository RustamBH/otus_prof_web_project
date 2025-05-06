#include "client.hpp"


int main(int argc, char* argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " <host> <port> <target>\n";
            return 1;
        }

        std::string host = argv[1];
        std::string port = argv[2];
        std::string target = argv[3];

        net::io_context ioc;
        ssl::context ctx{ ssl::context::tlsv12_client };

        // Настройка SSL контекста клиента
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(ssl::verify_none); // verify_peer

        https_client::HttpsClient client{ ioc, ctx };
        client.request(host, port, target);

        ioc.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
