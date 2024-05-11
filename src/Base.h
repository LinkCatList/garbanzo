#pragma once

#include "jwt-cpp/jwt.h"
#include <string>
#include <bcrypt.h>

const std::string SECRET_KEY = "21421r32fgwdf45";

struct User {
    std::string login;
    std::string password;
    std::string email;
    std::string img_link;
    std::string city;
    int64_t cash;

    User (std::string login_ = "", std::string password_ = "", std::string email_ = "", std::string img_link_ = "", 
        std::string city_ = "", int64_t cash_ = 0) 
            : login(login_), password(password_), email(email_), img_link(img_link_), 
                city(city_), cash(cash_) {
    }
};

inline std::string hash_string (std::string user_password) {
    std::string hash = bcrypt::generateHash(user_password);
    return hash;
}

inline std::string generate_jwt_access_token (const std::string &user_id) {
    auto token = jwt::create()
        .set_type("JWS")
        .set_payload_claim("user_id", jwt::claim(user_id))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{600})
        .sign(jwt::algorithm::hs256{SECRET_KEY});
    return token;
}

inline std::string generate_jwt_refresh_token(const std::string &user_id) {
    auto token = jwt::create()
        .set_type("JWS")
        .set_payload_claim("user_id", jwt::claim(user_id))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{2592000})
        .sign(jwt::algorithm::hs256{SECRET_KEY});
    return token;
}