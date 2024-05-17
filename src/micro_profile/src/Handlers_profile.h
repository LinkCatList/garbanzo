#pragma once

#include <exception>
#include <string>
#include "../../Validation.h"
#include "../../Tokens.h"
#include <httplib.h>
#include <bcrypt.h>
#include <json/json.h>
#include <jwt-cpp/jwt.h>

inline void handle_get_profile (const httplib::Request &req, httplib::Response &res, Database &db) {

    std::istringstream iss(req.body);
    Json::Value j;
    Json::CharReaderBuilder builder;
    std::string errs;

    if (!Json::parseFromStream(builder, iss, &j, &errs)) {
        res.status = 400;
        res.set_content(R"({"Reason" : "Error while reading json"})", "application/json");
        return;
    }
    Token t;

    try {
        t =  {
            j["access_token"].asString(),
            j["refresh_token"].asString()
        };
    }
    catch (std::exception &exp) {
        res.status = 400;
        res.set_content(R"({"Reason" : "Error while parsing json"})", "application/json");
        return;
    }
        
    auto val = update_tokens(t.access, t.refresh, db);
        
    if (val.first == "") {
        res.status = 401;
        res.set_content(R"({"Reason" : "Invalid refresh or access token"})", "application/json");
        return;
    }

    t.access = val.first;
    t.refresh = val.second;
    
    Json::Value j_token;
    j_token["access_token"] = t.access;
    j_token["refresh_token"] = t.refresh;

    auto decoded_jwt = jwt::decode(t.refresh);
    auto user_id = decoded_jwt.get_payload_claim("user_id").as_string();
    

    auto row = db.queryRow("select * from users where user_id=$1", user_id);


    Json::Value j_profile;
    j_profile["user_id"] = row["user_id"].as<std::string>();
    j_profile["login"] = row["login"].as<std::string>();
    j_profile["email"] = row["email"].as<std::string>();
    j_profile["img_link"] = row["img_link"].as<std::string>();
    j_profile["city"] = row["city"].as<std::string>();
    j_profile["cash"] = row["cash"].as<std::string>();
    
    Json::Value big_json;
    big_json["tokens"] = j_token;
    big_json["user"] = j_profile;

    Json::StreamWriterBuilder builder2;
    // возвращает вложенные jsonы с токенами и с профилем
    const std::string big_json_str = Json::writeString(builder2, big_json);

    res.status = 200;
    res.set_content(big_json_str, "application/json");
    return;
}