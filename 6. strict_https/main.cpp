#include <crow.h>
#include <thread>
#include <string>
#include <iostream>
#include <chrono>

// Helper function to add security headers
crow::response add_security_headers(crow::response res)
{
    res.set_header("Strict-Transport-Security", "max-age=31536000; includeSubDomains; preload");
    res.set_header("X-Frame-Options", "DENY");
    res.set_header("X-Content-Type-Options", "nosniff");
    res.set_header("X-XSS-Protection", "1; mode=block");
    return res;
}

class HTTPSRedirectApp
{
public:
    void run()
    {
        std::thread http_thread([this]()
                                {
            crow::SimpleApp http_app;

            // Catch all HTTP requests and redirect to HTTPS
            CROW_ROUTE(http_app, "/<path>")
            ([](const crow::request& req, const std::string& path) {
                crow::response res(301);
                std::string host = req.get_header_value("Host");
                if (host.empty()) {
                    host = "localhost:8443";
                } else {
                    // Replace port 8081 with 8443 for testing
                    size_t port_pos = host.find(":8081");
                    if (port_pos != std::string::npos) {
                        host = host.substr(0, port_pos) + ":8443";
                    } else if (host.find(":") == std::string::npos) {
                        host += ":8443";
                    }
                }
                res.set_header("Location", "https://" + host + "/" + path);
                res.set_header("Cache-Control", "no-cache");
                return res;
            });

            // Root redirect
            CROW_ROUTE(http_app, "/")
            ([](const crow::request& req) {
                crow::response res(301);
                std::string host = req.get_header_value("Host");
                if (host.empty()) {
                    host = "localhost:8443";
                } else {
                    // Replace port 8081 with 8443 for testing
                    size_t port_pos = host.find(":8081");
                    if (port_pos != std::string::npos) {
                        host = host.substr(0, port_pos) + ":8443";
                    } else if (host.find(":") == std::string::npos) {
                        host += ":8443";
                    }
                }
                res.set_header("Location", "https://" + host + "/");
                res.set_header("Cache-Control", "no-cache");
                return res;
            });

            std::cout << "HTTP redirect server starting on port 8081...\n";
            http_app.port(8081).multithreaded().run(); });

        // Give the HTTP server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // HTTPS server with HSTS
        crow::SimpleApp https_app;

        // HTTPS routes with security headers
        CROW_ROUTE(https_app, "/")
        ([]()
         {
            crow::response res(200, "text/html", 
                "<h1>Welcome to Secure HTTPS Server!</h1>"
                "<p>This connection is secured with HTTPS and HSTS headers.</p>"
                "<p>Try accessing this via HTTP at <a href='http://localhost:8081'>http://localhost:8081</a> - you'll be redirected here!</p>"
                "<p><a href='/api/status'>Check API Status</a> | <a href='/security-headers'>View Security Headers</a></p>");
            return add_security_headers(std::move(res)); });

        CROW_ROUTE(https_app, "/api/status")
        ([]()
         {
            crow::json::wvalue response;
            response["status"] = "secure";
            response["protocol"] = "https";
            response["hsts_enabled"] = true;
            response["port"] = 8443;
            response["redirect_port"] = 8081;
            crow::response res(200, "application/json", response.dump());
            return add_security_headers(std::move(res)); });

        CROW_ROUTE(https_app, "/security-headers")
        ([]()
         {
            crow::response res(200, "text/plain", 
                "Security Headers Information:\n"
                "- Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\n"
                "- X-Frame-Options: DENY\n"
                "- X-Content-Type-Options: nosniff\n"
                "- X-XSS-Protection: 1; mode=block\n\n"
                "These headers are automatically added to all HTTPS responses.\n");
            return add_security_headers(std::move(res)); });

        std::cout << "HTTPS server starting on port 8443 with SSL...\n";
        std::cout << "Using SSL certificates: server.crt and server.key\n";
        std::cout << "\nTesting URLs:\n";
        std::cout << "- HTTP (will redirect): http://localhost:8081\n";
        std::cout << "- HTTPS (with HSTS): https://localhost:8443\n";

        https_app.port(8443)
            .ssl_file("server.crt", "server.key")
            .multithreaded()
            .run();

        http_thread.join();
    }
};

int main()
{
    HTTPSRedirectApp app;
    app.run();
    return 0;
}
