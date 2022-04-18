#include "matchfinder.hpp"

match_finder::match_finder(
    const std::string& filepath,
    const std::string& pwdpath,
    const std::regex& patrn
)
:patrn(patrn)
{
    auto it = filepath.begin();
    it += pwdpath.size();

    this->filepath = std::string(it + 1, filepath.end());
}

std::vector<std::pair<bool,std::string>>
match_finder::operator()() {
    std::vector<std::pair<bool,std::string>> res;
    auto sz = file_size();

    if (sz == 0) {
        std::string msg(
            BOLD_RED 
            "<cannot read file>" 
            RESET
        );
        res.push_back(print_match(-1,msg,true));
        return res;
    }

    // bail if the file is too big
    constexpr int max_sz_mb = MAX_FILE_SZ / 1000 / 1000;
    if (sz > MAX_FILE_SZ) {
        std::ostringstream os;
        os 
            << BOLD_RED "<file is over " 
            << max_sz_mb 
            << "MB>" RESET;
        std::string msg(os.str());
        res.push_back(print_match(-1,msg,true));
        return res;
    }

    std::ifstream is(filepath,std::ios::binary);
    std::istreambuf_iterator<char> it (is), eos;
    std::string data(it,eos);

    // bail if the file is likely binary
    if (!is_binary(data)) {
        std::string msg(
            BOLD_RED 
            "<file is not plaintext>" 
            RESET
        );
        res.push_back(print_match(-1,msg,true));
        return res;
    }

    auto m = get_matches(data);

    for (const auto& e : m) {
        std::string s (e.second[0]);
        for (const auto& e2 : e.second) {
            s = merge_results(s, e2);
        }
        res.push_back(print_match(e.first,s,false));
    }

    if (res.empty()) {
        std::string msg(
            BOLD_RED 
            "<no matches>" 
            RESET
        );
        res.push_back(print_match(-1,msg,true));
    }

    return res;
}

std::streamoff
match_finder::file_size() {
    std::ifstream is (filepath);
    std::streamoff start = is.tellg();
    is.seekg(0, std::ios_base::end);
    return is.tellg() - start;
}

bool
match_finder::is_binary(
    const std::string& s
) {
    if (s.length() < 16385){ 
        for (const auto& e : s)
        if (e == 0)
            return false;
    } else {
        for (int i = 0; i < 16384; ++i)
            if (s[i] == 0)
                return false;
    }
    return true; 
}

std::map<int, std::vector<std::string>> 
match_finder::get_matches(
    const std::string& str
) {
    using namespace std;
    map<int,vector<string>> res;

    auto newline_locs = find_all(str,'\n');
    int max_index = newline_locs.size() - 1;
    int curr_index = 0;

    auto it = sregex_iterator(str.begin(),str.end(),patrn);
    auto end = sregex_iterator();
    for (smatch match; it != end; ++it) {
        match = *it;

        string prefix = match.prefix().str();
        string curr_m = match[0];
        string suffix = match.suffix().str();

        curr_index += find_all(prefix,'\n').size(); 

        decltype(newline_locs)::value_type start,end;
        if (newline_locs.empty()) { // edge case: file has no \n
            start = str.begin();
            end = str.end();
        } else if (curr_index == 0) { // start
            start = str.begin();
            end = newline_locs[0] + 1;
        } else if (curr_index == max_index) { // end
            start = newline_locs[curr_index - 1] + 1;
            end = str.end();
        } else { // middle
            start = newline_locs[curr_index - 1] + 1;
            end = newline_locs[curr_index] + 1;
        }

        string line(start, end);
        string line_color = color_output_string(line,curr_m,suffix);

        if (line_color.back() == '\n')
            line_color.pop_back();

        res[curr_index + 1].push_back(line_color);

        curr_index += find_all(curr_m,'\n').size(); 
    }

    return res;
}

