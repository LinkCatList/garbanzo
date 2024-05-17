#include <iostream>
#include <httplib.h>
#include "../../../include/laserpants/dotenv/dotenv.h"
#include <jwt-cpp/jwt.h>
#include "Handlers_profile.h"
#include "Kafka.h"
#include "Database.h"
#include <librdkafka/rdkafkacpp.h>
#include <ostream>

int32_t main() {


    dotenv::init();
    const std::string URL = std::getenv("URL1");
    Database db(URL);
    // ------------------------------------------
    
    std::string errstr;
    std::string brokers = "localhost:9092"; 
    std::string topic = "my_topic"; 
    std::string group_id = "test_group2"; 

    // Create configuration objects
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
    
    RdKafka::Conf *producer_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    producer_conf->set("metadata.broker.list", brokers, errstr);

    RdKafka::Producer *producer = RdKafka::Producer::create(producer_conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        return 1;
    }
    std::cout << "ok\n";
    std::thread worker1([&consumer, &db, &producer]{
        while (true) {
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
                    if (key == "1") { // если сообщение пришло от регистрации
                        auto payload = static_cast<const char *>(msg->payload());
                        std::string payload_str = payload;
                        bool success = add_payload_user_to_db(payload_str, db);
                        std::cout << payload << std::endl;
                        if (success) {
                            std::string response = R"({"Status" : "ok"})";
                            bool flag = send_payload(response, "my_topic", producer);
                            std::cout << "send" << std::endl;
                            if (!flag) {
                                std::cout << "aboba" << std::endl;
                            }
                        }
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
