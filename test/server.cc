#include <httplib.h>
#include <iostream>
using namespace httplib;

const char *html = R"(
<form id="formElem">
  <div>
    <label>STL file of 3D model</label>
    <input type="file" name="stl_file" accept="stl/*">
  </div>
  <div>
    <label>JSON file of placeholder coordinates</label>
    <input type="file" name="json_file" accept="json/*">
  </div>
  <div>
    <input type="submit">
  </div>
</form>
<script>
  formElem.onsubmit = async (e) => {
    e.preventDefault();
    let res = await fetch('/manu-supp', {
      method: 'POST',
      body: new FormData(formElem)
    });
    console.log(await res.text());
  };
</script>
)";

int main(void) {
  Server svr;

  svr.Get("/", [](const Request & /*req*/, Response &res) {
    res.set_content(html, "text/html");
  });

  svr.Post("/manu-supp", [](const Request &req, Response &res) {
    auto stl_file = req.get_file_value("stl_file");
    auto json_file = req.get_file_value("json_file");

    std::cout << "stl file length: " << stl_file.content.length() << std::endl
         << "stl file name: " << stl_file.filename << std::endl
         << "json file length: " << json_file.content.length() << std::endl
         << "json file name: " << json_file.filename << std::endl;

    {
      std::ofstream ofs(stl_file.filename, std::ios::binary);
      ofs << stl_file.content;
    }
    {
      std::ofstream ofs(json_file.filename);
      ofs << json_file.content;
    }

    res.set_content("done", "text/plain");
  });

  int port = 58080;
  std::cout << "=> service going to listen on port: " << port << std::endl;
  svr.listen("localhost", port);
}