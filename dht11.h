//
// Created by ALESSIO CECCHERINI on 15/07/23.
//

#ifndef DHT11_DHT11_H
#define DHT11_DHT11_H


#include <cstdint>
#include <vector>

struct processed_data {
    int humidity;
    int decimal_humidity;
    int temperature;
    int decimal_temperature;

    processed_data& operator=(int const& num) {
        humidity = num;
        decimal_humidity = num;
        temperature = num;
        decimal_temperature = num;
        return *this;
    }
};

class DHT11 {
public:
    explicit DHT11(int pin);
    DHT11(const DHT11& sensor) = delete;
    struct processed_data misure();
private:

    void initPin();
    long int waitLow();
    long int waitHigh();
    void readData();
    void print();
    int controlData(int hh, int lh, int ht, int lt);

    int64_t data;
    processed_data misured_data;
    int gpio;
};


#endif //DHT11_DHT11_H
