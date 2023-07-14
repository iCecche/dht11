#include <iostream>
#include <unistd.h>
#include <sstream>
#include <cmath>
#include "dht11.h"

using namespace std;

int main() {

    wiringPiSetup();
    DHT11 dht(0);
    std::vector<processed_data> historical;

    while (1) {

        int i = 0;
        while (i < 12) {
            processed_data misured_data = dht.misure();
            historical.push_back(misured_data);
            sleep(5);
            i++;
        }

        float integer_temperature, decimal_temperature;
        try {

            dht.calcTempMedia(historical, integer_temperature, decimal_temperature);
            float temperature = integer_temperature + (decimal_temperature / 10);
            temperature = round(temperature * 10) / 10; //round to first number after decimal

            if(temperature > 25.0) {

                std::cout << "ATTENZIONE: TEMPERATURA SOPRA LA NORMA!" << std::endl;
                std::cout << "VALORE TEMPERATURA : " << temperature << std::endl;

                //converti float in string mantenendo inalterato il numero di cifre dopo la virgola
                std::ostringstream os;
                os << temperature;
                dht.sendSMS(os.str());
                dht.sendMail(os.str());
            }

        }catch(std::runtime_error &err) {
            std::cout << err.what() << std::endl;
            std::cout << "Prossimo tentativo in 60s..." << std::endl;
        }
        sleep(1800);
    }
    return 0;
}


