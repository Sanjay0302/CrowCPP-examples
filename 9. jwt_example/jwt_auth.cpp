#include "jwt_auth.h"
#include "crow.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

JWTAuthenticator::JWTAuthenticator(const std::string &secret, const std::string &issuer,
                                   const std::string &audience)
    : secret_(secret), issuer_(issuer), audience_(audience) {}

std::string JWTAuthenticator::generate_token(const std::string &user_id, const std::string &username,
                                             const std::string &role, int expires_in_hours)
{
    auto now = std::chrono::system_clock::now();
    auto iat = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    auto exp = iat + (expires_in_hours * 3600);

    std::string header = create_header();
    std::string payload = create_payload(user_id, username, role, exp, iat);

    std::string header_b64 = base64_url_encode(header);
    std::string payload_b64 = base64_url_encode(payload);

    std::string signing_input = header_b64 + "." + payload_b64;
    std::string signature = hmac_sha256(signing_input, secret_);
    std::string signature_b64 = base64_url_encode(signature);

    return header_b64 + "." + payload_b64 + "." + signature_b64;
}

ValidationResult JWTAuthenticator::validate_token(const std::string &token)
{
    ValidationResult result;
    result.valid = false;

    auto parts = split_token(token);
    if (parts.size() != 3)
    {
        result.error = "Invalid token format";
        return result;
    }

    try
    {
        // Verify signature
        if (!verify_signature(parts[0], parts[1], parts[2]))
        {
            result.error = "Invalid signature";
            return result;
        }

        // Decode and parse payload
        std::string payload_json = base64_url_decode(parts[1]);
        result.payload = parse_payload(payload_json);

        // Check expiration
        auto now = std::chrono::system_clock::now();
        auto current_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        if (current_time > result.payload.exp)
        {
            result.error = "Token expired";
            return result;
        }

        // Check issuer
        if (result.payload.iss != issuer_)
        {
            result.error = "Invalid issuer";
            return result;
        }

        // Check audience
        if (result.payload.aud != audience_)
        {
            result.error = "Invalid audience";
            return result;
        }

        result.valid = true;
    }
    catch (const std::exception &e)
    {
        result.error = "Token parsing error: " + std::string(e.what());
    }

    return result;
}

std::string JWTAuthenticator::extract_bearer_token(const std::string &auth_header)
{
    if (auth_header.length() > 7 && auth_header.substr(0, 7) == "Bearer ")
    {
        return auth_header.substr(7);
    }
    return "";
}

std::string JWTAuthenticator::base64_url_encode(const std::string &data)
{
    std::string encoded = crow::utility::base64encode(data, data.length());

    // Convert to URL-safe base64
    std::replace(encoded.begin(), encoded.end(), '+', '-');
    std::replace(encoded.begin(), encoded.end(), '/', '_');

    // Remove padding
    encoded.erase(std::find(encoded.begin(), encoded.end(), '='), encoded.end());

    return encoded;
}

std::string JWTAuthenticator::base64_url_decode(const std::string &encoded)
{
    std::string data = encoded;

    // Convert from URL-safe base64
    std::replace(data.begin(), data.end(), '-', '+');
    std::replace(data.begin(), data.end(), '_', '/');

    // Add padding if needed
    while (data.length() % 4)
    {
        data += "=";
    }

    return crow::utility::base64decode(data, data.length());
}

std::string JWTAuthenticator::hmac_sha256(const std::string &data, const std::string &key)
{
    unsigned char result[SHA256_DIGEST_LENGTH];
    unsigned int len = SHA256_DIGEST_LENGTH;

    HMAC(EVP_sha256(), key.c_str(), key.length(),
         reinterpret_cast<const unsigned char *>(data.c_str()), data.length(),
         result, &len);

    return std::string(reinterpret_cast<char *>(result), len);
}

std::vector<std::string> JWTAuthenticator::split_token(const std::string &token)
{
    std::vector<std::string> parts;
    std::stringstream ss(token);
    std::string part;

    while (std::getline(ss, part, '.'))
    {
        parts.push_back(part);
    }

    return parts;
}

JWTPayload JWTAuthenticator::parse_payload(const std::string &payload_json)
{
    crow::json::rvalue payload_obj = crow::json::load(payload_json);

    JWTPayload payload;
    payload.user_id = payload_obj["sub"].s();
    payload.username = payload_obj["username"].s();
    payload.role = payload_obj["role"].s();
    payload.exp = payload_obj["exp"].i();
    payload.iat = payload_obj["iat"].i();
    payload.iss = payload_obj["iss"].s();
    payload.aud = payload_obj["aud"].s();

    return payload;
}

std::string JWTAuthenticator::create_header()
{
    crow::json::wvalue header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    return header.dump();
}

std::string JWTAuthenticator::create_payload(const std::string &user_id, const std::string &username,
                                             const std::string &role, std::int64_t exp, std::int64_t iat)
{
    crow::json::wvalue payload;
    payload["sub"] = user_id;
    payload["username"] = username;
    payload["role"] = role;
    payload["exp"] = exp;
    payload["iat"] = iat;
    payload["iss"] = issuer_;
    payload["aud"] = audience_;
    return payload.dump();
}

bool JWTAuthenticator::verify_signature(const std::string &header, const std::string &payload,
                                        const std::string &signature)
{
    std::string signing_input = header + "." + payload;
    std::string expected_signature = hmac_sha256(signing_input, secret_);
    std::string provided_signature = base64_url_decode(signature);

    return expected_signature == provided_signature;
}
