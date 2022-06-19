
#include <iostream>
#include "lyn/algorithm.hpp"

#include <string>

int main() {

    std::string apa = "abcdefghijklmnopqrstuvwxyz";
    std::cout << '[' << apa << "](" << apa.size() << ") => [";

    lyn::alg::unstable_erase_if(apa, [](char ch) { return ch % 2 == 0; });

    std::cout << apa << "](" << apa.size() << ")\n";

    lyn::alg::unstable_erase(apa, 'a');

    std::cout << '[' << apa << "](" << apa.size() << ")\n";
}
