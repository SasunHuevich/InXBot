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
#include <SQLiteCpp/SQLiteCpp.h>
#include <atomic>

class SheetConnector{


private:
    std::string  spreadsheet_id; // хз зачем он нужен
    std::atomic<bool> running{true}; // Флаг для остановки потока

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


    std::string getJwtToken() {
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

        return jwt;
    }


    std::string getAcessToken() {
        CURL* curl = curl_easy_init();

        //TODO: Возможно нужно как-то обработать этот случай
        if(!curl) return "";
        std::string postData = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + getJwtToken();
        std::string readBuffer;

        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // для логирования
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Ошибка CURL: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return "";
        }
        curl_easy_cleanup(curl);

        rapidjson::Document doc_res;
        doc_res.Parse(readBuffer.c_str());

        if (!(doc_res.HasMember("access_token") && doc_res["access_token"].IsString())) {
            std::cerr << "Ошибка: Не найден access_token в ответе!" << std::endl;
            return "";
        }

        return doc_res["access_token"].GetString();
    }
    

    int getGoogleSheetRowCount() {
        return 0;
    };



    void updateDbWithSheets() {
        std::string accessToken = getAcessToken();
        std::string range = "clients!D:D";
        std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + SPREADSHEET_ID + "/values:batchGet?ranges=clients!D:D";

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
            
            CURLcode res = curl_easy_perform(curl);
            
            if (res != CURLE_OK) {
                std::cerr << "Ошибка CURL: " << curl_easy_strerror(res) << std::endl;
            } else {
                std::cout << "Ответ от Google Sheets API:\n" << readBuffer << std::endl;
            }
            
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
    }


    std::pair<int, std::string> getRowCount() {
        // Настройка cURL
        CURL* curl = curl_easy_init();
        if (curl) {
            std::string accessToken = getAcessToken();
            std::string range = "clients!D:D";
            std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + SPREADSHEET_ID + "/values:batchGet?ranges=" + range;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            
            CURLcode res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                rapidjson::Document doc;
                doc.Parse(readBuffer.c_str());

                if (doc.HasParseError()) {
                    std::cerr << "Ошибка парсинга JSON" << std::endl;
                    return {-1, ""};
                }

                if(doc.HasMember("valueRanges") && doc["valueRanges"].IsArray()) {
                    if(doc["valueRanges"][0].HasMember("values") && doc["valueRanges"][0]["values"].IsArray()) {
                        int size = doc["valueRanges"][0]["values"].Size();
                        return {size,  doc["valueRanges"][0]["values"][size - 1].GetString()};
                    }  
                }
                
                return {0, ""};
                
            } else {
                std::cerr << "Ошибка CURL: " << curl_easy_strerror(res) << std::endl;
            }
            
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
        return {-1, ""};
    }

    
    rapidjson::Value getOneRowArray(SQLite::Database& db, int id) {
        rapidjson::Document doc;
        auto& allocator = doc.GetAllocator();
        rapidjson::Value resultArray(rapidjson::kArrayType);
        int64_t user_id = db.getUserIdById(id);

        SQLite::Statement users_query(db, "SELECT * FROM users WHERE user_id = ?");
        users_query.bind(1, user_id);

        if (users_query.executeStep()) {
            resultArray.PushBack(users_query.getColumn(6).getText(), allocator);
            resultArray.PushBack(users_query.getColumn(2).getText(), allocator);
            resultArray.PushBack(users_query.getColumn(3).getText(), allocator);
            resultArray.PushBack(users_query.getColumn(2).getInt64(), allocator);

            if(users_query.getColumn(4).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(users_query.getColumn(4).getText(), allocator);
            }

            resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            resultArray.PushBack(rapidjson::Value().SetNull(), allocator);

            if(users_query.getColumn(5).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(users_query.getColumn(5).getInt64(), allocator);
            }   
        }

        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);

        SQLite::Statement easy_query(db, "SELECT api_key, secret_key FROM easy WHERE user_id = ?");
        easy_query.bind(1, user_id);

        if(easy_query.executeStep()) {
            if(easy_query.getColumn(0).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(easy_query.getColumn(0).getText(), allocator);
            } 

            if(easy_query.getColumn(1).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(easy_query.getColumn(1).getText(), allocator);
            } 
        }

        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        
        SQLite::Statement elite_query(db, "SELECT api_key, secret_key FROM elite WHERE user_id = ?");
        elite_query.bind(1, user_id);

        if(elite_query.executeStep()) {
            if(elite_query.getColumn(0).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(elite_query.getColumn(0).getText(), allocator);
            } 

            if(elite_query.getColumn(1).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(elite_query.getColumn(1).getText(), allocator);
            } 
        }

        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);

        SQLite::Statement spot_query(db, "SELECT api_key, secret_key FROM spot WHERE user_id = ?");
        spot_query.bind(1, user_id);

        if(spot_query.executeStep()) {
            if(spot_query.getColumn(0).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(spot_query.getColumn(0).getText(), allocator);
            } 

            if(spot_query.getColumn(1).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(spot_query.getColumn(1).getText(), allocator);
            } 
        }

        return resultArray;
    }


    void updateSheetsWithDb(SQLite::Database& db) {
        while(running) {
            int sheet_row_count = getRowCount().first - 2;
            int total_count = db.getTotalRowCount();

            if(sheet_row_count == -1 || totoal_count == -1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10000));
                continue;
            }

            if(sheet_row_count < total_count) {
                for(int i = sheet_row_count; i <= total_count; i++) {
                    getOneRowArray(db, i);
                }
            }
        }
    }


    /*void updateSheetsWithDb() {
        CURL* curl = curl_easy_init();
        if(curl) {
            const std::string accessToken = getAcessToken();
            const std::string range = "clients!A3:AD3";
            std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + SPREADSHEET_ID + "/values/" + range + "?valueInputOption=RAW";
            

            const std::string jsonData = R"({"range":"clients!A3:AD3","majorDimension":"ROWS","values":[["01.03.25","Артем Дадерко", "Tg", "id", "90", null, null, null, "80"]]})";

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

            CURLcode res = curl_easy_perform(curl);

            if(res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
                
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
    }*/

};


#endif // GOOGLE_SHEETS_CLIENT_H