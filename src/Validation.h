#include <cctype>
#include <string>
#include "Base.h"
#include <iostream>
#include <pqxx/pqxx>
#include "Database.h"
#include "bcrypt.h"
#include <regex>
#include <jwt-cpp/jwt.h>

namespace user_validation {
    inline bool validate_password (std::string &password) {
        if (password.size() < 6) return false;

        bool wasLow = false;
        bool wasUp = false;
        bool wasDigit = false;

        for (const auto &c : password) {
            if (std::isdigit(c)) {
                wasDigit = true;
            }
            else if (std::islower(c)) {
                wasLow = true;
            }
            else {
                wasUp = true;
            }
        }

        return wasDigit && wasLow && wasUp;
    }

    inline bool validate_email (std::string &email) {
        auto reg = std::regex(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");

        return std::regex_match(email, reg) && (email.size() <= 50);
    }

    inline bool validate_login (std::string &login) {

        return 4 < login.size() && login.size() < 12;
    }

    inline bool validate_city (std::string &city, Database &db) {
        bool flag = db.queryValue<bool>("select exists(select 1 from cities where name=$1)", city);

        return flag;
    }

    inline bool validate_user (User &u, Database &db) {
        return validate_password(u.password) && validate_city(u.city, db) &&
            validate_email(u.email) && validate_login(u.login);
    }
};

namespace token_validation {
    inline bool validate_access_token (User &u, std::string &access_token, Database &db) {
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
            .expires_at_leeway(60); 

        auto decoded_jwt = jwt::decode(access_token);

        try {
            verifier.verify(decoded_jwt);
        }
        catch (const std::exception& exc) {
            std::cerr << "Error while check access token: " << exc.what() << "\n";
            return false;
        }
        
        auto user_id = decoded_jwt.get_payload_claim("user_id").as_string();

        std::string login = db.queryValue<std::string>("select login from users where user_id=$1",
            user_id);
        
        if (login != u.login) {
            return false;
        }

        std::string db_hash_token = db.queryValue<std::string>("select hash_access_token from tokens "
            "where user_id=$1", user_id);
        
        // валидация токена (не пароля). просто бкрипт тупо назвал свои функции
        if (!bcrypt::validatePassword(access_token, db_hash_token)) {
            return false;
        }
        return true;
    }

    inline bool validate_refresh_token (User &u, std::string &refresh_token, Database &db) {
        // ТУДУ
        return true;
    }
};