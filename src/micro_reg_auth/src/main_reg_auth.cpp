#include <iostream>
#include <httplib.h>
#include "Handlers_reg_auth.h"
#include "../../../include/laserpants/dotenv/dotenv.h"
#include <jwt-cpp/jwt.h>
#include <librdkafka/rdkafkacpp.h>

int32_t main() {

    // std::string brokers = "localhost:9092";
    // std::string topic_str = "test_topic";
    // std::string errstr;

    // RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    // conf->set("metadata.broker.list", brokers, errstr);

    // RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    // if (!producer) {
    //     std::cerr << "Failed to create producer: " << errstr << std::endl;
    //     return 1;
    // }
    
    dotenv::init();
    const std::string URL = std::getenv("URL1");

    httplib::Server svr;
    Database db(URL);

    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        std::string responseString = R"({"Status" : "ok"})";
        res.set_content(responseString, "application/json");
    });
    

    svr.Post("/register", [&db](const httplib::Request &req, httplib::Response &res){
        handle_register(req, res, db);
    });

    svr.Post("/sign-in", [&db](const httplib::Request &req, httplib::Response &res) {
        handle_sign_in(req, res, db);
    });

    svr.listen("localhost", 1337);
}
