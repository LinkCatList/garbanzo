#include <iostream>
#include <httplib.h>
#include "../../../include/laserpants/dotenv/dotenv.h"
#include <jwt-cpp/jwt.h>
#include "Handlers_profile.h"
#include "Kafka.h"
#include "Database.h"
#include <librdkafka/rdkafkacpp.h>

int32_t main() {


    dotenv::init();
    const std::string URL = std::getenv("URL1");
    Database db(URL);
    // ------------------------------------------
    std::cout << "ok\n";
    
    std::string errstr;
    std::string brokers = "localhost:9092"; // Замените на адреса ваших брокеров
    std::string topic = "my_topic"; // Замените на имя вашего топика
    std::string group_id = "test_group"; // Замените на ID вашей группы потребителей

    // Create configuration objects
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    // Set configuration properties
    conf->set("bootstrap.servers", brokers, errstr);
    conf->set("group.id", group_id, errstr);
    conf->set("auto.offset.reset", "earliest", errstr);

    // Create KafkaConsumer instance
    RdKafka::KafkaConsumer *consumer = RdKafka::KafkaConsumer::create(conf, errstr);
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

    std::thread worker1([&consumer, &db]{
        while (true) {
            RdKafka::Message *msg = consumer->consume(1000);
            switch (msg->err()) {
            case RdKafka::ERR__TIMED_OUT:
                break;

            case RdKafka::ERR_NO_ERROR:
                {
                    auto payload = static_cast<const char *>(msg->payload());
                    std::string payload_str = payload;
                    add_payload_user_to_db(payload_str, db);
                    std::cout << "ok\n";
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
                return 1;
            }
            delete msg;
        }
    });

    // ------------------------------------------



    httplib::Server svr;
    std::thread worker2([&svr, &db]{
        svr.Post("/get-profile", [&db](const httplib::Request &req, httplib::Response &res) {
            handle_get_profile(req, res, db);
        });

        svr.listen("localhost", 1338);
    });

    worker1.join();
    worker2.join(); 
    
}
