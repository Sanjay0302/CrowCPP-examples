#ifndef JWT_AUTH_H
#define JWT_AUTH_H

#include <string>
#include <vector>
#include <chrono>
#include <map>

struct JWTPayload
{
    std::string user_id;
    std::string username;
    std::string role;
    std::int64_t exp; // expiration timestamp
    std::int64_t iat; // issued at timestamp
    std::string iss;  // issuer
    std::string aud;  // audience
};

struct ValidationResult
{
    bool valid;
    JWTPayload payload;
    std::string error;
};

class JWTAuthenticator
{
public:
    JWTAuthenticator(const std::string &secret, const std::string &issuer = "jwt_example",
                     const std::string &audience = "jwt_example_users");

    // Token generation
    std::string generate_token(const std::string &user_id, const std::string &username,
                               const std::string &role = "user", int expires_in_hours = 24);

    // Token validation
    ValidationResult validate_token(const std::string &token);

    // Token extraction
    std::string extract_bearer_token(const std::string &auth_header);

private:
    std::string secret_;
    std::string issuer_;
    std::string audience_;

    // Utility functions
    std::string base64_url_encode(const std::string &data);
    std::string base64_url_decode(const std::string &encoded);
    std::string hmac_sha256(const std::string &data, const std::string &key);
    std::vector<std::string> split_token(const std::string &token);
    JWTPayload parse_payload(const std::string &payload_json);
    std::string create_header();
    std::string create_payload(const std::string &user_id, const std::string &username,
                               const std::string &role, std::int64_t exp, std::int64_t iat);
    bool verify_signature(const std::string &header, const std::string &payload,
                          const std::string &signature);
};

#endif // JWT_AUTH_H
