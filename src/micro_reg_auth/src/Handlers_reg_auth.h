#pragma once

#include <exception>
#include <librdkafka/rdkafkacpp.h>
#include <string>
#include "Database.h"
#include "Kafka.h"
#include "../../Base.h"
#include "../../Validation.h"
#include <httplib.h>
#include <bcrypt.h>
#include <json/json.h>
#include <jwt-cpp/jwt.h>

inline void handle_register (const httplib::Request &req, httplib::Response &res, Database &db, RdKafka::Producer *producer) {
    std::istringstream iss(req.body);
    Json::Value j;
    Json::CharReaderBuilder builder;
    std::string errs;

    if (!Json::parseFromStream(builder, iss, &j, &errs)) {
        res.status = 400;
        res.set_content(R"({"Reason" : "Error while reading json"})", "application/json");
        return;
    }

    User u;
    try {
        u = {
            j["login"].asString(),
            j["password"].asString(),
            j["email"].asString(),
            "",
            j["city"].asString(),
            0
        };
    }
    catch (std::exception &exp) {
        res.status = 400;
        res.set_content(R"({"Reason" : "Error while parsing json"})", "application/json");
        return;
    }

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

    Json::Value js;
    js["user_id"] = user_id;
    js["login"] = u.login;
    js["email"] = u.email;
    js["img_link"] = u.img_link;
    js["city"] = u.city;
    js["cash"] = u.cash;
    Json::StreamWriterBuilder builder2; // биндинг json к строке
    const std::string json_str = Json::writeString(builder2, js);

    // отправляет данные пользователя на микросервис профиля, для добавления их там в бд
    bool success = send_payload(json_str, "my_topic", producer);
    if (!success) {
        res.status = 500;
        std::string responseString = R"({"Reason" : "Error while send msg"})";
        res.set_content(responseString, "application/json");
        return;
    }
    // ТУДУ:
    // прежде чем отправлять сообщение, что юзер зареган, нужно дождаться ответа микросервиса профиля о регистрации
    res.status = 201;
    res.set_content(json_str, "application/json");

}

inline void handle_sign_in (const httplib::Request &req, httplib::Response &res, Database &db) {

    std::istringstream iss(req.body);
    Json::Value j;
    Json::CharReaderBuilder builder;
    std::string errs;

    if (!Json::parseFromStream(builder, iss, &j, &errs)) {
        res.status = 400;
        res.set_content(R"({"Reason" : "Error while reading json"})", "application/json");
        return;
    }

    User u;
    try {
        u = {
            j["login"].asString(),
            j["password"].asString()
        };
    }
    catch (std::exception &exp) {
        res.status = 400;
        res.set_content(R"({"Reason" : "Error while parsing json"})", "application/json");
        return;
    }
        
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

    Json::Value js;
    js["access_token"] = user_access_token;
    js["refresh_token"] = user_refresh_token;
        
    Json::StreamWriterBuilder builder2; // биндинг json к строке
    const std::string json_str = Json::writeString(builder2, js);
    res.status = 200;
    res.set_content(json_str, "application/json");
}

