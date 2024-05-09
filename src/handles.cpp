#include <iostream>
#include "Database.h"
#include "Validation.h"
#include <httplib.h>



void handle_register (const httplib::Request &req, httplib::Response &res) {
    if (req.has_param("login") && req.has_param("password") && 
        req.has_param("email") && req.has_param("city")) {
            User u {
                req.get_param_value("login"),
                req.get_param_value("password"),
                req.get_param_value("email"),
                "",
                req.get_param_value("city"),
                0
            };

            if (!validate_user(u)) {
                res.status = 400;
                res.set_content(R"({"Reason" : "Invalid user's data"})", "application/json");
                return;
            }
            

    }
}
