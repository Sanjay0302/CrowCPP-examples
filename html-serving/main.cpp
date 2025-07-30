#include "crow.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>

std::string load_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "<html><body><h1>404 - File Not Found</h1></body></html>";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string get_mime_type(const std::string& filepath) {
    if (ends_with(filepath, ".html") || ends_with(filepath, ".htm")) {
        return "text/html";
    } else if (ends_with(filepath, ".css")) {
        return "text/css";
    } else if (ends_with(filepath, ".js")) {
        return "application/javascript";
    } else if (ends_with(filepath, ".png")) {
        return "image/png";
    } else if (ends_with(filepath, ".jpg") || ends_with(filepath, ".jpeg")) {
        return "image/jpeg";
    } else if (ends_with(filepath, ".gif")) {
        return "image/gif";
    }
    return "text/plain";
}

void start_http_redirect_server() {
    crow::SimpleApp http_app;
    
    // Catch all HTTP requests and redirect to HTTPS
    CROW_ROUTE(http_app, "/<path>")
    ([](const crow::request& req, const std::string& path){
        std::string host = req.get_header_value("Host");
        if (!host.empty()) {
            size_t colon_pos = host.find(':');
            if (colon_pos != std::string::npos) {
                host = host.substr(0, colon_pos);
            }
        } else {
            host = "localhost";
        }
        
        std::string redirect_url = "https://" + host + ":8443" + req.url;
        crow::response res(301);
        res.add_header("Location", redirect_url);
        return res;
    });
    
    // Root path redirection
    CROW_ROUTE(http_app, "/")
    ([](const crow::request& req){
        std::string host = req.get_header_value("Host");
        if (!host.empty()) {
            size_t colon_pos = host.find(':');
            if (colon_pos != std::string::npos) {
                host = host.substr(0, colon_pos);
            }
        } else {
            host = "localhost";
        }
        
        std::string redirect_url = "https://" + host + ":8443/";
        crow::response res(301);
        res.add_header("Location", redirect_url);
        return res;
    });
    
    std::cout << "Starting HTTP redirect server on port 8080..." << std::endl;
    http_app.port(8080).multithreaded().run();
}

int main() {
    std::cout << "HTML Server with HTTP to HTTPS redirection" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // Start HTTP redirect server in a separate thread
    std::thread http_thread(start_http_redirect_server);
    
    // Give HTTP server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Main HTTPS server
    crow::SimpleApp https_app;
    
    // API endpoint - MUST come before the catch-all route
    CROW_ROUTE(https_app, "/api/status")
    ([](){
        crow::json::wvalue response;
        response["status"] = "ok";
        response["server"] = "HTML Server with HTTPS";
        response["timestamp"] = std::time(nullptr);
        
        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
    
    // Serve static files from public directory - Root
    CROW_ROUTE(https_app, "/")
    ([](){
        std::string content = load_file("public/index.html");
        crow::response res(200, content);
        res.add_header("Content-Type", "text/html");
        return res;
    });
    
    // Serve specific static files
    CROW_ROUTE(https_app, "/style.css")
    ([](){
        std::string content = load_file("public/style.css");
        crow::response res(200, content);
        res.add_header("Content-Type", "text/css");
        return res;
    });
    
    CROW_ROUTE(https_app, "/app.js")
    ([](){
        std::string content = load_file("public/app.js");
        crow::response res(200, content);
        res.add_header("Content-Type", "application/javascript");
        return res;
    });
    
    CROW_ROUTE(https_app, "/about.html")
    ([](){
        std::string content = load_file("public/about.html");
        crow::response res(200, content);
        res.add_header("Content-Type", "text/html");
        return res;
    });
    
    // Catch-all for other files - MUST be last
    CROW_ROUTE(https_app, "/<path>")
    ([](const std::string& path){
        std::string filepath = "public/" + path;
        
        // Security check: prevent directory traversal
        if (filepath.find("..") != std::string::npos) {
            return crow::response(403, "Forbidden");
        }
        
        std::string content = load_file(filepath);
        std::string mime_type = get_mime_type(filepath);
        
        crow::response res(200, content);
        res.add_header("Content-Type", mime_type);
        return res;
    });
    
    std::cout << "Starting HTTPS server on port 8443..." << std::endl;
    std::cout << "Visit: https://localhost:8443" << std::endl;
    std::cout << "HTTP redirect available at: http://localhost:8080" << std::endl;
    
    try {
        https_app.port(8443)
                 .ssl_file("server.crt", "server.key")
                 .multithreaded()
                 .run();
    } catch (const std::exception& e) {
        std::cerr << "HTTPS server error: " << e.what() << std::endl;
        return 1;
    }
    
    // Wait for HTTP thread to finish
    http_thread.join();
    
    return 0;
}
