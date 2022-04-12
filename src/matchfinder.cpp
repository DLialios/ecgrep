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
    } else
        for (int i = 0; i < 16384; ++i)
            if (s[i] == 0)
                return false;
    return true; 
}

std::map<int, std::vector<std::string>> 
match_finder::get_matches(
        const std::string& str
        ) {
    using namespace std;
    map<int,vector<string>> res;
    auto str_begin = str.cbegin();
    auto str_end = str.cend();

    auto newline_locs = find_all(str,'\n');
    int total_lines = newline_locs.size() + 1;
    int curr_lineno = 1;
    int prev_lineno = 1;

    smatch match;
    vector<string> curr_line_matches;
    bool hasrun = false;
    while (regex_search(str_begin,str_end,match,patrn)) {
        hasrun = true;
        string prefix = match.prefix().str();
        string curr_m = match[0];
        string suffix = match.suffix().str();

        curr_lineno += find_all(prefix + curr_m,'\n').size();

        decltype(newline_locs)::value_type start,end;
        if (newline_locs.empty()) { // edge case: file has no \n
            start = str.begin();
            end = str.end();
        } else if (curr_lineno < 2) { // start
            start = str.begin();
            end = newline_locs[0];
        } else if (curr_lineno == total_lines) { // end
            start = newline_locs[curr_lineno - 2] + 1;
            end = str.end();
        } else { // middle
            start = newline_locs[curr_lineno - 2] + 1;
            end = newline_locs[curr_lineno - 1];
        }

        if (curr_lineno != prev_lineno
                && !curr_line_matches.empty()) {
            res.insert({prev_lineno, curr_line_matches});
            curr_line_matches.clear();
        }
        curr_line_matches.emplace_back(
                color_output_string(string(start,end),curr_m,suffix)
                );

        str_begin = match.suffix().first;
        prev_lineno = curr_lineno;
    }

    if (hasrun)
        res.insert({curr_lineno, curr_line_matches});

    return res;
}

std::string 
match_finder::color_output_string(
        const std::string& s,
        const std::string& match,
        const std::string& suffix
        ) {
    std::string res(s);
    std::string color(HIGHLIGHT);
    std::string mtch (match);

    // we have to remove newlines from match
    std::string::size_type inewline = mtch.find_last_of('\n');
    if (inewline != std::string::npos) {
        if (inewline == mtch.length() - 1)
            mtch.clear();
        else 
            mtch = mtch.substr(inewline + 1);
    }

    // insert terminal codes
    if (s == mtch) {
        res.insert(0, color);
        res.insert(res.length(), RESET);
    }
    else {    
        int start_index = find_match_index(s,mtch+suffix);
        int end_index = start_index + mtch.length() - 1;

        res.insert(start_index, color);
        res.insert(color.length() + end_index + 1, RESET);
    }

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
    while(i < s1len && j < s2len) {
        if (s1[i] != '\033' && s1[i] == s2[j]) {
            res += s1[i]; 
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
            } else if (s2_term_seq == RESET) {
                res += RESET;
                j += strlen(RESET);
            } else if (s1_term_seq.empty()) {
                res += HIGHLIGHT;
                j += strlen(HIGHLIGHT);
            } else {
                res += HIGHLIGHT;
                i += strlen(HIGHLIGHT);
            }
        }
    }

    if (i != s1len || j != s2len)
        res += RESET;

    return res;
}

std::pair<std::string::const_iterator,std::string::const_iterator>
match_finder::get_term_ctrl_loc(
        const std::string& s,
        int offset
        ) {
    using cstriter = std::string::const_iterator;
    cstriter start = s.begin() + offset;
    while (s[offset] != 'm') ++offset;
    ++offset;
    cstriter end = s.begin() + offset;
    return {start, end};
}

std::pair<bool,std::string>
match_finder::print_match(
        const int lineno,
        std::string& line,
        const bool is_err
        ) { 
    using sz_t = std::string::size_type; 
    sz_t min_len = std::strlen(HIGHLIGHT) + std::strlen(RESET);

    if (line.length() >= min_len
            && line.substr(0, min_len) == HIGHLIGHT RESET) {
        line.assign(BOLD_BLUE "<blank_match>" RESET);
    }

    std::ostringstream os;
    os
        <<PURPLE<<filepath<<RESET
        <<CYAN<<":"<<RESET
        <<GREEN<<lineno<<RESET
        <<CYAN<<":"<<RESET
        <<line<<'\n';

    return {is_err,os.str()};
}

