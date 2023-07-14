//
// Created by ALESSIO CECCHERINI on 15/07/23.
//

#include "dht11.h"
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <wiringPi.h>

using namespace std::chrono;

DHT11::DHT11(int pin) : gpio(pin) {
    data = -1;
    misured_data = -1;

}

void DHT11::initPin() {
    pinMode(gpio,OUTPUT);
    digitalWrite(gpio, LOW);
    usleep(20000);
    digitalWrite(gpio, HIGH);
    usleep(40);
    pinMode(gpio, INPUT);
}

struct processed_data DHT11::misure() {
    initPin();
    data = 0;
    try {
        waitLow();
        waitHigh();
        for (int i = 0; i < 40; i++) {
            data <<= 1;
            waitHigh();
            int high_time = waitLow();
            if (50 < high_time) {
                data |= 0x1;
            }
        }
        waitHigh();
        readData();
    }catch(...) {
        std::cout << "Errore nella misurazione..." << std::endl;
        misured_data = -1;
        pinMode(gpio, OUTPUT);
        digitalWrite(gpio, LOW);
    }
    pinMode(gpio, OUTPUT);
    digitalWrite(gpio, HIGH);
    return misured_data;
}

void DHT11::readData() {
    int high_humidity = (data >> 32) & 0xFF;
    int low_humidity = (data >> 24) & 0xFF;
    int high_temperature = (data >> 16) & 0xFF;
    int low_temperature = (data >> 8) & 0xFF;
    int totalBit = data & 0xFF;

    if (totalBit == controlData(high_humidity,low_humidity, high_temperature, low_temperature)) {
        misured_data.temperature = high_temperature;
        misured_data.decimal_temperature = low_temperature;
        misured_data.humidity = high_humidity;
        misured_data.decimal_humidity = low_humidity;
        print();
    }else {
        std::cout << "Error: corrupted data!" << std::endl;
        misured_data = -1;
    }
}

int DHT11::controlData(int hh, int lh, int ht, int lt) {
    return static_cast<int>(hh + ht + lh + lt);
}

long int DHT11::waitHigh() {
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);
    int count = 10000;
    while (digitalRead(gpio) == 0) {
        if(count == 0) {
            throw std::runtime_error("Timeout while waiting for pin to get high");
        }
        count--;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    return (end.tv_nsec - begin.tv_nsec) / 1000 ;
}

long int DHT11::waitLow() {
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);
    int count = 10000;
    while (digitalRead(gpio) == 1){
        if(count == 0) {
            throw std::runtime_error("Timeout while waiting for pin to get low");
        }
        count--;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    return (end.tv_nsec - begin.tv_nsec) / 1000;
}

void DHT11::print() {
    std::cout << "Nuova misurazione in corso..." << std::endl;
    std::cout << "Temp: " << misured_data.temperature << "." << misured_data.decimal_temperature << " °C" <<std::endl;
    std::cout << "Hum: " << misured_data.humidity <<  "." << misured_data.decimal_humidity << " %" << std::endl;
}