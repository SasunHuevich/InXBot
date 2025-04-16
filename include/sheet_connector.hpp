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
#include <database_connector.hpp>


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


    std::string normalizeDate(std::string date) {
        char sep = (date.find('.') != std::string::npos) ? '.' : '-';
        std::stringstream ss(date);
        std::string day;
        std::getline(ss, day, sep);
        std::string month;
        std::getline(ss, month, sep);
        std::string year;
        std::getline(ss, year, sep);
        
        if(year.length() == 2) {
            return "20" + year + "-" + month + "-" + day;
        }
        return year + "-" + month + "-" + day;
    }
    

    bool isStringNumeric(const std::string& str) {
        return !str.empty() && std::find_if(str.begin(), str.end(), [](char c) {
                   return !std::isdigit(c) && c != '.' && c != '-';}) == str.end();
    }


    double safeParseDouble(const rapidjson::Value& value) {
        double result = 0.0;
    
        if (value.IsNumber()) {
            result = value.GetDouble();
        } else if (value.IsString()) {
            std::string valStr = value.GetString();
            std::replace(valStr.begin(), valStr.end(), ',', '.');

            if (isStringNumeric(valStr)) {
                try {
                    result = std::stod(valStr);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Ошибка преобразования строки в double: " << valStr << " - " << e.what() << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Ошибка: строка слишком большая для double: " << valStr << " - " << e.what() << std::endl;
                }
            } else {
                std::cerr << "Некорректная строка для конвертации в double: " << valStr << std::endl;
            }
        } else {
            std::cerr << "Значение не является числом или строкой: " << value.GetType() << std::endl;
        }
    
        return result;
    }


    int64_t safeParseInt64(const rapidjson::Value& value) {
        int64_t result = 0;
        if (value.IsInt64()) {
            result = value.GetInt64();
        } else if (value.IsString()) {
            std::string valStr = value.GetString();
            try {
                result = std::stoll(valStr);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Ошибка преобразования строки в int64_t: " << valStr << " - " << e.what() << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Ошибка: строка слишком большая для int64_t: " << valStr << " - " << e.what() << std::endl;
            }
        } else if (value.IsUint64()) {
            result = static_cast<int64_t>(value.GetUint64());
        } else {
            std::cerr << "Значение не является целым числом, строкой или числом без знака: " << value.GetType() << std::endl;
        }
    
        return result;
    }


    void updateDbWithSheets(DatabaseConnector& db) {
        std::string accessToken = getAcessToken();
        int sheet_row_count = getRowCount().first;
        std::cout << "sheet " << sheet_row_count << std::endl;

        if(sheet_row_count == 2) {
            return;
        }

        std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + SPREADSHEET_ID + 
                          "/values:batchGet?ranges=clients!D3:D" + std::to_string(sheet_row_count) +
                          "&ranges=clients!F3:G" + std::to_string(sheet_row_count) +
                          "&ranges=clients!I3:J" + std::to_string(sheet_row_count) +
                          "&ranges=clients!M3:Q" + std::to_string(sheet_row_count) +
                          "&ranges=clients!T3:X" + std::to_string(sheet_row_count) +
                          "&ranges=clients!AA3:AC" + std::to_string(sheet_row_count);

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

                rapidjson::Document doc;
                auto& allocator = doc.GetAllocator();
                doc.Parse(readBuffer.c_str());
                if (!doc.IsObject() || !doc.HasMember("valueRanges")) {
                    std::cerr << "Неверный JSON-ответ." << std::endl;
                    return;
                }

                if(doc["valueRanges"][0].HasMember("values") && doc["valueRanges"][0]["values"].IsArray()) {
                    for(int i = 0; i < sheet_row_count - 2; i++) {
                        int64_t user_id = 0;
                        double balance = 0 ;
                        double comission = 0;

                        std::string date_easy = "1970-01-01";
                        std::string status_easy = "Отключен";
                        double total_easy = 0;
                        double profit_easy = 0;
                        double comission_easy = 0;

                        std::string date_elite = "1970-01-01";
                        std::string status_elite = "Отключен";
                        double total_elite = 0;
                        double profit_elite = 0;
                        double comission_elite = 0;

                        std::string date_spot = "1970-01-01";
                        std::string status_spot = "Отключен";
                        double total_spot = 0;
                        double profit_spot = 0;
                        double comission_spot = 0;

                        if(doc["valueRanges"][0]["values"][i].Empty()) {
                            continue;
                        }

                        if (i < doc["valueRanges"][0]["values"].Size() && doc["valueRanges"][0]["values"].IsArray() && doc["valueRanges"][0]["values"].Size() > 0) {
                            user_id = safeParseInt64(doc["valueRanges"][0]["values"][i][0]);
                        }
                        
                        if(doc["valueRanges"][1].HasMember("values") && doc["valueRanges"][1]["values"].IsArray()) {
                            if(i < doc["valueRanges"][1]["values"].Size()) {
                                auto& valRange = doc["valueRanges"][1]["values"][i];
                                if(valRange.IsArray() && !valRange.Empty()) {
                                    if(valRange[0].IsString() && std::string(valRange[0].GetString()).empty() == false) {
                                        balance = safeParseDouble(valRange[0]);
                                    }

                                    if(valRange[1].IsString() && std::string(valRange[1].GetString()).empty() == false) {
                                        comission = safeParseDouble(valRange[1]);
                                    }
                                }
                            }
                        }

                        if(doc["valueRanges"][2].HasMember("values") && doc["valueRanges"][2]["values"].IsArray()) {
                            if(i < doc["valueRanges"][2]["values"].Size()) {
                                auto& valRange = doc["valueRanges"][2]["values"][i];
                                if(valRange.IsArray() && !valRange.Empty()) {
                                    if(valRange[0].IsString() && std::string(valRange[0].GetString()).empty() == false) {
                                        date_easy = normalizeDate(valRange[0].GetString());
                                    }

                                    if(valRange[1].IsString() && std::string(valRange[1].GetString()).empty() == false) {
                                        status_easy = valRange[1].GetString();
                                    }
                                }
                            }
                        }

                        if(doc["valueRanges"][3].HasMember("values") && doc["valueRanges"][3]["values"].IsArray()) {
                            if(i < doc["valueRanges"][3]["values"].Size()) {
                                auto& valRange = doc["valueRanges"][3]["values"][i];
                                if(valRange.IsArray() && !valRange.Empty()) {
                                    if(valRange[0].IsString() && std::string(valRange[0].GetString()).empty() == false) {
                                        total_easy = safeParseDouble(valRange[0]);
                                    }

                                    if(valRange[1].IsString() && std::string(valRange[1].GetString()).empty() == false) {
                                        profit_easy = safeParseDouble(valRange[1]);
                                    }

                                    if(valRange[2].IsString() && std::string(valRange[2].GetString()).empty() == false) {
                                       comission_easy = safeParseDouble(valRange[2]);
                                    }

                                    if(valRange[3].IsString() && std::string(valRange[3].GetString()).empty() == false) {
                                        date_elite = normalizeDate(valRange[3].GetString());
                                    }

                                    if(valRange[4].IsString() && std::string(valRange[4].GetString()).empty() == false) {
                                        status_elite = valRange[4].GetString();
                                    }
                                }   
                            }  
                        }

                        if(doc["valueRanges"][4].HasMember("values") && doc["valueRanges"][4]["values"].IsArray()) {
                            if(i < doc["valueRanges"][4]["values"].Size()) {
                                auto& valRange = doc["valueRanges"][4]["values"][i];
                                if(valRange.IsArray() && !valRange.Empty()) {
                                    if(valRange[0].IsString() && std::string(valRange[0].GetString()).empty() == false) {
                                        total_elite = safeParseDouble(valRange[0]);
                                    }

                                    if(valRange[1].IsString() && std::string(valRange[1].GetString()).empty() == false) {
                                        profit_elite = safeParseDouble(valRange[1]);
                                    }

                                    if(valRange[2].IsString() && std::string(valRange[2].GetString()).empty() == false) {
                                       comission_elite = safeParseDouble(valRange[2]);
                                    }

                                    if(valRange[3].IsString() && std::string(valRange[3].GetString()).empty() == false) {
                                        date_spot = normalizeDate(valRange[3].GetString());
                                    }

                                    if(valRange[4].IsString() && std::string(valRange[4].GetString()).empty() == false) {
                                        status_spot = valRange[4].GetString();
                                    }
                                }   
                            }  
                        }

                        if(doc["valueRanges"][5].HasMember("values") && doc["valueRanges"][5]["values"].IsArray()) {
                            if(i < doc["valueRanges"][5]["values"].Size()) {
                                auto& valRange = doc["valueRanges"][5]["values"][i];
                                if(valRange.IsArray() && !valRange.Empty()) {
                                    if(valRange[0].IsString() && std::string(valRange[0].GetString()).empty() == false) {
                                        total_spot = safeParseDouble(valRange[0]);
                                    }

                                    if(valRange[1].IsString() && std::string(valRange[1].GetString()).empty() == false) {
                                        profit_spot = safeParseDouble(valRange[1]);
                                    }

                                    if(valRange[2].IsString() && std::string(valRange[2].GetString()).empty() == false) {
                                        comission_spot = safeParseDouble(valRange[2]);
                                    }
                                }   
                            }  
                        }

                        db.updateDb(
                            user_id, balance, comission,
                        
                            date_easy, status_easy, total_easy, profit_easy, comission_easy,
                        
                            date_elite, status_elite, total_elite, profit_elite, comission_elite,
                        
                            date_spot, status_spot,  total_spot, profit_spot, comission_spot
                        );
                    }
                }
                
                    
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

    
    rapidjson::Value getOneRowArray(DatabaseConnector& db, int id, rapidjson::Document::AllocatorType& allocator) {
        //rapidjson::Document doc;
        //auto& allocator = doc.GetAllocator();
        rapidjson::Value resultArray(rapidjson::kArrayType);
        int64_t user_id = db.getUserIdById(id);

        db.getUsersInfoForSheet(resultArray, allocator, user_id);

        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);

        db.getEasyInfoForSheet(resultArray, allocator, user_id);

        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        
        db.getEliteInfoForSheet(resultArray, allocator, user_id);

        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
        resultArray.PushBack(rapidjson::Value().SetNull(), allocator);

        db.getSpotInfoForSheet(resultArray, allocator, user_id);


        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        resultArray.Accept(writer);

        return resultArray;
    }


    void updateSheetsWithDb(DatabaseConnector& db) {
        while(running) {
            int sheet_row_count = getRowCount().first - 2;
            int total_count = db.getTotalRowCount();
            std::string range = "clients!A" + std::to_string(sheet_row_count + 2 + 1);

            if(sheet_row_count == -1 || total_count == -1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10000));
                continue;
            }

            if(sheet_row_count < total_count) {
                rapidjson::Document doc;
                doc.SetObject();
                auto& allocator = doc.GetAllocator();
                doc.AddMember("range", rapidjson::Value(range.c_str(), allocator), allocator);
                doc.AddMember("majorDimension", "ROWS", allocator);
                rapidjson::Value values(rapidjson::kArrayType);

                //TODO: сделать разбивку на партии по 200 человек
                for(int i = sheet_row_count + 1; i <= total_count; i++) {
                    values.PushBack(getOneRowArray(db, i, allocator), allocator);
                }
                doc.AddMember("values", values, allocator);

                uploadToGoogleSheet(range, doc);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(900000));
        }
    }


    std::string serializeJson(rapidjson::Document& doc) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }    


    void uploadToGoogleSheet(std::string range, rapidjson::Document& doc) {
        CURL* curl = curl_easy_init();
        //TODO: Наверное следует убрать вывод curl в stdout
        if(curl) {
            const std::string accessToken = getAcessToken();
            std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + SPREADSHEET_ID + "/values/" + range + "?valueInputOption=RAW";
            std::string jsonData = serializeJson(doc);

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