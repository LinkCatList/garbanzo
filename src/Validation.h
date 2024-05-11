#pragma once

#include <cctype>
#include <string>
#include <iostream>
#include <pqxx/pqxx>
#include "Database.h"
#include "bcrypt.h"
#include "Tokens.h"
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
