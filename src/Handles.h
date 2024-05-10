#include <cstdint>
#include <functional>
#include <json/value.h>
#include <string>
#include "Database.h"
#include "Validation.h"
#include <httplib.h>
#include <bcrypt.h>
#include <json/json.h>
#include <jwt-cpp/jwt.h>

const std::string SECRET_KEY = "21421r32fgwdf45";

inline std::string generate_jwt_token (const std::string &user_id) {
    auto token = jwt::create()
        .set_type("JWS")
        .set_payload_claim("login", jwt::claim(user_id))
        .sign(jwt::algorithm::hs256{SECRET_KEY});
    return token;
}

inline std::string hash_password (std::string user_password) {
    std::string hash = bcrypt::generateHash(user_password);
    return hash;
}

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
            u.password = hash_password(u.password + u.login);

            db.exec("insert into users (login, hash_password, email, img_link, city, cash) "
                "values ($1, $2, $3, $4, $5, $6)", u.login, u.password, u.email, u.img_link, u.city, 
                std::to_string(u.cash));


            std::string user_id = db.queryValue<std::string>("select user_id from users where login=$1", u.login);

            Json::Value j;
            j["user_id"] = user_id;
            j["login"] = u.login;
            j["email"] = u.email;
            j["img_link"] = u.img_link;
            j["city"] = u.city;
            j["cash"] = u.cash;
            Json::StreamWriterBuilder builder;
            const std::string json_str = Json::writeString(builder, j);

            res.status = 201;
            res.set_content(json_str, "application/json");
    }
    else {
        res.status = 400;
        res.set_content(R"({"Reason" : "Empty user's register fields"})", "application/json");
    }
}

inline void handle_sign_in (const httplib::Request &req, httplib::Response &res, Database &db) {
    if (req.has_param("login") && req.has_param("password")) {

        User u {
            req.get_param_value("login"),
            req.get_param_value("password")
        };

        auto row = db.queryRow("select user_id, hash_password from users where login=$1",
            u.login);
        
        std::string db_password = row["hash_password"].as<std::string>();
        std::string user_id = row["user_id"].as<std::string>();

        if (!bcrypt::validatePassword(u.password + u.login, db_password)) {
            res.status = 401;
            res.set_content(R"({"Reason" : "Invalid user's login or password"})", "application/json");
            return;
        }

        std::string user_token = generate_jwt_token(user_id);

        Json::Value j;
        j["token"] = user_token;
        Json::StreamWriterBuilder builder;
        const std::string json_str = Json::writeString(builder, j);
        res.status = 200;
        res.set_content(json_str, "application/json");
    }
    else {
        res.status = 400;
        res.set_content(R"({"Reason" : "Empty user's register fields"})", "application/json");
    }
}
