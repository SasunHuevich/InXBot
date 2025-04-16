#ifndef DATABASECONNECTOR_HPP
#define DATABASECONNECTOR_HPP

#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <filesystem>
#include <atomic>
#include <mutex>
#include <deque>
#include <stack>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>


class DatabaseConnector {


private:
    SQLite::Database db;
    std::atomic<bool> running{true}; // Флаг для остановки потока
    std::mutex queueMutex;
    std::deque<std::function<void()>> operations;

public:
    DatabaseConnector(const std::string& dbName, int openFlags) : db(dbName, openFlags){
        std::cout << "База данных '" << dbName << "' открыта" << std::endl;
    }

    
    void createTableUsers() {
        std::string tableName = "users";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "name TEXT,"
                             "username TEXT,"
                             "phone TEXT," 
                             "balance DOUBLE DEFAULT NULL,"
                             "comission DOUBLE DEFAULT NULL,"
                             "referrer_id INTEGER DEFAULT NULL,"
                             "date_registration TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
    }


    void createTableEASY() {
        std::string tableName = "easy";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "date_connection TIMESTAMP,"
                             "status TEXT,"
                             "api_key TEXT," 
                             "secret_key TEXT,"   
                             "total DOUBLE DEFAULT NULL,"
                             "profit DOUBLE DEFAULT NULL,"
                             "comission DOUBLE DEFAULT NULL);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
    }


    void createTableELITE() {
        std::string tableName = "elite";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "date_connection TIMESTAMP,"
                             "status TEXT,"
                             "api_key TEXT," 
                             "secret_key TEXT,"   
                             "total DOUBLE DEFAULT NULL,"
                             "profit DOUBLE DEFAULT NULL,"
                             "comission DOUBLE DEFAULT NULL);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
    }


    void createTableSPOT() {
        std::string tableName = "spot";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "date_connection TIMESTAMP,"
                             "status TEXT,"
                             "api_key TEXT," 
                             "secret_key TEXT,"   
                             "total DOUBLE DEFAULT NULL,"
                             "profit DOUBLE DEFAULT NULL,"
                             "comission DOUBLE DEFAULT NULL);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
    }


    void createTableReferrers() {
        std::string tableName = "referrers";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "level_1 INTEGER DEFAULT 0,"
                             "level_2 INTEGER DEFAULT 0," 
                             "level_3 INTEGER DEFAULT 0,"
                             "level_4 INTEGER DEFAULT 0,"
                             "level_5 INTEGER DEFAULT 0);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
    }
    

    void insertUser(int64_t user_id, const std::string& name, const std::string& username, std::optional<int64_t> referrer_id = std::nullopt) {
        std::string queryStr = "INSERT INTO users (user_id, name, username, referrer_id) VALUES (?, ?, ?, ?)";
        SQLite::Statement query(db, queryStr);

        query.bind(1, user_id);
        query.bind(2, name);
        query.bind(3, username);
        if(referrer_id.has_value()) {
            query.bind(4, referrer_id.value());
        }
        else{
            query.bind(4);
        }
        
        query.exec();
    }


    void insertIntoReferrals(int64_t user_id) {
        std::string queryStr = "INSERT INTO referrers (user_id) VALUES (?)";
        SQLite::Statement query(db, queryStr);

        query.bind(1, user_id);
        query.exec();
    }



    void insertIntoSpot(int64_t user_id) {
        std::string queryStr = "INSERT INTO spot (user_id) VALUES (?)";
        SQLite::Statement query(db, queryStr);

        query.bind(1, user_id);
        query.exec();
    }



    void insertIntoElite(int64_t user_id) {
        std::string queryStr = "INSERT INTO elite (user_id) VALUES (?)";
        SQLite::Statement query(db, queryStr);

        query.bind(1, user_id);
        query.exec();
    }


    void insertIntoEasy(int64_t user_id) {
        std::string queryStr = "INSERT INTO easy (user_id) VALUES (?)";
        SQLite::Statement query(db, queryStr);

        query.bind(1, user_id);
        query.exec();
    }

