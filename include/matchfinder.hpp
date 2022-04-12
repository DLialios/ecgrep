#include <fstream>
#include <regex>

#define GREEN       "\e[0;32m"
#define PURPLE      "\e[0;35m"
#define CYAN        "\e[0;36m"
#define BOLD_RED    "\e[1;31m"
#define BOLD_YELLOW "\e[1;33m"
#define BOLD_BLUE   "\e[1;34m"
#define RED_BGRND   "\e[41m"
#define RESET       "\e[0m"
#define HIGHLIGHT   RED_BGRND BOLD_YELLOW

#define MAX_FILE_SZ 100'000'000

/* @brief Find all @a v in @a c.
 * @param c Container to search.
 * @param v Value to search for.
 * @return Vector of const iterators to matching elements. */
template<typename C, typename V>
auto find_all(C&& c, const V& v) {
    using iter_t = typename std::remove_reference<C>::type::const_iterator;
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

    /* @return False if the first 16KiB of the file
     *         contain a null byte. */
    bool
    is_binary(const std::string& s);

    /* @brief Find all regex matches in @a str.
     * @return Map of line numbers to vectors containing
     *         matches for that line. Match results are
     *         colored. */
    std::map<int, std::vector<std::string>> 
    get_matches(
            const std::string& str
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

    /* @brief Formats filename, line number, and matched 
     *        line so that final result can be printed. 
     * @param lineno Line number where match was found.
     * @param line Line in file that is to be displayed.
     * @param is_err If this message should be sent
     *               to stdout or stderr.
     * @return Pair denoting the string to print and which
     *         file descriptor to write it to. */
    std::pair<bool,std::string>
    print_match(
            const int lineno,
            std::string& line,
            const bool is_err
    );

    std::string filepath;
    const std::regex& patrn;

    public:

    match_finder(
            const std::string& filepath,
            const std::string& pwdpath,
            const std::regex& patrn
    );

    /* @return Vector of pairs denoting a final result 
     * to print and which file descriptor to write it to.
     * This operation loads the file into memory. */
    std::vector<std::pair<bool,std::string>>
    operator()();

};

