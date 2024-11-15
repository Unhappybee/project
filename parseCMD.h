#ifndef IMAP_HELPER_H
#define IMAP_HELPER_H

#include <string>
#include <vector>

namespace inputParsing{
    struct cmd{
    std::string server = "";
    int port = 143;
    bool imaps = false;
    std::string cert_file = "";
    std::string cert_addr = "/etc/ssl/certs";
    bool new_only = false;
    bool headers_only = false;
    std::string username;
    std::string password;
    std::string box = "INBOX";
    std::string out_dir = "";

};
//returns the an option of a parametr
std::string get_option(std::vector<std::string>& args, const std::string& option_name);

//returns a position of an argument and if it's not set returns the end of a vector
std::vector<std::string>::iterator is_set(std::vector<std::string>& args, const std::string& arg);

void throw_error(std::string err_msg, int err_code);

struct cmd parseCMD(int argc, char* argv[]);
}

#endif