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

inline void handle_register (const httplib::Request &req, httplib::Response &res, Database &db, RdKafka::Producer *producer,
    RdKafka::KafkaConsumer *consumer) {
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


    std::mutex res_mutex;

    // отправляет данные пользователя на микросервис профиля, для добавления их там в бд
    std::thread worker1([&json_str, &producer, &res, &res_mutex]{
        bool success = send_payload(json_str, "my_topic", producer);
        std::cout << "send" << std::endl;
        if (!success) {
            std::lock_guard<std::mutex> lock(res_mutex);
            res.status = 500;
            std::string responseString = R"({"Reason" : "Error while send msg"})";
            res.set_content(responseString, "application/json");
            return;
        }
    });


    std::thread worker2([&res, &consumer, &json_str, &res_mutex]{
        std::cout << "capybara"<< std::endl;
        int timeout_ms = 10000; // 50 секунд таймаут
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::string response = "";
        while (response == "") {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
            if (duration.count() > timeout_ms) {
                std::lock_guard<std::mutex> lock(res_mutex);
                res.status = 504; // Gateway Timeout
                res.set_content(R"({"Reason" : "Timeout waiting for profile service"})", "application/json");
                return; 
            }
            RdKafka::Message *msg = consumer->consume(1000);
            // раскомментить в случае треша
            // std::cerr << msg->errstr() << std::endl;
            switch (msg->err()) {
            case RdKafka::ERR__TIMED_OUT:
                break;

            case RdKafka::ERR_NO_ERROR:
                {  
                    const char* key_data = static_cast<const char*>(msg->key_pointer());
                    std::string key(key_data, msg->key_len()); 
                    std::cout << key << std::endl;
                    if (key == "2") { // если сообщение пришло от регистрации
                        auto payload = static_cast<const char *>(msg->payload());
                        std::string payload_str = payload;
                        std::cout << payload_str << "\n";
                        response = payload_str;
                    }
                    break;
                }
            case RdKafka::ERR__PARTITION_EOF:
                std::cerr << "Reached end of partition" << std::endl;
                break;

            case RdKafka::ERR_UNKNOWN_TOPIC_OR_PART:
                std::cerr << "Unknown topic or partition" << std::endl;
                break;

            default:
                std::cerr << "Consume failed: " << msg->errstr() << std::endl;
                return;
            }
            delete msg;
        }
        std::lock_guard<std::mutex> lock(res_mutex);
        res.status = 201;
        res.set_content(json_str, "application/json");
    });
    worker1.join();
    worker2.join();
    // ТУДУ:
    // прежде чем отправлять сообщение, что юзер зареган, нужно дождаться ответа микросервиса профиля о регистрации

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

