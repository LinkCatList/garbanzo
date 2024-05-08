#include <httplib.h>
#include "Database.h"

int main() {

    httplib::Server svr;

    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("OK!", "text/plain");
    });
    svr.listen("localhost", 1337);
}