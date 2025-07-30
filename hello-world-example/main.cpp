#include "crow.h"
#include <string>

int main()
{
    crow::SimpleApp app;

    // Pre-allocate response string to avoid repeated allocations
    static const std::string response = "hello world";

    CROW_ROUTE(app, "/")
    ([]() -> const char *
     { return "hello world"; });

    // Optimize server settings for performance
    app.port(8081)
        .multithreaded()
        .concurrency(8)  // Increase thread count
        .server_name("") // Remove server name header overhead
        .run();

    return 0;
}
