#include <string>

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
