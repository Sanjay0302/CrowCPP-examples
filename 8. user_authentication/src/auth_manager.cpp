#include "auth_manager.h"
#include <chrono>
#include <algorithm>
#include <cstring>

namespace auth {

AuthManager::AuthManager() {
    // Initialize OpenSSL
    EVP_add_digest(EVP_sha256());
}

bool AuthManager::register_user(const std::string& username, const std::string& password, const std::string& email) {
    if (username.empty() || password.empty()) {
        return false;
    }
    
    // Check password strength (minimum 8 characters, at least one digit, one letter)
    if (password.length() < 8) {
        return false;
    }
    
    bool has_digit = false, has_alpha = false;
    for (char c : password) {
        if (std::isdigit(c)) has_digit = true;
        if (std::isalpha(c)) has_alpha = true;
    }
    
    if (!has_digit || !has_alpha) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    if (users_.find(username) != users_.end()) {
        return false; // User already exists
    }
    
    auto user = std::make_shared<User>();
    user->username = username;
    user->salt = generate_salt();
    user->password_hash = hash_password(password, user->salt);
    user->email = email;
    user->is_active = true;
    user->created_at = get_current_timestamp();
    user->last_login = 0;
    
    users_[username] = user;
    return true;
}

bool AuthManager::authenticate_user(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false; // User not found
    }
    
    auto user = it->second;
    if (!user->is_active) {
        return false; // User is deactivated
    }
    
    if (verify_password(password, user->password_hash, user->salt)) {
        user->last_login = get_current_timestamp();
        return true;
    }
    
    return false;
}

bool AuthManager::user_exists(const std::string& username) const {
    std::lock_guard<std::mutex> lock(users_mutex_);
    return users_.find(username) != users_.end();
}

bool AuthManager::change_password(const std::string& username, const std::string& old_password, const std::string& new_password) {
    if (new_password.length() < 8) {
        return false;
    }
    
    bool has_digit = false, has_alpha = false;
    for (char c : new_password) {
        if (std::isdigit(c)) has_digit = true;
        if (std::isalpha(c)) has_alpha = true;
    }
    
    if (!has_digit || !has_alpha) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false; // User not found
    }
    
    auto user = it->second;
    if (!verify_password(old_password, user->password_hash, user->salt)) {
        return false; // Wrong old password
    }
    
    user->salt = generate_salt();
    user->password_hash = hash_password(new_password, user->salt);
    return true;
}

bool AuthManager::deactivate_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    it->second->is_active = false;
    return true;
}

bool AuthManager::activate_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    it->second->is_active = true;
    return true;
}

std::shared_ptr<User> AuthManager::get_user(const std::string& username) const {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return nullptr;
    }
    
    return it->second;
}

void AuthManager::update_last_login(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    
    auto it = users_.find(username);
    if (it != users_.end()) {
        it->second->last_login = get_current_timestamp();
    }
}

std::string AuthManager::generate_salt() {
    unsigned char salt_bytes[32];
    if (RAND_bytes(salt_bytes, sizeof(salt_bytes)) != 1) {
        // Fallback to time-based random (less secure)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (size_t i = 0; i < sizeof(salt_bytes); ++i) {
            salt_bytes[i] = static_cast<unsigned char>(dis(gen));
        }
    }
    
    return bytes_to_hex(salt_bytes, sizeof(salt_bytes));
}

std::string AuthManager::hash_password(const std::string& password, const std::string& salt) {
    std::string salted_password = salt + password;
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(mdctx, salted_password.c_str(), salted_password.length());
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    return bytes_to_hex(hash, hash_len);
}

bool AuthManager::verify_password(const std::string& password, const std::string& hash, const std::string& salt) {
    std::string computed_hash = hash_password(password, salt);
    return computed_hash == hash;
}

std::string AuthManager::generate_session_token(const std::string& username) {
    unsigned char token_bytes[32];
    if (RAND_bytes(token_bytes, sizeof(token_bytes)) != 1) {
        // Fallback to time-based random
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (size_t i = 0; i < sizeof(token_bytes); ++i) {
            token_bytes[i] = static_cast<unsigned char>(dis(gen));
        }
    }
    
    std::string token = bytes_to_hex(token_bytes, sizeof(token_bytes));
    
    // Store token with expiry
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    int64_t expiry = get_current_timestamp() + TOKEN_EXPIRY_SECONDS;
    session_tokens_[token] = std::make_pair(username, expiry);
    
    return token;
}

bool AuthManager::validate_session_token(const std::string& token) const {
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    
    auto it = session_tokens_.find(token);
    if (it == session_tokens_.end()) {
        return false;
    }
    
    int64_t current_time = get_current_timestamp();
    return current_time < it->second.second; // Check if token is not expired
}

void AuthManager::invalidate_session_token(const std::string& token) {
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    session_tokens_.erase(token);
}

void AuthManager::cleanup_expired_tokens() {
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    
    int64_t current_time = get_current_timestamp();
    auto it = session_tokens_.begin();
    
    while (it != session_tokens_.end()) {
        if (current_time >= it->second.second) {
            it = session_tokens_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AuthManager::bytes_to_hex(const unsigned char* bytes, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    
    return ss.str();
}

int64_t AuthManager::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
}

} // namespace auth
