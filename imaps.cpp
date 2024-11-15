#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include "imaps.h"
 

ImapsClient::ImapsClient(const std::string& server, int port, const std::string& cert_file, const std::string& cert_dir)
    :message_id_counter_(1), server_(server), port_(port), cert_file_(cert_file), cert_dir_(cert_dir), ctx_(nullptr), bio_(nullptr) {
 
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    
    ctx_ = SSL_CTX_new(TLS_client_method());
    if (!ctx_) {
        throw std::runtime_error("Failed to create SSL context.");
    }

    if (!cert_file_.empty() || !cert_dir_.empty()) {
        if (!SSL_CTX_load_verify_locations(ctx_, cert_file_.empty() ? nullptr : cert_file_.c_str(),
                                           cert_dir_.empty() ? nullptr : cert_dir_.c_str())) {
            throw std::runtime_error("Failed to load certificates.");
        }
    }
}

ImapsClient::~ImapsClient() {
    if (bio_) BIO_free_all(bio_);
    if (ctx_) SSL_CTX_free(ctx_);
    EVP_cleanup();
}

bool ImapsClient::connect() {
    std::string address = server_ + ":" + std::to_string(port_);
    bio_ = BIO_new_ssl_connect(ctx_);
    if (!bio_) {
        std::cerr << "Error creating BIO." << std::endl;
        return false;
    }

    BIO_set_conn_hostname(bio_, address.c_str());

    if (BIO_do_connect(bio_) <= 0) {
        std::cerr << "Failed to connect to server." << std::endl;
        return false;
    }

    if (BIO_do_handshake(bio_) <= 0) {
        std::cerr << "Failed to establish SSL connection." << std::endl;
        return false;
    }

    return true;
}

bool ImapsClient::login(const std::string& username, const std::string& password) {
    std::string command = "A" + std::to_string(message_id_counter_++) + " LOGIN " + username + " " + password + "\r\n";
    std::string response = send_command(command);
    return response.find("OK") != std::string::npos;
}


bool ImapsClient::logout() {
    std::string command = "A" + std::to_string(message_id_counter_++) + " LOGOUT\r\n";
    std::string response = send_command(command);
    return response.find("BYE") != std::string::npos && response.find("OK") != std::string::npos;
}

std::string ImapsClient::select_mailbox(const std::string& mailbox) {
    std::string command = "A" + std::to_string(message_id_counter_++) + " SELECT " + mailbox + "\r\n";
    return send_command(command);
}

std::string ImapsClient::search_mailbox(const std::string& criteria) {
    std::string command = "A" + std::to_string(message_id_counter_++) + " SEARCH " + criteria + "\r\n";
    return send_command(command);
}

std::string ImapsClient::fetch_message(int message_id, bool headers) {
    std::string command;
    if(headers){
        command = "A" + std::to_string(message_id_counter_++) + " FETCH " + std::to_string(message_id) + " BODY[HEADER]\r\n";
    }else{
        command = "A" + std::to_string(message_id_counter_++) + " FETCH " + std::to_string(message_id) + " BODY[]\r\n";
    }
    std::cout << message_id_counter_ << std::endl;//TODO DELETE
    return send_command(command);
}


void ImapsClient::disconnect() {
    if (bio_) {
        BIO_free_all(bio_);   // Free the BIO chain, including any underlying BIOs
        bio_ = nullptr;
    }

    if (ctx_) {
        SSL_CTX_free(ctx_);   // Free the SSL context to release resources
        ctx_ = nullptr;
    }

}

std::string ImapsClient::send_command(const std::string& command) {
    if (BIO_write(bio_, command.c_str(), command.size()) <= 0) {
        throw std::runtime_error("Failed to write command to SSL connection.");
    }

    std::string response;
    char buffer[4096];
    int bytes_read;
    while ((bytes_read = BIO_read(bio_, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        response += buffer;

        if (response.find("A" + std::to_string(message_id_counter_ - 1) + " OK") != std::string::npos ||
            response.find("NO") != std::string::npos ||
            response.find("BAD") != std::string::npos) {
            break;
        }
    }
    return response;
}

std::string ImapsClient::receive_response() {
    char buffer[4096];
    std::string response;

    int bytes_received;
    while ((bytes_received = BIO_read(bio_, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_received] = '\0';  
        response += buffer;
       
        if (response.find("\r\nA" + std::to_string(message_id_counter_ - 1) + " ") != std::string::npos) {
            break;
        }
    }

    if (bytes_received <= 0 && !BIO_should_retry(bio_)) {
        throw std::runtime_error("Failed to receive response from server");
    }

    return response;
}

