#pragma once

#include "Database.h"
#include <exception>
#include <iostream>
#include "../../Base.h"
#include <json/reader.h>
#include <json/value.h>
#include <librdkafka/rdkafkacpp.h>

inline bool add_payload_user_to_db (const std::string &json_str, Database &db) {

    Json::Reader reader;
    Json::Value j;
    bool parsingSuccessful = reader.parse(json_str, j);
    // происходит парсинг json
    if (!parsingSuccessful) {
        std::cout << "cant parse json" << std::endl;
        return false;
    }

    User u;
    u = {
        j["login"].asString(),
        "",
        j["email"].asString(),
        j["img_link"].asString(),
        j["city"].asString(),
        j["cash"].asInt64(),
    };
    
    bool exists = db.queryValue<bool>("select exists(select 1 from users where login=$1)", u.login);
    if (exists) {
        std::cout << "user exists" << std::endl;
        return false;
    }
    else {
        // в таблице пользователей базы данных микросервиса профиля не хранится пароль
        // мб потом если понадобится придется хранить...
        db.exec("insert into users(login, email, img_link, city, cash) values($1, $2, $3, $4, $5)", 
            u.login, u.email, u.img_link, u.city, u.cash);
        return true;
    }
}

inline bool send_payload (const std::string &payload, const std::string &topic, RdKafka::Producer *producer) {
    RdKafka::ErrorCode resp = producer->produce(
        topic,                   
        RdKafka::Topic::PARTITION_UA, // Раздел 
        RdKafka::Producer::RK_MSG_COPY /* Копируем payload */,
        (void *)payload.c_str(), 
        payload.size(),          // Размер payload
        "2",                    // Ключ (профиль -> регистрация)
        1,                       
        0,                      
        NULL
    );  

    if (resp != RdKafka::ERR_NO_ERROR) {
        return false;
    }
    
    return true;
}