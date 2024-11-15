#ifndef IMAPS_CLIENT_H
#define IMAPS_CLIENT_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include "BaseClient.h"

class ImapsClient : public BaseClient{
public:
    ImapsClient(const std::string& server, int port, const std::string& cert_file = "", const std::string& cert_dir = "/etc/ssl/certs");
    ~ImapsClient();

    bool connect()override;
    bool login(const std::string& username, const std::string& password)override;
    bool logout()override;
    std::string select_mailbox(const std::string& mailbox)override;
    std::string search_mailbox(const std::string& criteria)override;
    std::string fetch_message(int message_id)override;
    std::string send_command(const std::string& command)override;
    void disconnect()override;
    int message_id_counter_;

private:
    std::string receive_response();
    std::string server_;
    int port_;
    std::string cert_file_;
    std::string cert_dir_;
    SSL_CTX* ctx_;
    SSL* ssl_;
    BIO* bio_;
};

#endif // IMAPS_CLIENT_H
