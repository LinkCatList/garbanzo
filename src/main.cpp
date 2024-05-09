#include <httplib.h>
#include "Database.h"
#include "Handles.h"
#include "../include/laserpants/dotenv/dotenv.h"

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

    
    svr.listen("localhost", 1337);
}