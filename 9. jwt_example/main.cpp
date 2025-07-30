#include "crow.h"
#include "jwt_middleware.h"
#include "jwt_auth.h"
#include <memory>
#include <map>
#include <fstream>
#include <iostream>

// Simple user database
struct User
{
    std::string id;
    std::string username;
    std::string password;
    std::string role;
};

class UserDatabase
{
public:
    UserDatabase()
    {
        // Add default admin user
        users["admin"] = {"1", "admin", "adminpass123", "admin"};
        users["user"] = {"2", "user", "userpass123", "user"};
    }

    User *authenticate(const std::string &username, const std::string &password)
    {
        auto it = users.find(username);
        if (it != users.end() && it->second.password == password)
        {
            return &it->second;
        }
        return nullptr;
    }

private:
    std::map<std::string, User> users;
};

std::string load_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

int main()
{
    // Initialize JWT authenticator and user database
    auto jwt_auth = std::make_shared<JWTAuthenticator>("my-secret-key-2024", "jwt_example", "jwt_example_users");
    UserDatabase user_db;

    // Create Crow app with middlewares
    crow::App<JWTMiddleware, AdminJWTMiddleware> app;

    // Serve static files
    CROW_ROUTE(app, "/")
    ([](const crow::request &req)
     {
        std::string content = load_file("index.html");
        if (content.empty()) {
            return crow::response(404, "File not found");
        }
        crow::response res(200, content);
        res.set_header("Content-Type", "text/html");
        return res; });

    CROW_ROUTE(app, "/login.html")
    ([](const crow::request &req)
     {
        std::string content = load_file("login.html");
        if (content.empty()) {
            return crow::response(404, "File not found");
        }
        crow::response res(200, content);
        res.set_header("Content-Type", "text/html");
        return res; });

    // Login endpoint
    CROW_ROUTE(app, "/api/login").methods("POST"_method)([&](const crow::request &req)
                                                         {
        try {
            crow::json::rvalue body = crow::json::load(req.body);
            if (!body) {
                crow::json::wvalue error;
                error["error"] = true;
                error["message"] = "Invalid JSON";
                return crow::response(400, error.dump());
            }
            
            std::string username = body["username"].s();
            std::string password = body["password"].s();
            
            User* user = user_db.authenticate(username, password);
            if (!user) {
                crow::json::wvalue error;
                error["error"] = true;
                error["message"] = "Invalid credentials";
                return crow::response(401, error.dump());
            }
            
            // Generate JWT token
            std::string token = jwt_auth->generate_token(user->id, user->username, user->role);
            
            crow::json::wvalue response;
            response["success"] = true;
            response["token"] = token;
            response["user"]["id"] = user->id;
            response["user"]["username"] = user->username;
            response["user"]["role"] = user->role;
            
            crow::response res(200, response.dump());
            res.set_header("Content-Type", "application/json");
            return res;
            
        } catch (const std::exception& e) {
            crow::json::wvalue error;
            error["error"] = true;
            error["message"] = "Server error: " + std::string(e.what());
            return crow::response(500, error.dump());
        } });

    // Protected route (requires valid JWT)
    CROW_ROUTE(app, "/api/protected")
        .CROW_MIDDLEWARES(app, JWTMiddleware)([&](const crow::request &req)
                                              {
        // Access JWT payload from middleware context
        auto& ctx = app.get_context<JWTMiddleware>(req);
        
        crow::json::wvalue response;
        response["message"] = "This is protected content";
        response["user"]["id"] = ctx.payload.user_id;
        response["user"]["username"] = ctx.payload.username;
        response["user"]["role"] = ctx.payload.role;
        response["timestamp"] = std::time(nullptr);
        
        crow::response res(200, response.dump());
        res.set_header("Content-Type", "application/json");
        return res; });

    // User profile route (requires valid JWT)
    CROW_ROUTE(app, "/api/profile")
        .CROW_MIDDLEWARES(app, JWTMiddleware)([&](const crow::request &req)
                                              {
        auto& ctx = app.get_context<JWTMiddleware>(req);
        
        crow::json::wvalue profile;
        profile["id"] = ctx.payload.user_id;
        profile["username"] = ctx.payload.username;
        profile["role"] = ctx.payload.role;
        profile["token_issued_at"] = ctx.payload.iat;
        profile["token_expires_at"] = ctx.payload.exp;
        
        crow::response res(200, profile.dump());
        res.set_header("Content-Type", "application/json");
        return res; });

    // Admin-only route (requires admin JWT)
    CROW_ROUTE(app, "/api/admin")
        .CROW_MIDDLEWARES(app, AdminJWTMiddleware)([&](const crow::request &req)
                                                   {
        auto& ctx = app.get_context<AdminJWTMiddleware>(req);
        
        crow::json::wvalue admin_data;
        admin_data["message"] = "Welcome to admin panel";
        admin_data["admin"]["id"] = ctx.payload.user_id;
        admin_data["admin"]["username"] = ctx.payload.username;
        admin_data["server_status"] = "running";
        admin_data["active_users"] = 42;
        admin_data["system_health"] = "good";
        
        crow::response res(200, admin_data.dump());
        res.set_header("Content-Type", "application/json");
        return res; });

    // Admin users list (admin only)
    CROW_ROUTE(app, "/api/admin/users")
        .CROW_MIDDLEWARES(app, AdminJWTMiddleware)([&](const crow::request &req)
                                                   {
        crow::json::wvalue users_list;
        users_list["users"][0]["id"] = "1";
        users_list["users"][0]["username"] = "admin";
        users_list["users"][0]["role"] = "admin";
        users_list["users"][0]["active"] = true;
        
        users_list["users"][1]["id"] = "2";
        users_list["users"][1]["username"] = "user";
        users_list["users"][1]["role"] = "user";
        users_list["users"][1]["active"] = true;
        
        users_list["total"] = 2;
        
        crow::response res(200, users_list.dump());
        res.set_header("Content-Type", "application/json");
        return res; });

    // Unprotected route
    CROW_ROUTE(app, "/api/public")
    ([](const crow::request &req)
     {
        crow::json::wvalue response;
        response["message"] = "This is public content";
        response["timestamp"] = std::time(nullptr);
        response["server"] = "Crow JWT Example";
        
        crow::response res(200, response.dump());
        res.set_header("Content-Type", "application/json");
        return res; });

    std::cout << "JWT Example Server starting on port 18080..." << std::endl;
    std::cout << "Default users:" << std::endl;
    std::cout << "  - Username: admin, Password: adminpass123 (admin role)" << std::endl;
    std::cout << "  - Username: user, Password: userpass123 (user role)" << std::endl;
    std::cout << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  - GET  /              - Main page" << std::endl;
    std::cout << "  - GET  /login.html    - Login page" << std::endl;
    std::cout << "  - POST /api/login     - Login endpoint" << std::endl;
    std::cout << "  - GET  /api/public    - Public endpoint (no auth)" << std::endl;
    std::cout << "  - GET  /api/protected - Protected endpoint (JWT required)" << std::endl;
    std::cout << "  - GET  /api/profile   - User profile (JWT required)" << std::endl;
    std::cout << "  - GET  /api/admin     - Admin endpoint (admin JWT required)" << std::endl;
    std::cout << "  - GET  /api/admin/users - Admin users list (admin JWT required)" << std::endl;

    app.port(18080).multithreaded().run();
    return 0;
}
