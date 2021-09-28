#include "httplib.h"
#include <iostream>
using namespace httplib;

int main(void)
{
  Server svr;

  svr.Post("/get-points", [](const Request &req, Response &res)
           {
             if (!req.has_param("mesh") || !req.has_param("config") || !req.has_param("slices") || !req.has_param("args") || !req.has_param("output"))
             {
               res.set_content("parameters are not as expected", "text/plain");
             }
             else
             {
               auto mesh = req.get_param_value("mesh");
               auto config = req.get_param_value("config");
               auto slices = req.get_param_value("slices");
               auto args = req.get_param_value("args");
               auto output = req.get_param_value("output");

               std::cout << "mesh: " << mesh << std::endl
                         << "config: " << config << std::endl
                         << "slices: " << slices << std::endl
                         << "args: " << args << std::endl
                         << "output: " << output << std::endl;

               // Call the logic executable then wait for it to finish
               // https://stackoverflow.com/a/5155626/3405291
               // https://stackoverflow.com/a/31521217/3405291
               char command[100];
               std::sprintf(command,
                            "../build/cpp-service --get-points -mesh %s -config %s -slices %s -args %s -outputpoints %s",
                            mesh.c_str(),
                            config.c_str(),
                            slices.c_str(),
                            args.c_str(),
                            output.c_str());
               std::system(command);

               res.set_content("done", "text/plain");
             }
           });

  svr.Post("/generate", [](const Request &req, Response &res)
           {
             if (!req.has_param("mesh") || !req.has_param("config") || !req.has_param("points"))
             {
               res.set_content("parameters are not as expected", "text/plain");
             }
             else
             {
               auto mesh = req.get_param_value("mesh");
               auto config = req.get_param_value("config");
               auto points = req.get_param_value("points");

               std::cout << "mesh: " << mesh << std::endl
                         << "config: " << config << std::endl
                         << "points: " << points << std::endl;

               // Call the logic executable then wait for it to finish
               // https://stackoverflow.com/a/5155626/3405291
               // https://stackoverflow.com/a/31521217/3405291
               char command[100];
               std::sprintf(command,
                            "../build/cpp-service --generate -mesh %s -config %s -points %s",
                            mesh.c_str(),
                            config.c_str(),
                            points.c_str());
               std::system(command);

               res.set_content("done", "text/plain");
             }
           });

  int port = 58080;
  std::string address = "0.0.0.0"; // All addresses
  std::cout << "=> service going to listen on port: " << port << std::endl;
  if (!svr.listen(address.c_str(), port, 0))
  {
    std::cerr << "Server stopped in error state" << std::endl;
    return EXIT_FAILURE;
  }
}