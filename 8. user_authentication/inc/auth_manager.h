#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <random>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace auth {

struct User {
    std::string username;
    std::string password_hash;
    std::string salt;
    std::string email;
    bool is_active;
    int64_t created_at;
    int64_t last_login;
};

class AuthManager {
public:
    AuthManager();
    ~AuthManager() = default;

    // User management
    bool register_user(const std::string& username, const std::string& password, const std::string& email = "");
    bool authenticate_user(const std::string& username, const std::string& password);
    bool user_exists(const std::string& username) const;
    bool change_password(const std::string& username, const std::string& old_password, const std::string& new_password);
    bool deactivate_user(const std::string& username);
    bool activate_user(const std::string& username);
    
    // User info
    std::shared_ptr<User> get_user(const std::string& username) const;
    void update_last_login(const std::string& username);

    // Password security
    static std::string generate_salt();
    static std::string hash_password(const std::string& password, const std::string& salt);
    static bool verify_password(const std::string& password, const std::string& hash, const std::string& salt);

    // Session token management
    std::string generate_session_token(const std::string& username);
    bool validate_session_token(const std::string& token) const;
    void invalidate_session_token(const std::string& token);
    void cleanup_expired_tokens();

private:
    mutable std::mutex users_mutex_;
    mutable std::mutex tokens_mutex_;
    std::unordered_map<std::string, std::shared_ptr<User>> users_;
    std::unordered_map<std::string, std::pair<std::string, int64_t>> session_tokens_; // token -> (username, expiry)
    
    // Token expiry time in seconds (default: 24 hours)
    static constexpr int64_t TOKEN_EXPIRY_SECONDS = 24 * 60 * 60;
    
    // Helper methods
    static std::string bytes_to_hex(const unsigned char* bytes, size_t length);
    static int64_t get_current_timestamp();
};

} // namespace auth
