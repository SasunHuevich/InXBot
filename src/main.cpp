#include <stdio.h>
#include <tgbot/tgbot.h>
#include <config.hpp>
#include <database_connector.hpp>
#include <algorithm> // Для std::all_of
#include <cctype>    // Для std::isdigit
#include <string> // Для std::stoi
#include <filesystem>
#include <sheet_connector.hpp>


TgBot::ReplyKeyboardMarkup::Ptr botButtons(TgBot::Bot &bot){
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup());
    TgBot::KeyboardButton::Ptr referrals(new TgBot::KeyboardButton());
    referrals->text = "Refferals";

    keyboard->keyboard = {{referrals}};
    keyboard->resizeKeyboard = true;  // Уменьшаем клавиатуру, чтобы она не занимала весь экран
    keyboard->oneTimeKeyboard = true; // Убираем клавиатуру после выбора
    return keyboard;
}


int main() {
    //TODO: Не должно открываться две версии одновременно
    //TODO: Поток для бд сделать отдельно
    SheetConnector gg(SPREADSHEET_ID);
    gg.updateDbWithSheets();
    gg.updateSheetsWithDb();

    std::filesystem::create_directory("db");
    DatabaseConnector users_db("db/users.db");
    users_db.createTableUsers();
    users_db.createTableEASY();
    users_db.createTableELITE();
    users_db.createTableSPOT();
    users_db.createTableReferrers();

    TgBot::Bot bot(TG_BOT_TOKEN);

    
    bot.getEvents().onCommand("start", [&bot,  &users_db](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Привет!", nullptr, 0, botButtons(bot));
        int64_t user_id = message->from->id;
        bot.getApi().sendMessage(message->chat->id, "Ваша ссылка: https://t.me/" + bot.getApi().getMe()->username + "?start=" + std::to_string(user_id));

        if(!users_db.doesUserExist(user_id)) {
            if(message->text.find(" ") != std::string::npos) {
                std::string param = message->text.substr(message->text.find(" ") + 1);
                
                if(!param.empty() && std::all_of(param.begin(), param.end(), ::isdigit)){
                    int64_t referrer_id = std::stoll(param);
                    users_db.insertUser(user_id, message->from->username, referrer_id);
                    users_db.insertIntoReferrals(user_id);
                    users_db.updateReferrersLevels(referrer_id);

                    bot.getApi().sendMessage(message->chat->id, "Ваш реферал: " + users_db.getUsernameById(referrer_id));
                }
                else{
                    bot.getApi().sendMessage(message->chat->id, "Реферальная ссылка неверна!");
                }
            }
            else{
                users_db.insertUser(user_id, message->from->username);
                users_db.insertIntoReferrals(user_id);
            }
        }
        else{
            bot.getApi().sendMessage(message->chat->id, "Вы уже зарегистрированы!");
        }
        
    });


    bot.getEvents().onAnyMessage([&bot, &users_db](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        else if (message->text == "Refferals") {
            bot.getApi().sendMessage(message->chat->id, "Количество ваших рефераллов: " + std::to_string(users_db.countReferrals(message->from->id)));
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
    return 0;
}