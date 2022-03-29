#include <iostream>
#include "matchfinder.hpp"

int main(int argc, const char** argv) 
{
    const char* pat = argv[argc - 1];
    const char* file = argv[argc - 2];


    if (!strcmp(pat,"")) {
        std::cerr 
            << "main: cannot search with empty pattern\n";
        return 1;
    }

    match_finder m1(file, pat);
    m1();


    return 0;
}
