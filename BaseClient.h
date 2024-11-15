#ifndef BASECLIENT_H
#define BASECLIENT_H

#include <string>

class BaseClient {
public:
    virtual ~BaseClient() = default; // Virtual destructor for proper cleanup of derived classes

    virtual bool connect() = 0;
    virtual bool login(const std::string& username, const std::string& password) = 0;
    virtual std::string select_mailbox(const std::string& mailbox) = 0;
    virtual std::string search_mailbox(const std::string& criteria) = 0;
    virtual std::string fetch_message(int message_id) = 0;
    virtual std::string send_command(const std::string& command) = 0;
    virtual bool logout() = 0;

    virtual void disconnect() = 0;
    int message_id_counter_ = 0;

// protected:
//     int message_id_counter_ = 1;
};


#endif // BASECLIENT_H
