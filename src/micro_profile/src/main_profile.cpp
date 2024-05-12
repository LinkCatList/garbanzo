#include <iostream>
#include <httplib.h>
#include "../../../include/laserpants/dotenv/dotenv.h"
#include <jwt-cpp/jwt.h>
#include "Handlers_profile.h"
#include "Database.h"
#include <librdkafka/rdkafkacpp.h>

int32_t main() {
    dotenv::init();
    const std::string URL = std::getenv("URL1");

    httplib::Server svr;
    Database db(URL);

    svr.Post("/get-profile", [&db](const httplib::Request &req, httplib::Response &res) {
        handle_get_profile(req, res, db);
    });

    svr.listen("localhost", 1338);
}
