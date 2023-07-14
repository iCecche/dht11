#include <iostream>
#include <unistd.h>
#include <vector>
#include <memory>
#include "dht11.h"
using namespace std;

void calcMed(std::vector<processed_data>& historical, float &temperature, float& decimal_temperature);

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

            }
        }catch(std::runtime_error &err) {
            std::cout << "Impossibile determinare temperatura ambientale: prossimo tentativo in 60s..." << std::endl;
        }
        sleep(60);
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

