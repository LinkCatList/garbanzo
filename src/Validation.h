#include <cctype>
#include <string>


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

}

inline bool validate_login (std::string &login) {

}

inline bool validate_city (std::string &city) {

}