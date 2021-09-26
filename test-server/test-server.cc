#include "httplib.h"
#include <iostream>
using namespace httplib;

int main(void)
{
  Server svr;

  svr.Post("/generate", [](const Request &req, Response &res)
           {
             if (!req.has_param("mesh") || !req.has_param("points") || !req.has_param("config"))
             {
               res.set_content("parameters are not as expected", "text/plain");
             }
             else
             {
               auto mesh = req.get_param_value("mesh");
               auto points = req.get_param_value("points");
               auto config = req.get_param_value("config");

               std::cout << "mesh: " << mesh << std::endl
                         << "points: " << points << std::endl
                         << "config" << config << std::endl;

               // Call the logic executable then wait for it to finish
               // https://stackoverflow.com/a/5155626/3405291
               // https://stackoverflow.com/a/31521217/3405291
               char command[100];
               std::sprintf(command,
                            "../build/cpp-service --generate -mesh %s -config %s -points %s",
                            mesh.c_str(),
                            config.c_str(),
                            points.c_str()
                            );
               std::system(command);

               res.set_content("done", "text/plain");
             }
           });

  int port = 58080;
  std::cout << "=> service going to listen on port: " << port << std::endl;
  svr.listen("localhost", port);
}