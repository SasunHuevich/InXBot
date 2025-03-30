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

    //TODO: Сделать проверку на существование таблицы
    void createTableUsers() {
        std::string tableName = "users";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL UNIQUE,"
                             "name TEXT,"  
                             "referrer_id INTEGER DEFAULT NULL,"
                             "date_registration TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана или существует." << std::endl;
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


    std::string getUsernameById(int user_id){
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


    bool doesUserExist(int user_id){
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE user_id = ?;");
        query.bind(1, user_id);

        if (query.executeStep()) {
            int count = query.getColumn(0);
            return count > 0;
        }

        return false;
    }


    int countReferrals(int referrer_id){
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