//
// Created by ALESSIO CECCHERINI on 15/07/23.
//

#ifndef DHT11_TWILIO_H
#define DHT11_TWILIO_H

#include <iostream>

namespace twilio {

    class Twilio {
    public:
        Twilio(const std::string &account_sid_in, const std::string &auth_token_in);
        bool send_message(const std::string &to_number, const std::string &from_number, const std::string &message_body,
                          std::string &response, const std::string &picture_url, bool verbose);
    private:

        std::string const account_sid;
        // Used for the password of the auth header
        std::string const auth_token;

        // Portably ignore curl response
        static size_t _null_write(char *, size_t, size_t, void *);

        // Write curl response to a stringstream
        static size_t _stream_write(char *, size_t, size_t, void *);

    };

}


#endif //DHT11_TWILIO_H
