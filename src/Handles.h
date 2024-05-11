#include <json/value.h>
#include <string>
#include "Database.h"
#include "Validation.h"
#include <httplib.h>
#include <bcrypt.h>
#include <json/json.h>
#include <jwt-cpp/jwt.h>


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

            bool exists_login = db.queryValue<bool>("select exists (select 1 from users where login=$1)",
                u.login);
            bool exists_email = db.queryValue<bool>("select exists(select 1 from users where email=$1)",
                u.email);

            if (exists_email || exists_login) {
                res.status = 409;
                res.set_content(R"({"Reason" : "User with such email or login exists"})", "application/json");
                return;
            }

            if (!user_validation::validate_user(u, db)) {
                res.status = 400;
                res.set_content(R"({"Reason" : "Invalid user's data"})", "application/json");
                return;
            }
            u.password = hash_string(u.password + u.login);

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
        
        bool user_exists = db.queryValue<bool>("select exists(select 1 from users where login=$1)", u.login);
        if (!user_exists) {
            res.status = 401;
            res.set_content(R"({"Reason" : "Invalid user's login or password"})", "application/json");
            return;
        }

        auto row = db.queryRow("select user_id, hash_password from users where login=$1",
            u.login);
        
        std::string db_password = row["hash_password"].as<std::string>();
        std::string user_id = row["user_id"].as<std::string>();

        
        if (!bcrypt::validatePassword(u.password + u.login, db_password)) {
            res.status = 401;
            res.set_content(R"({"Reason" : "Invalid user's login or password"})", "application/json");
            return;
        }

        std::string user_access_token = generate_jwt_access_token(user_id);
        std::string user_refresh_token = generate_jwt_refresh_token(user_id);

        std::string hash_refresh_token = hash_string(user_refresh_token);
        std::string hash_access_token = hash_string(user_access_token);

        bool exists = db.queryValue<bool>("select exists(select 1 from tokens where user_id=$1)", user_id);

        if (exists) {
            db.exec("update tokens set hash_refresh_token=$1, hash_access_token=$2 where user_id=$3",
                hash_refresh_token, hash_access_token, user_id);
        }
        else {
            db.exec("insert into tokens (user_id, hash_refresh_token, hash_access_token) "
                "values ($1, $2, $3)", user_id, hash_refresh_token, hash_access_token);
        }

        Json::Value j;
        j["access_token"] = user_access_token;
        j["refresh_token"] = user_refresh_token;
        
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

inline void handle_get_profile (const httplib::Request &req, httplib::Response &res, Database &db) {
    // if (req.has_param("access_token") &&)
}
