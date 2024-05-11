#pragma once

#include <jwt-cpp/jwt.h>
#include <iostream>
#include "Base.h"
#include "Database.h"

struct Token {
    std::string access;
    std::string refresh;
};

namespace token_validation {
    inline std::pair<bool, std::string> validate_access_token (std::string &access_token, Database &db) {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
            .expires_at_leeway(60); 

        auto decoded_jwt = jwt::decode(access_token);

        try {
            verifier.verify(decoded_jwt);
        }
        catch (const std::exception& exc) {
            std::cerr << "Error while check access token " << exc.what() << "\n";
            return {false, ""};
        }
        
        auto user_id = decoded_jwt.get_payload_claim("user_id").as_string();

        std::string db_hash_token = db.queryValue<std::string>("select hash_access_token from tokens "
            "where user_id=$1", user_id);
        
        // валидация токена (не пароля) просто бкрипт тупо назвал свои функции
        if (!bcrypt::validatePassword(access_token, db_hash_token)) {
            return {false, ""};
        }
        return {true, user_id};
    }

    inline std::pair<bool, std::string> validate_refresh_token (std::string &refresh_token, Database &db) {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
            .expires_at_leeway(2592000); 

        auto decoded_jwt = jwt::decode(refresh_token);

        try {
            verifier.verify(decoded_jwt);
        }
        catch (const std::exception& exc) {
            std::cerr << "Error while check refresh token: " << exc.what() << "\n";
            return {false, ""};
        }
        
        auto user_id = decoded_jwt.get_payload_claim("user_id").as_string();

        std::string db_hash_token = db.queryValue<std::string>("select hash_refresh_token from tokens "
            "where user_id=$1", user_id);
        
        // валидация токена (не пароля) просто бкрипт тупо назвал свои функции
        if (!bcrypt::validatePassword(refresh_token, db_hash_token)) {
            return {false, ""};
        }
        return {true, user_id};
    }
};

inline std::pair<std::string, std::string> update_tokens (const std::string &access_token, const std::string &refresh_token, Database &db) {

    Token t = {
        access_token,
        refresh_token
    };

    auto val = token_validation::validate_access_token(t.access, db);
    if (val.first) {
        return {access_token, refresh_token};
    }
    else {
        val = token_validation::validate_refresh_token(t.refresh, db);
        if (!val.first) {
            return {"", ""};
        }

        std::string new_refresh = generate_jwt_refresh_token(val.second);
        std::string new_access = generate_jwt_access_token(val.second);

        db.exec("update tokens set hash_refresh_token=$1, hash_access_token=$2 where user_id=$3",
            bcrypt::generateHash(new_refresh), bcrypt::generateHash(new_access), val.second);

        return {new_access, new_refresh};
    }
}