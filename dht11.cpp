//
// Created by ALESSIO CECCHERINI on 15/07/23.
//

#include <iostream>
#include <unistd.h>
#include <chrono>
#include <curl/curl.h>
#include <wiringPi.h>
#include "dht11.h"

using namespace std::chrono;

DHT11::DHT11(int pin) : gpio(pin) {
    data = -1;
    misured_data = -1;

}

void DHT11::initPin() const {
    pinMode(gpio,OUTPUT);
    digitalWrite(gpio, LOW);
    usleep(20000); //wait 20mills
    digitalWrite(gpio, HIGH);
    usleep(40); //wait 40micros
    pinMode(gpio, INPUT);
}

struct processed_data DHT11::misure() {
    initPin();
    data = 0;
    try {
        waitResponse(1);
        waitResponse(0);
        for (int i = 0; i < 40; i++) {
            data <<= 1;
            waitResponse(0);
            int high_time = waitResponse(1);
            if (50 < high_time) {
                data |= 0x1;
            }
        }
        waitResponse(0);
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

long int DHT11::waitResponse(int state) const {

    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);
    int count = 10000;

    while (digitalRead(gpio) == state){
        if(count == 0) {
            throw std::runtime_error("Timeout while waiting for pin to get response (low or high)");
        }
        count--;
    }

    clock_gettime(CLOCK_REALTIME, &end);
    return (end.tv_nsec - begin.tv_nsec) / 1000;
}

void DHT11::print() const{
    std::cout << "Nuova misurazione in corso..." << std::endl;
    std::cout << "Temp: " << misured_data.temperature << "." << misured_data.decimal_temperature << " °C" <<std::endl;
    std::cout << "Hum: " << misured_data.humidity <<  "." << misured_data.decimal_humidity << " %" << std::endl;
}

//calcola temperatura media dopo 5 misurazioni salvate sul vettore "historical" svuotandolo
void DHT11::calcTempMedia(std::vector<processed_data>& historical, float& integer_temperature, float& decimal_temperature) {

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

//Invia messaggi tramite whatsapp api (business account)
bool DHT11::sendSMS(std::string temperature) {

    CURL* curl;
    CURLcode res;

    std::string sendTo = "<SEND_TO_NUMBER>";
    std::string url = "https://graph.facebook.com/v17.0/<NUMBER_ID>/messages";
    char* accessToken = "Authorization: Bearer <ACCESS_TOKEN>";
    std::string jsonData = R"({ "messaging_product": "whatsapp", "to": )" + sendTo + R"(, "type": "text", "text": { "preview_url": false, "body": "Alert! Temperature is: )" +
                           temperature + R"( C" } } )";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, accessToken);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Error performing POST request: " << curl_easy_strerror(res) << std::endl;
            throw std::runtime_error("Error performing POST request during Whatsapp send message action");
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return true;
}

bool DHT11::sendMail(std::string temperature) {

    CURL *curl;
    CURLcode res;
    curl_mime *mime1;
    curl_mimepart *part1;

    mime1 = nullptr;

    temperature = "Temperature is: " + temperature + " °C";
    auto mess = temperature.c_str();

    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.mailgun.net/v3/<MAIL_DOMAIN>.mailgun.org/messages");
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERPWD, "api:<YOUR_APIKEY>");
        mime1 = curl_mime_init(curl);
        part1 = curl_mime_addpart(mime1);
        curl_mime_data(part1, "Temperature Station <mailgun@<YOUR_DOMAIN>.mailgun.org>", CURL_ZERO_TERMINATED);
        curl_mime_name(part1, "from");
        part1 = curl_mime_addpart(mime1);
        curl_mime_data(part1, "<SEND_TO_EMAIL>", CURL_ZERO_TERMINATED);
        curl_mime_name(part1, "to");
        part1 = curl_mime_addpart(mime1);
        curl_mime_data(part1, "Temperature Alert!", CURL_ZERO_TERMINATED);
        curl_mime_name(part1, "subject");
        part1 = curl_mime_addpart(mime1);
        curl_mime_data(part1, mess , CURL_ZERO_TERMINATED);
        curl_mime_name(part1, "text");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime1);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.87.0");
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    }else {
        throw std::runtime_error("Error sending email with mailgun api!");
    }

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "Error: " <<curl_easy_strerror(res) << std::endl;
        return false;
    }

    curl_easy_cleanup(curl);
    curl_mime_free(mime1);
    curl_global_cleanup();
    return true;
}