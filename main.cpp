#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::cout << "Main logic started!\n";

    // https://stackoverflow.com/a/442137/3405291
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string pathStl, pathJson;

    for (auto i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            std::cout << "Syntax: cpp-service -stl <stl_file> -json <json_file>" << std::endl;
            return 0;
        } else if (*i == "-stl") {
            pathStl = *++i;
        } else if (*i == "-json") {
            pathJson = *++i;
        }
    }

    std::cout << "STL=="<< pathStl << " | JSON==" << pathJson;
}
