#include <string>
#include "Database.h"
#include "Validation.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <bcrypt.h>

using json = nlohmann::json;


inline void handle_register (const httplib::Request &req, httplib::Response &res, Database &db) {
    if (req.has_param("login") && req.has_param("password") && 
        req.has_param("email") && req.has_param("city")) {
            User u {
                req.get_param_value("login"),
                req.get_param_value("password"),
                req.get_param_value("email"),
                "",
                req.get_param_value("city"),
                0
            };
        
            if (!validate::validate_user(u, db)) {
                res.status = 400;
                res.set_content(R"({"Reason" : "Invalid user's data"})", "application/json");
                return;
            }

            std::string hash = bcrypt::generateHash(u.password);
            u.password = hash;

            db.exec("insert into users (login, hash_password, email, img_link, city, cash) "
                "values ($1, $2, $3, $4, $5, $6)", u.login, u.password, u.email, u.img_link, u.city, 
                std::to_string(u.cash));

            std::string user_id = db.queryValue<std::string>("select user_id from users where login=$1", u.login);

            json j = {
                {"user_id", user_id},
                {"login", u.login},
                {"email", u.email},
                {"img_link", u.img_link},
                {"city", u.city},
                {"cash", u.cash}
            };

            res.status = 201;
            res.set_content(j.dump(), "application/json");
    }
    else {
        res.status = 400;
        res.set_content(R"({"Reason" : "Empty user's register fields"})", "application/json");
    }
}

inline void handle_auth (const httplib::Request &req, httplib::Response &res, Database &db) {
    
}
