#include <filesystem>
#include <iostream>
#include <future>
#include "matchfinder.hpp"

typedef unsigned char uchar;
using deque_str = std::deque<std::string>;
namespace fs = std::filesystem;

std::mutex m;
bool verbose = false;
fs::path pwdpath = fs::current_path();

void worker(deque_str& files, const std::regex& patrn) {
    unsigned int finished = 0;
    unsigned int todo = files.size();

    while (finished != todo) {
        using ret_t = std::vector<std::pair<bool,std::string>>;

        ret_t ret = match_finder(files.front(), pwdpath, patrn)();

        {
            std::lock_guard<std::mutex> lock(m);

            for (const auto& e : ret)
                if (e.first) {
                    if (verbose)
                        std::cerr << e.second;
                } else
                    std::cout << e.second;
        }

        files.pop_front();
        ++finished;
    }
}

int main(int argc, const char** argv) 
{
    namespace rc = std::regex_constants;
    auto flags = rc::ECMAScript | rc::optimize | rc::nosubs | rc::icase;

    auto bail = [] {
        puts("main: invalid args");
        exit(1);
    };

    if (argc < 2) bail();

    int i = 1;
    for (; i < argc; ++i) { // process each arg
        if (argv[i][0] != '-'
                || (argv[i][0] == '-' && i == argc - 1))
            break;

        for (int j = 1; j < strlen(argv[i]); ++j)
            switch (argv[i][j]) {
                case 'v':
                    verbose = true;
                    break;
                case 'c':
                    flags &= ~rc::icase;
                    break;
                default:
                    bail();
            }
    }

    if (!strcmp(argv[i],"")) bail();

    std::regex patrn(argv[i], flags);

    uchar nthread = std::thread::hardware_concurrency();
    deque_str jobs[nthread];

    fs::recursive_directory_iterator iter(pwdpath,
            fs::directory_options::skip_permission_denied);
    int curr = 0;
    for (const auto& e : iter) 
        if (e.is_regular_file()) {
            jobs[curr].emplace_back(e.path().c_str());
            curr = curr == nthread - 1 ? 0 : ++curr;
        }

    std::vector<std::thread> threads;

    for (int i = 0; i < nthread; ++i)
        threads.emplace_back(worker, std::ref(jobs[i]), std::cref(patrn));

    for (auto& e : threads)
        e.join();

    return 0;
}
