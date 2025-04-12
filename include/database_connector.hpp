#ifndef DATABASECONNECTOR_HPP
#define DATABASECONNECTOR_HPP

#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <filesystem>
#include <atomic>
#include <mutex>
#include <deque>
#include <stack>


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
                             "phone TEXT,"  
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
                             "api_key TEXT," 
                             "secret_key TEXT,"   
                             "total INTEGER DEFAULT NULL,"
                             "profit INTEGER DEFAULT NULL,"
                             "comission INTEGER DEFAULT NULL);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
    }


    void createTableELITE() {
        std::string tableName = "elite";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "api_key TEXT," 
                             "secret_key TEXT,"   
                             "total INTEGER DEFAULT NULL,"
                             "profit INTEGER DEFAULT NULL,"
                             "comission INTEGER DEFAULT NULL);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
    }


    void createTableSPOT() {
        std::string tableName = "spot";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "api_key TEXT," 
                             "secret_key TEXT,"   
                             "total INTEGER DEFAULT NULL,"
                             "profit INTEGER DEFAULT NULL,"
                             "comission INTEGER DEFAULT NULL);";
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
    

    //TODO: Если пользователя не вставило запись должна где-то сохраниться чтобы записать в будущем
    //TODO: Переход на транзакции
    void insertUser(int64_t user_id, const std::string& name, std::optional<int64_t> referrer_id = std::nullopt) {
        try{
            std::string queryStr = "INSERT INTO users (user_id, name, referrer_id) VALUES (?, ?, ?)";
            SQLite::Statement query(db, queryStr);

            query.bind(1, user_id);
            query.bind(2, name);
            if(referrer_id.has_value()) {
                query.bind(3, referrer_id.value());
            }
            else{
                query.bind(3);
            }
            
            query.exec();
        } catch(const SQLite::Exception& e) {
            if (std::string(e.what()).find("UNIQUE constraint failed") != std::string::npos) {
                std::cerr << "Ошибка: Пользователь с ID " << user_id << " уже существует." << std::endl;
            } else {
                std::cerr << "[SQLite Error] " << e.what() << std::endl;
            }
            
        }
    }


    void insertIntoReferrals(int64_t user_id) {
        try{
            std::string queryStr = "INSERT INTO referrers (user_id) VALUES (?)";
            SQLite::Statement query(db, queryStr);

            query.bind(1, user_id);
            query.exec();

        } catch(const SQLite::Exception& e) {
            if (std::string(e.what()).find("UNIQUE constraint failed") != std::string::npos) {
                std::cerr << "Ошибка: Пользователь с ID " << user_id << " уже существует." << std::endl;
            } else {
                std::cerr << "[SQLite Error] " << e.what() << std::endl;
            }
            
        }
    }


    void updateReferrersLevels(int64_t user_id, int level = 1) {
        try {
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
            
        } catch (const SQLite::Exception& e) {
            std::cerr << "Ошибка SQLite: " << e.what() << std::endl;
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


    int countReferrals(int64_t referrer_id){
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE referrer_id = ?;");
        query.bind(1, referrer_id);

        if (query.executeStep()) {
            return query.getColumn(0).getInt();
        }
    
        return 0; 
    }
    

    //TODO: Метод для бэкапов


    template <typename Func, typename... Args>
    void addToQueue(Func&& func, Args&&... args) {
        std::lock_guard<std::mutex> lock(queueMutex);

        operations.push_back([=]() {
            func(std::forward<Args>(args)...);
        });
    }


    void executeBatchInsert(SQLite::Database& db, size_t batchSize = 100) {
        std::stack<std::function<void()>> operations_stack;
        try{
            while(running) {
                std::lock_guard<std::mutex> lock(queueMutex);

                if (operations.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                db.exec("BEGIN TRANSACTION;");

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
    
                            std::cerr << "[executeBatchInsert][SQLite Error] " << errorMsg << std::endl;
                        }
                        
                    }
                    
                }

                db.exec("COMMIT;");

                while (!operations_stack.empty()) {
                    operations_stack.pop();
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[executeBatchInsert][Unexpected Error] " << e.what() << std::endl;
        }
    }
};

#endif