#ifndef DATABASECONNECTOR_HPP
#define DATABASECONNECTOR_HPP

#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <filesystem>

class DatabaseConnector {


private:
    SQLite::Database db;


public:
    DatabaseConnector(const std::string& dbName){
        std::filesystem::create_directory("db");
        db = SQLite::Database("db/" + dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        std::cout << "База данных '" << dbName << "' открыта" << std::endl;
    }


    void createTableUsers() {
        std::string tableName = "users";
        std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "user_id INTEGER NOT NULL,"
                             "referrer_id INTEGER DEFAULT NULL);";
        db.exec(query);
        std::cout << "Таблица '" << tableName << "' создана." << std::endl;
    }
    
};

#endif