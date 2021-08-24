#include <iostream>
#include "PerHash.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    PerHash::HashMap<PerHash::StringPerHash, int> map{};
    map.emplace(PerHash::StringPerHash("Key"), 1337);
    std::cout << map.at(PerHash::StringPerHash("Key")) << std::endl;


    return 0;
}
