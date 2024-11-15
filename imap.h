#ifndef IMAP_H
#define IMAP_H

#include <string>

class ImapClient {
public:
    ImapClient(const std::string& server, int port);
    ~ImapClient();

    // Connects to the server
    bool connect();

    // Log in with username and password
    bool login(const std::string& username, const std::string& password);

    // Send an IMAP command to the server
    std::string send_command(const std::string& command);

    // Perform specific actions
    std::string select_mailbox(const std::string& mailbox);
    std::string fetch_message(int message_id);
    std::string search_mailbox(const std::string& criteria);

    // Log out and disconnect from the server
    void logout();

private:
    std::string server_;
    int port_;
    int sockfd_;
    int message_id_counter_;

    // Helper function to receive a response from the server
    std::string receive_response();
};

#endif // IMAP_H
