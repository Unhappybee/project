#ifndef IMAP_H
#define IMAP_H

#include <string>
#include "BaseClient.h"

class ImapClient : public BaseClient{
public:
    ImapClient(const std::string& server, int port);
    ~ImapClient();

    bool connect()override;

    bool login(const std::string& username, const std::string& password)override;

    std::string send_command(const std::string& command)override;
    std::string select_mailbox(const std::string& mailbox)override;
    std::string fetch_message(int message_id, bool headers)override;
    std::string search_mailbox(const std::string& criteria)override;
    bool logout()override;
    void disconnect()override;

private:
    int message_id_counter_;
    std::string server_;
    int port_;
    int sockfd_;

    std::string receive_response();
};

#endif // IMAP_H
