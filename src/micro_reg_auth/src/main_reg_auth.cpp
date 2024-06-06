#include <iostream>
#include <httplib.h>
#include "Handlers_reg_auth.h"
#include "../../../include/laserpants/dotenv/dotenv.h"
#include <jwt-cpp/jwt.h>
#include <librdkafka/rdkafkacpp.h>

int32_t main() {

    dotenv::init();
    const std::string URL = std::getenv("URL1");

    httplib::Server svr;
    Database db(URL);

    // ----------------------------------------------------------------

    std::string brokers = "localhost:9092";
    std::string topic = "my_topic";
    std::string group_id = "test_group1"; 
    std::string errstr;

    RdKafka::Conf *producer_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    producer_conf->set("metadata.broker.list", brokers, errstr);

    RdKafka::Producer *producer = RdKafka::Producer::create(producer_conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        return 1;
    }
    
    RdKafka::Conf *consumer_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    // Set configuration properties
    consumer_conf ->set("bootstrap.servers", brokers, errstr);
    consumer_conf ->set("group.id", group_id, errstr);
    consumer_conf ->set("auto.offset.reset", "earliest", errstr);

    // Create KafkaConsumer instance
    RdKafka::KafkaConsumer *consumer = RdKafka::KafkaConsumer::create(consumer_conf, errstr);
    if (!consumer) {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
        return 1;
    }

    RdKafka::ErrorCode err = consumer->subscribe({topic});
    if (err) {
        std::cerr << "Failed to subscribe to topic: " << 
        RdKafka::err2str(err) << std::endl;
        return 1;
    }

    std::cout << "ok\n";
    // ----------------------------------------------------------------
    std::thread worker2([&db, &svr, &producer, &consumer]{
        svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
            std::string responseString = R"({"Status" : "ok"})";
            res.set_content(responseString, "application/json");
        });
        

        svr.Post("/register", [&db, &producer, &consumer](const httplib::Request &req, httplib::Response &res){
            handle_register(req, res, db, producer, consumer);
        });

        svr.Post("/sign-in", [&db](const httplib::Request &req, httplib::Response &res) {
            handle_sign_in(req, res, db);
        });
    });

    worker2.join();

    svr.listen("localhost", 1337);
}