    void updateReferrersLevels(int64_t user_id, int level = 1) {
        if (level < 1 || level > 5) {
            std::cerr << "Ошибка: Уровень должен быть от 1 до 5." << std::endl;
            return;
        }

        SQLite::Statement query(db, "SELECT level_" + std::to_string(level) + " FROM referrers WHERE user_id = ?");
        query.bind(1, user_id);
        
        int current_level = 0;
        if (query.executeStep()) {
            int columns = query.getColumnCount();
            std::cout << "Columns returned: " << columns << std::endl;
            current_level = query.getColumn(0).getInt();
        } else {
            std::cerr << "Реферер с user_id = " << user_id << " не найден." << std::endl;
            return;
        }

        current_level++;
        
        SQLite::Statement update_query(db, "UPDATE referrers SET level_" + std::to_string(level) + " = ? WHERE user_id = ?");
        update_query.bind(1, current_level);
        update_query.bind(2, user_id);
        update_query.exec();
        
        int64_t new_user_id = getReferrerByUserId(user_id);
        if(new_user_id != -1) {
            level++;
            updateReferrersLevels(new_user_id, level);
        }
    }


    int64_t getReferrerByUserId(int64_t user_id) {
        try {
            int64_t referrer_id;
            SQLite::Statement query(db, "SELECT * FROM users WHERE user_id = ?");
            query.bind(1, user_id);
            
            if(query.executeStep()){
                if (!query.getColumn(4).isNull()) {
                    referrer_id = query.getColumn(4).getInt64();  // Получаем referrer_id из 5-го столбца
                } else {
                    std::cerr << "Реферер для user_id = " << user_id << " равен NULL." << std::endl;
                    return -1;
                }
            }
            else{
                return -1;
            }
            

            return referrer_id;

        } catch (const SQLite::Exception& e) {
            std::cerr << "Ошибка SQLite: " << e.what() << std::endl;
            return -1;
        }
        
    }


    int64_t getUserIdById(int id) {
        SQLite::Statement query(db, "SELECT * FROM users WHERE id = ?");
        query.bind(1, id);

        if (query.executeStep()) {
            return query.getColumn(1).getInt64();
        }

        return -1;
    }


    std::string getUsernameById(int64_t user_id){
        std::string username;
        SQLite::Statement query(db, "SELECT * FROM users WHERE user_id = ?");
        query.bind(1, user_id);

        if (query.executeStep()) {
            username = query.getColumn(2).getText();
        } else {
            std::cerr << "Пользователь с ID " << user_id << " не найден!" << std::endl;
        }

        return username;
    }


    bool doesUserExist(int64_t user_id){
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE user_id = ?;");
        query.bind(1, user_id);

        if (query.executeStep()) {
            int count = query.getColumn(0);
            return count > 0;
        }

        return false;
    }


    /*int countReferrals(int64_t referrer_id){
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE referrer_id = ?;");
        query.bind(1, referrer_id);

        if (query.executeStep()) {
            return query.getColumn(0).getInt();
        }
    
        return 0; 
    }*/
    

    int getTotalRowCount() {
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users;");

        if (query.executeStep()) {
            int count = query.getColumn(0);
            return count;
        }
        
        return -1;
    }


    std::string unnormalizeDate(const std::string& date) {
        char sep = '-';
        std::stringstream ss(date);
        std::string year;
        std::getline(ss, year, sep);
        std::string month;
        std::getline(ss, month, sep);
        std::string day;
        sep = ' ';
        std::getline(ss, day, sep);
        
        std::cout << day + "." + month + "." + year.substr(2) << std::endl;
        return day + "." + month + "." + year.substr(2);
    }


