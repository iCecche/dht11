#include <iostream>
#include <unistd.h>
#include <vector>
#include <memory>
#include "twilio.h"
#include "dht11.h"
using namespace std;

void calcMed(std::vector<processed_data>& historical, float &temperature, float& decimal_temperature);
bool sendNotification(float& temperature);

int main(int argc, char * argv[]) {
    wiringPiSetup();
    DHT11 dht(0);
    std::vector<processed_data> historical;
    while (1) {
        int i = 0;
        while (i < 10) {
            processed_data misured_data = dht.misure();
            historical.push_back(misured_data);
            sleep(5);
            i++;
        }
        float integer_temperature, decimal_temperature;
        try {
            calcMed(historical, integer_temperature, decimal_temperature);
            float temperature = integer_temperature + (decimal_temperature / 10);
            if(temperature > 25.0) {
                std::cout << "ATTENZIONE: TEMPERATURA SOPRA LA NORMA!" << std::endl;
                std::cout << "VALORE TEMPERATURA : " << temperature << std::endl;
                if (!sendNotification(temperature)) {
                    std::cout << "Errore: impossibile inviare messaggio" << std::endl;
                }
            }
        }catch(std::runtime_error &err) {
            std::cout << "Impossibile determinare temperatura ambientale: prossimo tentativo in 60s..." << std::endl;
        }
        sleep(1800);
    }
    return 0;
}


void calcMed(std::vector<processed_data>& historical, float& integer_temperature, float& decimal_temperature) {
    float sumT = 0, sumDT = 0, valid_record = 0;
    for (auto record : historical) {
        if (record.temperature != -1 && record.decimal_temperature != -1) {
            sumT += record.temperature;
            sumDT += record.decimal_temperature;
            valid_record++;
        }
    }
    integer_temperature = sumT / valid_record;
    decimal_temperature = sumDT / valid_record;
    historical.clear();
    if(integer_temperature  == 0) {
        throw std::runtime_error("Error calculating temperature average!");
    }
}

bool sendNotification(float& temperature) {
    std::string account_sid = "<YOUR_ACCOUNT_SID>";
    std::string auth_token = "<YOUR_ACCESS_TOKEN>";
    std::string message = "Alert! Temperature is " + to_string(temperature) + "°C";
    std::string from_number = "<FROM_TWILIO_NUMBER>";
    std::string to_number = "<TO_NUMBER>";
    std::string picture_url;
    bool verbose = false;

    std::string response;
    auto twilio = std::make_shared<twilio::Twilio>(
            account_sid,
            auth_token
    );

    bool message_success = twilio->send_message(
            to_number,
            from_number,
            message,
            response,
            picture_url,
            verbose
    );

    if (!message_success) {
        std::cout << "Message send failed." << std::endl;
        if (!response.empty()) {
            std::cout << "Response:" << std::endl << response << std::endl;
        }
        return false;
    }else {
        std::cout << "SMS sent successfully!" << std::endl;
        std::cout << "Response:" << std::endl << response << std::endl;
        return true;
    }
}
