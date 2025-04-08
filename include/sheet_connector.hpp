#ifndef GOOGLE_SHEETS_CLIENT_H
#define GOOGLE_SHEETS_CLIENT_H

#include <iostream>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <sstream>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>
#include <traits/nlohmann-json/defaults.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <config.hpp>

class SheetConnector{


private:
    std::string  spreadsheet_id; // хз зачем он нужен

public:
    SheetConnector(const std::string& spreadsheet_id) : spreadsheet_id(spreadsheet_id) {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~SheetConnector() {
        curl_global_cleanup();
    }


    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        output->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    
    rapidjson::Document loadServiceAccountKey() {
        std::ifstream file(SERVICE_ACCOUNT_FILE);
        std::stringstream buffer;
        buffer << file.rdbuf();
        rapidjson::Document doc;
        doc.Parse(buffer.str().c_str());
        return doc;
    }


    std::string getAcessToken() {
        rapidjson::Document doc = loadServiceAccountKey();
        std::string privateKey = doc["private_key"].GetString();
        std::string clientEmail = doc["client_email"].GetString();

        auto now = std::chrono::system_clock::now();
        auto exp = now + std::chrono::minutes(60);
        auto rsa = jwt::algorithm::rs256("", privateKey, "", "RS256");

        std::string jwt = jwt::create()
        .set_issuer(clientEmail)
        .set_audience("https://oauth2.googleapis.com/token")
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_payload_claim("scope", jwt::claim(std::string("https://www.googleapis.com/auth/spreadsheets")))
        .set_type("JWT")
        .set_algorithm("RS256")
        .sign(rsa);

        std::cout << "Ключ: " << privateKey.substr(0, 50) << "..." << std::endl;
        std::cout << "JWT: " << jwt << std::endl;
        CURL* curl = curl_easy_init();

        //TODO: Возможно нужно как-то обработать этот случай
        if(!curl) return "";
        std::string postData = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + jwt;
        std::string readBuffer;

        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // для логирования
       
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        // Выполнение запроса
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Ошибка CURL: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return "";
        }
        curl_easy_cleanup(curl);

        // Парсинг ответа
        rapidjson::Document doc_res;
        doc_res.Parse(readBuffer.c_str());
        std::cout << "Ответ сервера:\n" << readBuffer << std::endl;

        if (!(doc_res.HasMember("access_token") && doc_res["access_token"].IsString())) {
            std::cerr << "Ошибка: Не найден access_token в ответе!" << std::endl;
            return "";
        }

        return doc_res["access_token"].GetString();
    }


    void updateDbWithSheets() {
        std::string accessToken = getAcessToken();
        std::string range = "Лист1!A1:B2";

        // URL для чтения данных с Google Sheets API
        std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + SPREADSHEET_ID + "/values/" + range;

        // Настройка cURL
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            
            // Выполняем запрос
            CURLcode res = curl_easy_perform(curl);
            
            if (res != CURLE_OK) {
                std::cerr << "Ошибка CURL: " << curl_easy_strerror(res) << std::endl;
            } else {
                // Обрабатываем ответ
                std::cout << "Ответ от Google Sheets API:\n" << readBuffer << std::endl;
            }
            
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
    }

};


#endif // GOOGLE_SHEETS_CLIENT_H