    void getUsersInfoForSheet(rapidjson::Value& resultArray, rapidjson::Document::AllocatorType& allocator, int64_t user_id) {
        SQLite::Statement users_query(db, "SELECT * FROM users WHERE user_id = ?");
        users_query.bind(1, user_id);

        if (users_query.executeStep()) {
            resultArray.PushBack(rapidjson::Value(unnormalizeDate(users_query.getColumn(8).getText()).c_str(), allocator), allocator);
            resultArray.PushBack(rapidjson::Value(users_query.getColumn(2).getText(), allocator), allocator);
            resultArray.PushBack(rapidjson::Value(users_query.getColumn(3).getText(), allocator), allocator);
            resultArray.PushBack(rapidjson::Value().SetInt64(users_query.getColumn(1).getInt64()), allocator);

            if(users_query.getColumn(4).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value(users_query.getColumn(4).getText(), allocator), allocator);
            }

            resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            resultArray.PushBack(rapidjson::Value().SetNull(), allocator);

            if(users_query.getColumn(7).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value().SetInt64(users_query.getColumn(7).getInt64()), allocator);
            }   
        }
    }


    void getEasyInfoForSheet(rapidjson::Value& resultArray, rapidjson::Document::AllocatorType& allocator, int64_t user_id) {
        SQLite::Statement easy_query(db, "SELECT api_key, secret_key FROM easy WHERE user_id = ?");
        easy_query.bind(1, user_id);

        if(easy_query.executeStep()) {
            if(easy_query.getColumn(0).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value(easy_query.getColumn(0).getText(), allocator), allocator);
            } 

            if(easy_query.getColumn(1).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value(easy_query.getColumn(1).getText(), allocator), allocator);
            } 
        }
    }


    void getEliteInfoForSheet(rapidjson::Value& resultArray, rapidjson::Document::AllocatorType& allocator, int64_t user_id) {
        SQLite::Statement elite_query(db, "SELECT api_key, secret_key FROM elite WHERE user_id = ?");
        elite_query.bind(1, user_id);

        if(elite_query.executeStep()) {
            if(elite_query.getColumn(0).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value(elite_query.getColumn(0).getText(), allocator), allocator);
            } 

            if(elite_query.getColumn(1).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value(elite_query.getColumn(1).getText(), allocator), allocator);
            } 
        }
    }


    void getSpotInfoForSheet(rapidjson::Value& resultArray, rapidjson::Document::AllocatorType& allocator, int64_t user_id) {
        SQLite::Statement spot_query(db, "SELECT api_key, secret_key FROM spot WHERE user_id = ?");
        spot_query.bind(1, user_id);

        if(spot_query.executeStep()) {
            if(spot_query.getColumn(0).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value(spot_query.getColumn(0).getText(), allocator), allocator);
            } 

            if(spot_query.getColumn(1).isNull()) {
                resultArray.PushBack(rapidjson::Value().SetNull(), allocator);
            }
            else{
                resultArray.PushBack(rapidjson::Value(spot_query.getColumn(1).getText(), allocator), allocator);
            } 
        }
    }


    //TODO: Метод для бэкапов


    void newUser(int64_t user_id, const std::string& name,const std::string& username, std::optional<int64_t> referrer_id = std::nullopt) {
        queueMutex.lock();

        operations.push_back([this, user_id, name, username, referrer_id]() {
            insertUser(user_id, name, username, referrer_id);
        });
        
        operations.push_back([this, user_id]() {
            insertIntoReferrals(user_id);
        });

        operations.push_back([this, user_id]() {
            insertIntoEasy(user_id);
        });

        operations.push_back([this, user_id]() {
            insertIntoSpot(user_id);
        });

        operations.push_back([this, user_id]() {
            insertIntoElite(user_id);
        });

        operations.push_back([this, user_id]() {
            updateReferrersLevels(user_id);
        });
        
        queueMutex.unlock();
    }


    void updateUser(int64_t user_id, double balance, double comission) {
        SQLite::Statement users(db, "UPDATE users SET balance = ?, comission = ? WHERE user_id = ?");

        users.bind(1, balance);
        users.bind(2, comission);
        users.bind(3, user_id);

        users.exec();  
    }


    void updateEasy(int64_t user_id, std::string date_connection, std::string status, double total, double profit, double comission) {
        SQLite::Statement easy(db, "UPDATE easy date_connection = ?, status = ?, total = ?, profit = ?, comission = ? WHERE user_id = ?");

        easy.bind(1, date_connection);
        easy.bind(2, status);
        easy.bind(3, total);
        easy.bind(4, profit);
        easy.bind(5, comission);
        easy.bind(6, user_id);

        easy.exec();
    }


    void updateElite(int64_t user_id, std::string date_connection, std::string status, double total, double profit, double comission) {
        SQLite::Statement elite(db, "UPDATE elite date_connection = ?, status = ?, total = ?, profit = ?, comission = ? WHERE user_id = ?");

        elite.bind(1, date_connection);
        elite.bind(2, status);
        elite.bind(3, total);
        elite.bind(4, profit);
        elite.bind(5, comission);
        elite.bind(6, user_id);

        elite.exec();
    }


    void updateSpot(int64_t user_id, std::string date_connection, std::string status, double total, double profit, double comission) {
        SQLite::Statement spot(db, "UPDATE spot date_connection = ?, status = ?, total = ?, profit = ?, comission = ? WHERE user_id = ?");

        spot.bind(1, date_connection);
        spot.bind(2, status);
        spot.bind(3, total);
        spot.bind(4, profit);
        spot.bind(5, comission);
        spot.bind(6, user_id);

        spot.exec();
    }
    


    void updateDb(int64_t user_id, double balance, double comission, 
        std::string date_connection_easy, std::string status_easy, double total_easy, double profit_easy, double comission_easy, 
        std::string date_connection_elite, std::string status_elite, double total_elite, double profit_elite, double comission_elite, 
        std::string date_connection_spot, std::string status_spot, double total_spot, double profit_spot, double comission_spot) {

        queueMutex.lock();

        operations.push_back([this, user_id, balance, comission]() {
            updateUser(user_id, balance, comission);
        });

        operations.push_back([this, user_id, date_connection_easy, status_easy, total_easy, profit_easy, comission_easy]() {
            updateEasy(user_id, date_connection_easy, status_easy, total_easy, profit_easy, comission_easy);
        });

        operations.push_back([this, user_id, date_connection_elite, status_elite, total_elite, profit_elite, comission_elite]() {
            updateElite(user_id, date_connection_elite, status_elite, total_elite, profit_elite, comission_elite);
        });

        operations.push_back([this, user_id, date_connection_spot, status_spot, total_spot, profit_spot, comission_spot]() {
            updateSpot(user_id, date_connection_spot, status_spot, total_spot, profit_spot, comission_spot);
        });

        queueMutex.unlock();
    }


    void executeBatchInsert() {
        std::stack<std::function<void()>> operations_stack;
        try{
            while(running) {
                queueMutex.lock();

                if (operations.empty()) {
                    queueMutex.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                db.exec("BEGIN TRANSACTION;");

                size_t batchSize = 100;
                size_t count = 0;

                while (!operations.empty() && count < batchSize) {
                    try{
                        operations.front()(); // Выполнение операции
                        operations_stack.push(operations.front());
                        operations.pop_front();
                        count++;
                    } catch (const SQLite::Exception& e) {
                        std::string errorMsg = e.what();
    
                        if (errorMsg.find("UNIQUE constraint failed") != std::string::npos) {
                            std::cerr << "[executeBatchInsert][UNIQUE constraint violation] Skipping operation: " << errorMsg << std::endl;
                            operations_stack.pop();
                            continue;
                        }
                        else {
                            db.exec("ROLLBACK;");

                            while (!operations_stack.empty()) {
                                operations.emplace_front(operations_stack.top());
                                operations_stack.pop();
                            }

                            queueMutex.unlock();
                            std::cerr << "[executeBatchInsert][SQLite Error] " << errorMsg << std::endl;
                        }
                        
                        
                    }
                    
                }

                db.exec("COMMIT;");
                queueMutex.unlock();
                
                std::cout << "NEW COMMIT" << std::endl;

                while (!operations_stack.empty()) {
                    operations_stack.pop();
                }
            }
        } catch (const std::exception& e) {
            queueMutex.unlock();
            std::cerr << "[executeBatchInsert][Unexpected Error] " << e.what() << std::endl;
        }
    }
};

#endif