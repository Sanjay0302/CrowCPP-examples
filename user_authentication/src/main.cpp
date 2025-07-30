#include "crow.h"
#include "crow/middlewares/session.h"
#include "crow/middlewares/cookie_parser.h"
#include "auth_manager.h"
#include <fstream>
#include <thread>
#include <chrono>

auth::AuthManager auth_manager;

std::string load_html(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "File not found";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

std::string replace_placeholders(std::string html, const std::map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        std::string placeholder = "{{" + pair.first + "}}";
        size_t pos = 0;
        while ((pos = html.find(placeholder, pos)) != std::string::npos) {
            html.replace(pos, placeholder.length(), pair.second);
            pos += pair.second.length();
        }
    }
    return html;
}

void cleanup_thread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
        auth_manager.cleanup_expired_tokens();
    }
}

int main() {
    using Session = crow::SessionMiddleware<crow::InMemoryStore>;
    
    crow::App<crow::CookieParser, Session> app;

    std::thread cleanup_worker(cleanup_thread);
    cleanup_worker.detach();

    // Root redirect
    CROW_ROUTE(app, "/")
    ([&](const crow::request& req) {
        auto& session = app.get_context<Session>(req);
        std::string username = session.get("username", std::string(""));
        
        crow::response res(302);
        if (!username.empty()) {
            res.set_header("Location", "/dashboard");
        } else {
            res.set_header("Location", "/login");
        }
        return res;
    });

    // Login page
    CROW_ROUTE(app, "/login")
    ([](const crow::request& req) {
        return load_html("login.html");
    });

    // Register page
    CROW_ROUTE(app, "/register")
    ([](const crow::request& req) {
        return load_html("register.html");
    });

    // Dashboard (protected)
    CROW_ROUTE(app, "/dashboard")
    ([&](const crow::request& req) {
        auto& session = app.get_context<Session>(req);
        std::string username = session.get("username", std::string(""));
        
        if (username.empty()) {
            crow::response res(302);
            res.set_header("Location", "/login");
            return res;
        }
        
        auto user = auth_manager.get_user(username);
        if (!user) {
            crow::response res(302);
            res.set_header("Location", "/login");
            return res;
        }
        
        std::string html = load_html("dashboard.html");
        std::map<std::string, std::string> replacements = {
            {"username", user->username},
            {"email", user->email.empty() ? "Not provided" : user->email},
            {"status", user->is_active ? "Active" : "Inactive"}
        };
        
        return crow::response(replace_placeholders(html, replacements));
    });

    // API endpoints
    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([&](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "{\"success\": false, \"message\": \"Invalid JSON\"}");
        }
        
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        
        if (auth_manager.authenticate_user(username, password)) {
            auto& session = app.get_context<Session>(req);
            session.set("username", username);
            return crow::response(200, "{\"success\": true}");
        } else {
            return crow::response(401, "{\"success\": false, \"message\": \"Invalid credentials\"}");
        }
    });

    CROW_ROUTE(app, "/api/register").methods("POST"_method)
    ([&](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "{\"success\": false, \"message\": \"Invalid JSON\"}");
        }
        
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        std::string email = body.has("email") ? std::string(body["email"].s()) : std::string("");
        
        if (auth_manager.register_user(username, password, email)) {
            return crow::response(200, "{\"success\": true}");
        } else {
            return crow::response(400, "{\"success\": false, \"message\": \"Registration failed\"}");
        }
    });

    CROW_ROUTE(app, "/api/change-password").methods("POST"_method)
    ([&](const crow::request& req) {
        auto& session = app.get_context<Session>(req);
        std::string username = session.get("username", std::string(""));
        
        if (username.empty()) {
            return crow::response(401, "{\"success\": false, \"message\": \"Not authenticated\"}");
        }
        
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "{\"success\": false, \"message\": \"Invalid JSON\"}");
        }
        
        std::string currentPassword = body["currentPassword"].s();
        std::string newPassword = body["newPassword"].s();
        
        if (auth_manager.change_password(username, currentPassword, newPassword)) {
            return crow::response(200, "{\"success\": true, \"message\": \"Password changed\"}");
        } else {
            return crow::response(400, "{\"success\": false, \"message\": \"Failed to change password\"}");
        }
    });

    CROW_ROUTE(app, "/api/logout")
    ([&](const crow::request& req) {
        auto& session = app.get_context<Session>(req);
        session.remove("username");
        crow::response res(302);
        res.set_header("Location", "/login");
        return res;
    });

    app.port(18080).multithreaded().run();
    return 0;
}
