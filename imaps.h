#ifndef IMAPS_CLIENT_H
#define IMAPS_CLIENT_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <string>
#include <stdexcept>
#include <iostream>

class ImapsClient {
public:
    ImapsClient(const std::string& server, int port, const std::string& cert_file = "", const std::string& cert_dir = "/etc/ssl/certs");
    ~ImapsClient();

    bool connect();
    bool login(const std::string& username, const std::string& password);
    bool logout();
    std::string select_mailbox(const std::string& mailbox);
    std::string search_mailbox(const std::string& criteria);
    std::string fetch_message(int message_id);
    std::string send_command(const std::string& command);
    //bool send_command(const std::string& command);
    //std::string receive_response();
    void disconnect();

private:
    //std::string send_command(const std::string& command);
    std::string receive_response();

    std::string server_;
    int port_;
    std::string cert_file_;
    std::string cert_dir_;
    SSL_CTX* ctx_;
    SSL* ssl_;
    BIO* bio_;
    int message_id_counter_;
};

#endif // IMAPS_CLIENT_H
