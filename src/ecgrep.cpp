#include <iostream>
#include <deque>
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
    std::deque<const char*> files;

    for (int i = 2; i < argc; ++i)
        files.push_back(argv[i]);

    unsigned int threads = 1;
    unsigned int finished = 0;
    unsigned int todo = files.size();
    while (finished != todo) {
        if (threads < std::thread::hardware_concurrency()
                && !files.empty()) {
            std::promise<f_ret_type> p;
            futures.push_back(p.get_future());

            std::thread(
                    match_finder(files.front(), patrn), std::move(p)
                    ).detach();

            ++threads;
            files.pop_front();
        }

        using iter_t = std::list<std::future<f_ret_type>>::iterator;

        iter_t it = futures.begin(), end = futures.end();
        while (it != end) {
            if ((*it).wait_for(4ms) == std::future_status::ready) {
                for (const auto& e : (*it).get())
                    if (e.first)
                        std::cerr << e.second;
                    else
                        std::cout << e.second;

                it = futures.erase(it);
                ++finished;
                --threads;
            } else
                ++it;
        }
    }

    return 0;
}
