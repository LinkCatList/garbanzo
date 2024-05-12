#include <iostream>
#include <httplib.h>
#include "Handlers.h"
#include "../include/laserpants/dotenv/dotenv.h"
#include <jwt-cpp/jwt.h>

int32_t main() {
    dotenv::init();
    const std::string URL = std::getenv("URL");

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

    svr.Post("/get-profile", [&db](const httplib::Request &req, httplib::Response &res) {
        handle_get_profile(req, res, db);
    });

    svr.listen("localhost", 1337);
}
