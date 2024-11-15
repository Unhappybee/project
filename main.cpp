#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "parseCMD.h"     
#include <regex>
#include <variant>
#include "BaseClient.h"
#include "imaps.h"
#include "imap.h"


std::string extract_body_from_fetch(const std::string& fetch_response) {

    std::regex body_marker_regex(R"(\{(\d+)\}\r\n)");
    std::smatch match;

    if (!std::regex_search(fetch_response, match, body_marker_regex)) {
        throw std::runtime_error("Failed to locate body size marker in FETCH response.");
    }

     
    int body_size = std::stoi(match[1]);

     
    size_t body_start_pos = match.position() + match.length();

    
    if (fetch_response.size() < body_start_pos + body_size) {
        throw std::runtime_error("FETCH response is truncated and does not contain the full body.");
    }

    std::string body = fetch_response.substr(body_start_pos, body_size);

    return body;
}

// Function to save the email message to a file
void save_message(const std::string& content, const std::string& out_dir, int message_id) {
    std::string formated_response = extract_body_from_fetch(content);
    std::string file_path = out_dir + "/message_" + std::to_string(message_id) + ".eml";
    std::ofstream file(file_path);
    if (file.is_open()) {
        file << formated_response;
        file.close();
    } else {
        std::cerr << "Failed to save message to " << file_path << std::endl;
    }
}

bool folder_exists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
         
        return false;
    } else if (info.st_mode & S_IFDIR) {
         
        return true;
    }
     
    return false;
}



int main(int argc, char* argv[]) {
     
    inputParsing::cmd options = inputParsing::parseCMD(argc, argv);

    std::unique_ptr<BaseClient> client;
    if(options.imaps){
        client = std::make_unique<ImapsClient>(options.server, options.port, options.cert_file, options.cert_addr);
    }else{
        client = std::make_unique<ImapClient>(options.server, options.port);
    }
    
    if (!client->connect()) {
        std::cerr << "Failed to connect to server." << std::endl;
        return 1;
    }

    if (!client->login(options.username, options.password)) {
        std::cerr << "Failed to log in to the server." << std::endl;
        return 1;
    }

    // Select mailbox (e.g., INBOX)
    std::string select_response = client->select_mailbox(options.box);
    if (select_response.find("OK") == std::string::npos) {
        std::cerr << "Failed to select mailbox." << std::endl;
        return 1;
    }

    // Fetch message IDs based on criteria
    std::string criteria = options.new_only ? "UNSEEN" : "ALL";
    std::string search_response = client->search_mailbox(criteria);
    if (search_response.find("OK") == std::string::npos) {
        std::cerr << "Failed to search mailbox." << std::endl;
        return 1;
    }

    // Parse message IDs from the search response
    std::vector<int> message_ids;
    std::istringstream iss(search_response);
    int message_id;
    std::string temp;
    while (!iss.eof()) {
        iss >> temp;

        if (!temp.empty() && temp[0] == 'A') {
            break; 
        }

        if (std::stringstream(temp) >> message_id) {
            message_ids.push_back(message_id);
        }

        temp = ""; 
    }   

    for( int id: message_ids){
        std:: cout << id << std::endl;//TODO delete
    }

    if (!folder_exists(options.out_dir)) {
       std::cout << "Directory does not exist.\n";
    }
    // Download each message
    int downloaded_count = 0;
    for (int id : message_ids) {
        std::string fetch_response;
        if (options.headers_only) {
            fetch_response = client->fetch_message(id, true);
        } else {
            fetch_response = client->fetch_message(id, false);
        }

        // Save message content to file
        save_message(fetch_response, options.out_dir, id);
        downloaded_count++;
    }

    // Output number of downloaded messages
    std::cout << "Downloaded " << downloaded_count << " messages." << std::endl;

    // Log out and close the connection
    client->logout();
    client->disconnect();

    return 0;
}
