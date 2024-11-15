#include <iostream> //TODO sort alphabeticly 
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include "parseCMD.h"

//TODO error codes
namespace inputParsing{
    //returns the an option of a parametr
std::string get_option(std::vector<std::string>& args, const std::string& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            if (it + 1 != end)
                return *(it + 1);
    }
   
    return "";
}

//returns a position of an argument and if it's not set returns the end of a vector
std::vector<std::string>::iterator is_set(std::vector<std::string>& args, const std::string& arg) {
    auto it = std::find(args.begin(), args.end(), arg);  
    return it;  
}

void throw_error(std::string err_msg, int err_code){
    std::cerr << err_msg;
    exit(err_code);
}

struct cmd parseCMD(int argc, char* argv[]){
    std::vector<std::string> args(argv + 1, argv + argc);
    struct cmd input;

    if(auto pos = is_set(args, "-T"); pos != args.end()){
        input.imaps = true;
        args.erase(pos);  
    }

    if(auto pos = is_set(args, "-p"); pos != args.end()){
    std::string port = get_option(args, "-p");
        if(isdigit(port[0])){
            input.port = std::stoi(port);
            args.erase(pos, pos+2);  
        }
        else{
            throw_error("Port is not a number!\n", 1);
        }
    }else{//The default port used by the IMAP client is 143, while if you use the SSL secure connection the port 993 is used.
        if(input.imaps){
        input.port = 993;
        }else{
            input.port = 143;
        }
    }

    if(auto pos = is_set(args, "-n"); pos != args.end()){
        input.new_only = true;
        args.erase(pos);
    }

    if(auto pos = is_set(args, "-h"); pos != args.end()){
        input.headers_only = true;
        args.erase(pos);
    }

    if(auto pos = is_set(args, "-c"); pos != args.end()){
        if(input.imaps){
            input.cert_file = get_option(args, "-c"); 
        }else{
            throw_error("Encription was not enabled\n", 2);
        } 
        args.erase(pos, pos+2);     
    }

    if(auto pos = is_set(args, "-C"); pos != args.end()){
        if(input.imaps){
            input.cert_file = get_option(args, "-C"); //TODO file in the same directory
        }else{
            throw_error("Encription was not enabled\n", 2);
        } 
        args.erase(pos, pos+2);     
    }

    if(auto pos = is_set(args, "-b"); pos != args.end()){
        input.box = get_option(args, "-b");
        args.erase(pos, pos+2);
    }

    if(auto pos = is_set(args, "-a"); pos != args.end()){
        std::ifstream auth_file(get_option(args, "-a"));
        if (!auth_file.is_open()) {
            throw_error("Authentication file could not be opened.\n", 3);
        }

        std::string line;

        while (std::getline(auth_file, line)) {
            if (line.rfind("username = ", 0) == 0) {
                input.username = line.substr(11);
            } else if (line.rfind("password = ", 0) == 0) {
                input.password = line.substr(11);
            }
        }

        auth_file.close();
        args.erase(pos, pos+2);

        if (input.username.empty() || input.password.empty()) {
        throw_error("Invalid authentication file format. Ensure it contains 'username = ...' and 'password = ...'.\n", 3);
    }
    }else{
        throw_error("Authentication file is missing\n", 3);
    }

    if(auto pos = is_set(args, "-o"); pos != args.end()){
        input.out_dir = get_option(args, "-o");
        input.out_dir += '/';//TODO do I realy need it?
        args.erase(pos, pos+2);
    }else{
        throw_error("Output directory is missing\n", 4);
    }

    if(args.size() != 1 ){
        throw_error("Wrong number of input parameters!\n", 5);
    }
    else{
        input.server = args[0];
    }
return input;
}
}
