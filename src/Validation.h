#include <cctype>
#include <string>
#include <iostream>
#include <regex>


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
    auto reg = std::regex("([^.])([\\w-\\.]+)([^.])@((?:\\w+\\.)+)([a-zA-Z]{2,4})");
    return std::regex_match(email, reg) && (email.size() <= 50);
}

inline bool validate_login (std::string &login) {
    return 4 < login.size() && login.size() < 12;
}

inline bool validate_city (std::string &city) {

}