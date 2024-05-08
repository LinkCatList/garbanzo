#include <httplib.h>

int main() {

    httplib::Server svr;

    svr.Get("/home", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.listen("localhost", 1234);
}