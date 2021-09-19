#include <httplib.h>
#include <iostream>
using namespace httplib;

int main(void) {
  Server svr;

  svr.Get("/manu-supp", [](const Request & /*req*/, Response &res) {
    res.set_content("Hello World!", "text/plain");
  });

  int port = 58080;
  std::cout << "=> service going to listen on port: " << port << std::endl;
  svr.listen("localhost", port);
}