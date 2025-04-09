#ifndef DATABASECONNECTOR_HPP
#define DATABASECONNECTOR_HPP

#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <filesystem>

class DatabaseConnector {


private:
    SQLite::Database db;


public:
    DatabaseConnector(const std::string& dbName) : db(dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE){
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
                             "level_1 INTEGER DEFAULT NULL,"
                             "level_2 INTEGER DEFAULT NULL," 
                             "level_3 INTEGER DEFAULT NULL,"
                             "level_4 INTEGER DEFAULT NULL,"
                             "level_5 INTEGER DEFAULT NULL);";
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
};

#endif