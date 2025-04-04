#ifndef GOOGLE_SHEETS_CLIENT_H
#define GOOGLE_SHEETS_CLIENT_H

#include <iostream>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <fstream>
#include <sstream>
#include <jwt-cpp/jwt.h>


class SheetConnector{


private:
    std::string api_key;
    std::string  spreadsheet_id; // хз зачем он нужен

public:
    SheetConnector(const std::string& api_key, const std::string& spreadsheet_id) : api_key(api_key), spreadsheet_id(spreadsheet_id) {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~SheetConnector() {
        curl_global_cleanup();
    }


    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        output->append((char*)contents, size * nmemb);
    }

    
    rapidjson::Document loadServiceAccountKey(const std::string& filename) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        rapidjson::Document doc;
        doc.Parse(buffer.str().c_str());
        return doc;
    }


    std::string getRangeData(const std::string& range) {
        CURL* curl = curl_easy_init(); // По сути дескриптор
        std::string readBuffer;

        if(curl) {
            std::string url = "";
        }
    }

};


#endif // GOOGLE_SHEETS_CLIENT_H