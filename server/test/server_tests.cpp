#include <gtest/gtest.h>
#include "../src/server/request_handler.hpp"

using namespace https_server;

TEST(RequestHandlerTest, HandleValidRequest) {
	net::io_context ioc;
	ssl::context ctx{ ssl::context::tlsv12_client };

	// SSL контекст
	ssl::context ctx{ ssl::context::tlsv12 };
	ctx.set_options(
		ssl::context::default_workarounds |
		ssl::context::no_sslv2 |
		ssl::context::single_dh_use);

	// Загружаем сертификат и приватный ключ
	ctx.use_certificate_chain_file("server.crt");
	ctx.use_private_key_file("server.key", ssl::context::pem);

	https_client::HttpsClient client{ ioc, ctx };
	client.request("localhost", "4433", "/index.html");
	
	EXPECT_TRUE(response.find("HTTP/1.1 200 OK") != std::string::npos);

}

TEST(RequestHandlerTest, HandleNotFound) {
	net::io_context ioc;
	ssl::context ctx{ ssl::context::tlsv12_client };

	// Настройка SSL контекста клиента
	ctx.set_default_verify_paths();
	ctx.set_verify_mode(ssl::verify_none); // verify_peer

	https_client::HttpsClient client{ ioc, ctx };
	client.request("localhost", "4433", "/nonexistent.html");
	
	EXPECT_TRUE(response.find("HTTP/1.1 404 Not Found") != std::string::npos);	
}

TEST(RequestHandlerTest, HandleDirectoryTraversal) {
	
	net::io_context ioc;
	ssl::context ctx{ ssl::context::tlsv12_client };

	// Настройка SSL контекста клиента
	ctx.set_default_verify_paths();
	ctx.set_verify_mode(ssl::verify_none); // verify_peer

	https_client::HttpsClient client{ ioc, ctx };
	client.request("localhost", "4433", "/../secret.txt HTTP/1.1");
	
	EXPECT_TRUE(response.find("HTTP/1.1 403 Forbidden") != std::string::npos);
}

TEST(RequestHandlerTest, HandleUnsupportedMethod) {
	net::io_context ioc;
	ssl::context ctx{ ssl::context::tlsv12_client };

	// Настройка SSL контекста клиента
	ctx.set_default_verify_paths();
	ctx.set_verify_mode(ssl::verify_none); // verify_peer

	https_client::HttpsClient client{ ioc, ctx };
	client.request("localhost", "4433", "POST /index.html");
	
	EXPECT_TRUE(response.find("HTTP/1.1 405 Method Not Allowed") != std::string::npos);	
}