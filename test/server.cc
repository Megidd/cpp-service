#include <httplib.h>
#include <iostream>
using namespace httplib;

int main(void) {
  Server svr;

  svr.Get("/manu-supp", [](const Request & /*req*/, Response &res) {
    res.set_content("Creating manual supports...", "text/plain");
  });

  svr.Post("/manu-supp", [](const Request &req, Response &res) {
    auto stl_file = req.get_file_value("stl_file");
    
    std::cout << "stl file length: " << stl_file.content.length() << std::endl
         << "stl file name: " << stl_file.filename << std::endl;

    {
      std::ofstream ofs(stl_file.filename, std::ios::binary);
      ofs << stl_file.content;
    }

    res.set_content("done", "text/plain");
  });

  int port = 58080;
  std::cout << "=> service going to listen on port: " << port << std::endl;
  svr.listen("localhost", port);
}