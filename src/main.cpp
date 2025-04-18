#include <stdio.h>
#include <tgbot/tgbot.h>
#include <config.hpp>
#include <database_connector.hpp>
#include <algorithm> // Для std::all_of
#include <cctype>    // Для std::isdigit
#include <string> // Для std::stoi
#include <filesystem>
#include <sheet_connector.hpp>
#include <localization.hpp>


TgBot::ReplyKeyboardMarkup::Ptr botButtons(TgBot::Bot &bot){
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup());
    TgBot::KeyboardButton::Ptr referrals(new TgBot::KeyboardButton());
    referrals->text = "Refferals";

    keyboard->keyboard = {{referrals}};
    keyboard->resizeKeyboard = true;  // Уменьшаем клавиатуру, чтобы она не занимала весь экран
    //keyboard->oneTimeKeyboard = true; // Убираем клавиатуру после выбора
    return keyboard;
}


int main() {
    //TODO: Не должно открываться две версии одновременно

    SheetConnector sheet(SPREADSHEET_ID);
    
    std::cout << sheet.getRowCount().second << std::endl;

    std::filesystem::create_directory("db");
    DatabaseConnector write_db("db/users.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_FULLMUTEX);
    DatabaseConnector read_db("db/users.db", SQLite::OPEN_READONLY | SQLite::OPEN_FULLMUTEX);
    sheet.updateDbWithSheets(write_db);

    std::thread dbThread(&DatabaseConnector::executeBatchInsert, std::ref(write_db));
    std::thread sheetsThread(&SheetConnector::updateSheetsWithDb, &sheet, std::ref(write_db));

    write_db.createTableUsers();
    write_db.createTableEASY();
    write_db.createTableELITE();
    write_db.createTableSPOT();
    write_db.createTableReferrers();

    TgBot::Bot bot(TG_BOT_TOKEN);

    
    bot.getEvents().onCommand("start", [&bot,  &write_db, &read_db](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, getText("start_message", "ru"), nullptr, 0, botButtons(bot));
        int64_t user_id = message->from->id;
        bot.getApi().sendMessage(message->chat->id, "Ваша ссылка: https://t.me/" + bot.getApi().getMe()->username + "?start=" + std::to_string(user_id));
        std::string name = message->from->firstName + " " + message->from->lastName;
        std::string username = message->from->username;
        if(!read_db.doesUserExist(user_id)) {
            if(message->text.find(" ") != std::string::npos) {
                std::string param = message->text.substr(message->text.find(" ") + 1);
                
                if(!param.empty() && std::all_of(param.begin(), param.end(), ::isdigit)){
                    int64_t referrer_id = std::stoll(param);

                    if(read_db.doesUserExist(referrer_id)) {
                        write_db.newUser(user_id, name, username, referrer_id);
                        bot.getApi().sendMessage(message->chat->id, "Ваш реферал: " + read_db.getUsernameById(referrer_id));
                    }
                    else{
                        write_db.newUser(user_id, name, username);
                    }
                    
                }
                else{
                    bot.getApi().sendMessage(message->chat->id, "Реферальная ссылка неверна!");
                }
            }
            else{
                write_db.newUser(user_id, name, username);
            }
        }
        else{
            bot.getApi().sendMessage(message->chat->id, "Вы уже зарегистрированы!");
        }
        
    });


    bot.getEvents().onAnyMessage([&bot, &write_db, &read_db](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        else if (message->text == "Refferals") {
            //bot.getApi().sendMessage(message->chat->id, "Количество ваших рефераллов: " + std::to_string(read_db.countReferrals(message->from->id)));
        }
        else{
            bot.getApi().sendMessage(message->chat->id, "Ваше сообщение: " + message->text);
        }
        
    });


    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }

    dbThread.join();
    sheetsThread.join();
    return 0;
}