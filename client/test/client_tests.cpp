#include <gtest/gtest.h>
#include "../src/client/client.hpp"

using namespace https_client;

TEST(ClientTest, ConnectToServer) {
	net::io_context ioc;
	ssl::context ctx{ ssl::context::tlsv12_client };	
	
	https_client::HttpsClient client{ ioc, ctx };
	
	EXPECT_NO_THROW({     
		client.request("localhost", "4433", "/");   
		ioc.run();
		std::string response = client.get_response();		
        EXPECT_FALSE(response.empty());
    });	
}

TEST(ClientTest, HandleInvalidPath) {
	net::io_context ioc;
	ssl::context ctx{ ssl::context::tlsv12_client };	
	
	https_client::HttpsClient client{ ioc, ctx };
	
	EXPECT_NO_THROW({     
		client.request("localhost", "4433", "/nonexistent");   
		ioc.run();
		std::string response = client.get_response();		
        EXPECT_TRUE(response.find("404 Not Found") != std::string::npos);
    });	

}