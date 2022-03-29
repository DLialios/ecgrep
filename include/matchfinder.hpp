#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include <cmath>

#define GREEN       "\e[0;32m"
#define PURPLE      "\e[0;35m"
#define BOLD_RED    "\e[1;31m"
#define BOLD_YELLOW "\e[1;33m"
#define BOLD_BLUE   "\e[1;34m"
#define RED_BGRND   "\e[41m"
#define RESET       "\e[0m"
#define HIGHLIGHT   RED_BGRND

#define MAX_FILE_SZ 100'000'000

/* @brief Find all @a v in @a c.
 * @param c Container to search.
 * @param v Value to search for.
 * @return Vector of iterators to matching elements. */
template<typename C, typename V>
auto find_all(C&& c, const V& v) {
    using iter_t = typename std::remove_reference<C>::type::iterator;
    std::vector<iter_t> res;
    for (auto it = c.begin(), end = c.end(); it != end; ++it) 
        if (*it == v)
            res.push_back(it);
    return res; 
}

class match_finder {

    /* @return Size of this finder's file. */
    std::streamoff
    file_size();

    /* @brief Find all regex matches in @a str.
     * @return Map of line numbers to vectors containing
     *         matches for that line. Match results are
     *         colored. */
    std::map<int, std::vector<std::string>> 
    get_matches(
            std::string&& str,
            const std::regex& patrn
    );

    /* @brief Highlight the regex match in a string. 
     * @param s The result line to be colored.
     * @param match Chars in @a s for which the regex matched.
     * @param suffix Remainder of the text.
     * @return String with match highlighted. */
    std::string 
    color_output_string(
            const std::string& s,
            const std::string& match,
            const std::string& suffix
    );

    /* @return Index in @a s where @a suffix begins.
     * Variant of longest common substring problem. */
    int 
    find_match_index(
            const std::string& s,
            const std::string& suffix
    );

    /* @return String preserving the highlights from
     *         two separate matches. */
    std::string 
    merge_results(
            const std::string& s1,
            const std::string& s2
    );

    /* @param s String to search.
     * @param offset Offset in @a s to begin search.
     * @return Pair of const string iters that locate
     *         an ANSI terminal code in @a s. */
    std::pair<
        std::string::const_iterator,
        std::string::const_iterator
    > 
    get_term_ctrl_loc(
            const std::string& s,
            int offset
    );

    /* Print match in formatted style to stdout. */
    void 
    print_match(
            const char*,
            const int,
            std::string&,
            std::ostream&
    );

    const char* filepath;
    const char* input_ptrn;

    public:

    match_finder(
            const char*,
            const char*
    );

    /* Loads file into memory, performs searches,
     * and prints results to stdout. */
    void 
    operator()();

};
