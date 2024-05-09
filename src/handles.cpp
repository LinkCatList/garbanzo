#include <iostream>
#include "Database.h"
#include <httplib.h>



void handle_register (const httplib::Request &req, httplib::Response &res) {
    if (req.has_param("login") && req.has_param("password") && 
        req.has_param("email") && req.has_param("city")) {
            
    }
}
