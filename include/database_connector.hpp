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
                             "name TEXT NOT NULL,"  
                             "referrer_id INTEGER DEFAULT NULL),"
                             "date_registration TEXT DEFAULT (CURRENT_TIMESTAMP);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана." << std::endl;
    }
    


    void insertUser(int user_id, const std::string& name, std::optional<int> referrer_id = std::nullopt) {
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

    

    //TODO: Метод для бэкапов
};

#endif