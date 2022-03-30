#include <iostream>
#include <vector>
#include <list>
#include <future>
#include <thread>
#include <chrono>
#include "matchfinder.hpp"

using namespace std::chrono_literals;

int main(int argc, const char** argv) 
{
    const char* patrn = argv[1];

    if (argc < 3 || !strcmp(patrn,"")) {
        std::cerr 
            << "main: cannot search without pattern\n";
        return 1;
    }

    using f_ret_type = std::vector<std::pair<bool,std::string>>;
    std::list<std::future<f_ret_type>> futures;
    std::vector<const char*> files;

    for (int i = 2; i < argc; ++i)
        files.push_back(argv[i]);

    for (const auto& e : files) {
        std::promise<f_ret_type> p;
        futures.push_back(p.get_future());

        std::thread t(match_finder(e,patrn), std::move(p));
        t.detach();
    }

    std::future_status status;
    while (!futures.empty()) {
        using iter_t = std::list<std::future<f_ret_type>>::iterator;
        iter_t it = futures.begin(), end = futures.end();
        while (it != end) {
            status = (*it).wait_for(4ms);
            if (status == std::future_status::ready) {
                for (const auto& e : (*it).get())
                    if (e.first)
                        std::cerr << e.second;
                    else
                        std::cout << e.second;
                it = futures.erase(it);
            } else
                ++it;
        }
    }

    return 0;
}