std::string 
match_finder::color_output_string(
    const std::string& s,
    const std::string& match,
    const std::string& suffix
) {
    namespace rc = std::regex_constants;
    static const auto flags = rc::ECMAScript | rc::optimize | rc::nosubs;
    static const std::regex bpat(R"([[:space:]])", flags);

    std::string res(s);
    // location of match in res
    int sindex = find_match_index(res,match+suffix);
    int eindex = sindex + match.length();

    // when match does not contain any whitespace chars
    if (!std::regex_search(match, bpat)) {
        res.insert(sindex, HIGHLIGHT);
        res.insert(eindex + strlen(HIGHLIGHT), RESET);
        return res;
    }

    // when match does contain whitespace chars
    bool unclosed = false;
    bool altunclosed = false;
    std::string cmatch(match);
    for (int i = 0; cmatch.c_str()[i] != '\0'; ++i) {
        char curr = cmatch[i];
        if (std::regex_search(&curr, bpat)) {
            if (unclosed) {
                cmatch.insert(i, RESET);
                i += strlen(RESET);
                unclosed = false;
            }

            if (!altunclosed) {
                cmatch.insert(i, ALTHIGHLIGHT);
                i += strlen(ALTHIGHLIGHT);
                altunclosed = true;
            }

            cmatch[i] = XSTR(BLANK_CHAR)[0];
        } else { 
            if (altunclosed) {
                cmatch.insert(i, RESET);
                i += strlen(RESET);
                altunclosed = false;
            }

            if (!unclosed) {
                cmatch.insert(i, HIGHLIGHT);
                i += strlen(HIGHLIGHT);
                unclosed = true;
            }
        }
    }
    if (unclosed || altunclosed)
        cmatch += RESET;

    res.replace(sindex, match.size(), cmatch);
    return res;
}

int 
match_finder::find_match_index(
    const std::string& s,
    const std::string& suffix
) {
    using sz_t = std::string::size_type;
    for (sz_t i = 0; i < s.length(); ++i) {
        if (s[i] == suffix[0]) {
            for (sz_t j = i; j < s.length(); ++j) {
                if (s[j] != suffix[j - i]) {
                    break;
                }
                if (j == s.length() - 1)
                    return i;
            }
        }
    }
    return -1;
}

std::string
match_finder::merge_results(
    const std::string& s1,
    const std::string& s2
) {
    if (s1 == s2)
        return s1;

    std::string res;
    int s1len = s1.length();
    int s2len = s2.length();

    int i = 0, j = 0;
    bool unclosed = false; // does HIGHLIGHT have matching RESET?
    while(i < s1len && j < s2len) {
        bool both_equal = s1[i] == s2[j];
        bool neither_esc = s1[i]!='\033' && s2[j]!='\033';
        if (both_equal && neither_esc) {
            res += s1[i]; 
            ++i;
            ++j;
        } else if (!both_equal && neither_esc && unclosed) {
            res += XSTR(BLANK_CHAR);
            ++i;
            ++j;
        } else {
            using cstriter = std::string::const_iterator;
            std::pair<cstriter,cstriter> s1iters,s2iters;

            if (s1[i] == '\033')
                s1iters = get_term_ctrl_loc(s1, i);

            if (s2[j] == '\033') 
                s2iters = get_term_ctrl_loc(s2, j);

            std::string s1_term_seq (s1iters.first,s1iters.second);
            std::string s2_term_seq (s2iters.first,s2iters.second);

            if (s1_term_seq == RESET) {
                res += RESET;
                i += strlen(RESET);
                unclosed = false;
            } else if (s2_term_seq == RESET) {
                res += RESET;
                j += strlen(RESET);
                unclosed = false;
            } else if (s1_term_seq.empty()) {
                if (s2_term_seq == HIGHLIGHT) {
                    res += HIGHLIGHT;
                    j += strlen(HIGHLIGHT);
                } else {
                    res += ALTHIGHLIGHT;
                    j += strlen(ALTHIGHLIGHT);
                }
                unclosed = true;
            } else {
                if (s1_term_seq == HIGHLIGHT) {
                    res += HIGHLIGHT;
                    i += strlen(HIGHLIGHT);
                } else {
                    res += ALTHIGHLIGHT;
                    i += strlen(ALTHIGHLIGHT);
                }
                unclosed = true;
            }
        }
    }

    if (unclosed)
        res += RESET;

    // one of the strings may not have
    // finished parsing yet
    while (i < s1len) {
        res += s1[i];
        ++i;
    }

    while (j < s2len) {
        res += s2[j];
        ++j;
    }

    return res;
}

std::pair<std::string::const_iterator,std::string::const_iterator>
match_finder::get_term_ctrl_loc(
    const std::string& s,
    int offset
) {
    using cstriter = std::string::const_iterator;
    cstriter start = s.begin() + offset, end;
    std::string seq;

    do {
        while (s[offset] != 'm') 
            ++offset;
        offset += 1;
        end = s.begin() + offset;
        seq = std::string(start, end);
    } while (seq != HIGHLIGHT && seq != ALTHIGHLIGHT && seq != RESET);

    return {start, end};
}

std::pair<bool,std::string>
match_finder::print_match(
    const int lineno,
    std::string& line,
    const bool is_err
) { 
    std::ostringstream os;
    os
        <<PURPLE<<filepath<<RESET
        <<CYAN<<":"<<RESET
        <<GREEN<<lineno<<RESET
        <<CYAN<<":"<<RESET
        <<line<<'\n';

    return {is_err,os.str()};
}

