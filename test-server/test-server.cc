#include "httplib.h"
#include <iostream>
using namespace httplib;

int main(void)
{
  Server svr;

  svr.Post("/manu-supp", [](const Request &req, Response &res)
           {
             if (!req.has_param("stl") || !req.has_param("json"))
             {
               res.set_content("stl and json paths must be provided", "text/plain");
             }
             else
             {
               auto stl = req.get_param_value("stl");
               auto json = req.get_param_value("json");

               std::cout << "stl: " << stl << std::endl
                         << "json: " << json << std::endl;

               // Call the logic executable then wait for it to finish
               // https://stackoverflow.com/a/5155626/3405291
               // https://stackoverflow.com/a/31521217/3405291
               char command[50];
               std::sprintf(command, "../build/cpp-service -stl %s -json %s", stl.c_str(), json.c_str());
               std::system(command);

               res.set_content("done", "text/plain");
             }
           });

  int port = 58080;
  std::cout << "=> service going to listen on port: " << port << std::endl;
  svr.listen("localhost", port);
}