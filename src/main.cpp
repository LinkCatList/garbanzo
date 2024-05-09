#include <iostream>
#include <httplib.h>
#include "Database.h"
#include "Handles.h"
#include "../include/laserpants/dotenv/dotenv.h"
#include <jwt-cpp/jwt.h>

int32_t main() {

    std::string token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJpc3MiOiJhdXRoMCIsInNhbXBsZSI6InRlc3QifQ.lQm3N2bVlqt2-1L-FsOjtR6uE-L4E9zJutMWKIe1v1M";
    auto decoded = jwt::decode(token);
 
    for(auto& e : decoded.get_payload_json())
        std::cout << e.first << " = " << e.second << std::endl;
    
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
