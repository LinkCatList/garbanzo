#include <httplib.h>
#include "Database.h"

int32_t main() {

    httplib::Server svr;

    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        std::string responseString = R"({"Status" : "ok"})";
        res.set_content(responseString, "application/json");
    });
    
    svr.listen("localhost", 1337);
}