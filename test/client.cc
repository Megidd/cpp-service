#include <httplib.h>
#include <iostream>

int main(void) {
  httplib::Client cli("localhost", 58080);

  if (auto res = cli.Get("/manu-supp")) {
    std::cout << res->status << std::endl;
    std::cout << res->get_header_value("Content-Type") << std::endl;
    std::cout << res->body << std::endl;
  } else {
    std::cout << "error code: " << res.error() << std::endl;
  }

  return 0;
}