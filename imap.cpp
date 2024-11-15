#include "imap.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdexcept>
#include <cstring>

ImapClient::ImapClient(const std::string& server, int port)
    :message_id_counter_(1), server_(server), port_(port), sockfd_(-1) {}

ImapClient::~ImapClient() {
    if (sockfd_ != -1) {
        close(sockfd_);
    }
}

bool ImapClient::connect() {
    struct sockaddr_in serv_addr;
    struct hostent *server_host = gethostbyname(server_.c_str());

    if (!server_host) {
        std::cerr << "Error: No such host" << std::endl;
        return false;
    }

    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server_host->h_addr, server_host->h_length);
    serv_addr.sin_port = htons(port_);

    if (::connect(sockfd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return false;
    }

    std::string server_greeting = receive_response();
    if (server_greeting.empty() || server_greeting.find("OK") == std::string::npos) {
        std::cerr << "Error: Invalid server greeting" << std::endl;
        return false;
    }
    
    return true;
}

bool ImapClient::login(const std::string& username, const std::string& password) {
    std::ostringstream command;
    command << "A" << message_id_counter_++ << " LOGIN " << username << " " << password << "\r\n";
    std::string response = send_command(command.str());
    return response.find("OK") != std::string::npos;
}

std::string ImapClient::send_command(const std::string& command) {
    if (send(sockfd_, command.c_str(), command.size(), 0) < 0) {
        throw std::runtime_error("Failed to send command to server");
    }
    return receive_response();
}

std::string ImapClient::select_mailbox(const std::string& mailbox) {
    std::ostringstream command;
    command << "A" << message_id_counter_++ << " SELECT " << mailbox << "\r\n";
    return send_command(command.str());
}

std::string ImapClient::fetch_message(int message_id) {
    std::ostringstream command;
    command << "A" << message_id_counter_++ << " FETCH " << message_id << " BODY[]\r\n";
    return send_command(command.str());
}

std::string ImapClient::search_mailbox(const std::string& criteria) {
    std::ostringstream command;
    command << "A" << message_id_counter_++ << " SEARCH " << criteria << "\r\n";
    return send_command(command.str());
}

bool ImapClient::logout() {
    std::ostringstream command;
    command << "A" << message_id_counter_++ << " LOGOUT\r\n";
    std::string response = send_command(command.str());
    return response.find("BYE") != std::string::npos && response.find("OK") != std::string::npos;
}

void ImapClient::disconnect(){
    close(sockfd_);
    sockfd_ = -1;
}

std::string ImapClient::receive_response() {
    std::string response;
    char buffer[4096];
    int len;

    while (true) {
        len = recv(sockfd_, buffer, sizeof(buffer) - 1, 0);
        if (len < 0) {
            throw std::runtime_error("Failed to receive response from server");
        } else if (len == 0) {
            // No more data; exit the loop
            break;
        }

        buffer[len] = '\0';
        response += buffer;

        // Check for end of the IMAP response, which should contain the completion status
        if (response.find("OK") != std::string::npos ||
            response.find("NO") != std::string::npos ||
            response.find("BAD") != std::string::npos) {
            break;
        }
    }

    return response;
}

