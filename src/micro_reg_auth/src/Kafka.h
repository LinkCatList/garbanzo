#include <httplib.h>
#include <librdkafka/rdkafkacpp.h>

inline bool send_payload (const std::string &payload, const std::string &topic, RdKafka::Producer *producer) {
    RdKafka::ErrorCode resp = producer->produce(
        topic,                   
        RdKafka::Topic::PARTITION_UA, // Раздел (не указан)
        RdKafka::Producer::RK_MSG_COPY /* Копируем payload */,
        (void *)payload.c_str(), 
        payload.size(),          // Размер payload
        NULL,                    // Ключ (необязательный)
        0,                       
        0,                      
        NULL
    );  

    if (resp != RdKafka::ERR_NO_ERROR) {
        return false;
    }
    
    return true;
}