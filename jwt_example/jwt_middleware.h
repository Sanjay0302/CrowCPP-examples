#ifndef JWT_MIDDLEWARE_H
#define JWT_MIDDLEWARE_H

#include "crow.h"
#include "jwt_auth.h"
#include <memory>
#include <string>

struct JWTMiddleware : crow::ILocalMiddleware
{
    struct context
    {
        JWTPayload payload;
        bool authenticated = false;
        std::string error_message;
    };

    // Default constructor required by Crow
    JWTMiddleware()
    {
        // Initialize with default authenticator
        jwt_auth = std::make_shared<JWTAuthenticator>("my-secret-key-2024", "jwt_example", "jwt_example_users");
    }

    void before_handle(crow::request &req, crow::response &res, context &ctx)
    {
        // Extract Authorization header
        std::string auth_header = req.get_header_value("Authorization");

        if (auth_header.empty())
        {
            ctx.error_message = "Missing Authorization header";
            send_unauthorized_response(res, ctx.error_message);
            return;
        }

        // Extract Bearer token
        std::string token = jwt_auth->extract_bearer_token(auth_header);
        if (token.empty())
        {
            ctx.error_message = "Invalid Authorization header format. Expected: Bearer <token>";
            send_unauthorized_response(res, ctx.error_message);
            return;
        }

        // Validate JWT token
        ValidationResult result = jwt_auth->validate_token(token);
        if (!result.valid)
        {
            ctx.error_message = result.error;
            send_unauthorized_response(res, ctx.error_message);
            return;
        }

        // Store payload in context for handler use
        ctx.payload = result.payload;
        ctx.authenticated = true;
    }

    void after_handle(crow::request & /*req*/, crow::response & /*res*/, context & /*ctx*/)
    {
        // Can be used for logging, cleanup, etc.
    }

private:
    std::shared_ptr<JWTAuthenticator> jwt_auth;

    void send_unauthorized_response(crow::response &res, const std::string &message)
    {
        crow::json::wvalue error_response;
        error_response["error"] = true;
        error_response["message"] = message;
        error_response["timestamp"] = std::time(nullptr);

        res.code = 401;
        res.body = error_response.dump();
        res.set_header("Content-Type", "application/json");
        res.end();
    }
};

// Role-based JWT Middleware for admin access
struct AdminJWTMiddleware : crow::ILocalMiddleware
{
    struct context
    {
        JWTPayload payload;
        bool authenticated = false;
        bool is_admin = false;
        std::string error_message;
    };

    // Default constructor required by Crow
    AdminJWTMiddleware()
    {
        // Initialize with default authenticator
        jwt_auth = std::make_shared<JWTAuthenticator>("my-secret-key-2024", "jwt_example", "jwt_example_users");
    }

    void before_handle(crow::request &req, crow::response &res, context &ctx)
    {
        // Extract and validate JWT (same as JWTMiddleware)
        std::string auth_header = req.get_header_value("Authorization");

        if (auth_header.empty())
        {
            ctx.error_message = "Missing Authorization header";
            send_unauthorized_response(res, ctx.error_message);
            return;
        }

        std::string token = jwt_auth->extract_bearer_token(auth_header);
        if (token.empty())
        {
            ctx.error_message = "Invalid Authorization header format";
            send_unauthorized_response(res, ctx.error_message);
            return;
        }

        ValidationResult result = jwt_auth->validate_token(token);
        if (!result.valid)
        {
            ctx.error_message = result.error;
            send_unauthorized_response(res, ctx.error_message);
            return;
        }

        ctx.payload = result.payload;
        ctx.authenticated = true;

        // Check admin role
        if (result.payload.role != "admin")
        {
            ctx.error_message = "Admin access required";
            send_forbidden_response(res, ctx.error_message);
            return;
        }

        ctx.is_admin = true;
    }

    void after_handle(crow::request & /*req*/, crow::response & /*res*/, context & /*ctx*/) {}

private:
    std::shared_ptr<JWTAuthenticator> jwt_auth;

    void send_unauthorized_response(crow::response &res, const std::string &message)
    {
        crow::json::wvalue error_response;
        error_response["error"] = true;
        error_response["message"] = message;
        error_response["timestamp"] = std::time(nullptr);

        res.code = 401;
        res.body = error_response.dump();
        res.set_header("Content-Type", "application/json");
        res.end();
    }

    void send_forbidden_response(crow::response &res, const std::string &message)
    {
        crow::json::wvalue error_response;
        error_response["error"] = true;
        error_response["message"] = message;
        error_response["timestamp"] = std::time(nullptr);

        res.code = 403;
        res.body = error_response.dump();
        res.set_header("Content-Type", "application/json");
        res.end();
    }
};

#endif // JWT_MIDDLEWARE_H
