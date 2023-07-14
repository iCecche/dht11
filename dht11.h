//
// Created by ALESSIO CECCHERINI on 15/07/23.
//

#ifndef DHT11_DHT11_H
#define DHT11_DHT11_H

#include <cstdint>
#include <vector>
#include <wiringPi.h>

//dati forniti dal sensore
struct processed_data {

    int humidity;
    int decimal_humidity;
    int temperature;
    int decimal_temperature;

    //overload per assegnare tutte le variabili a -1 in caso di errore con singola istruzione
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

    void calcTempMedia(std::vector<processed_data>& historical, float &temperature, float& decimal_temperature);
    bool sendSMS(std::string temperature);
    bool sendMail(std::string temperature);

private:

    // inizzializza porta pin del dht11 secondo datasheet
    void initPin() const;

    // calcola tempo di risposta di dht11 mentre passa da stato LOW a HIGH e viceversa
    // da hardware dht11 ha tempo low 50micros: se high > low allora n-esimo bit 1 altrimenti 0
    // dht11 fornisce il risultato della misurazione un bit secondo il precedente schema fino a inviare
    // tutti e 40 bit: 8 + 8 umidità (valore intero + decimale), 8 + 8 temperatura (valore intero + decimale) e 8 "parity bit"
    long int waitResponse(int state) const;

    // verifica integrità dei dati ricevuti e li assegna alle variabili
    void readData();

    // somma bit ricevuti per confronto con parity bit
    int controlData(int hh, int lh, int ht, int lt);

    // stampa a video valore attributi dht11
    void print() const;

    // 64bit int to store 40bit response
    int64_t data;

    // struct to store hum and temp value
    processed_data misured_data;

    // gpio port: not physical number but wiringPi number
    int gpio;
};



#endif //DHT11_DHT11_H
