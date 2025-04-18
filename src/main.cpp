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


TgBot::ReplyKeyboardMarkup::Ptr botButtons(TgBot::Bot &bot, const std::string& language, int64_t user_id){
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup());

    TgBot::KeyboardButton::Ptr btnBriefing(new TgBot::KeyboardButton());
    btnBriefing->text = getText("btn_briefing", language);

    TgBot::KeyboardButton::Ptr btnProfile(new TgBot::KeyboardButton());
    btnProfile->text = getText("btn_profile", language);

    TgBot::KeyboardButton::Ptr btnDescription(new TgBot::KeyboardButton());
    btnDescription->text = getText("btn_description_of_algoritms", language);

    TgBot::KeyboardButton::Ptr btnInvite(new TgBot::KeyboardButton());
    btnInvite->text = getText("btn_invite_friend", language);

    TgBot::KeyboardButton::Ptr btnInstructions(new TgBot::KeyboardButton());
    btnInstructions->text = getText("btn_instructions", language);

    // Располагаем кнопки в 2 ряда: 3 сверху, 2 снизу
    keyboard->keyboard = {
        {btnBriefing, btnProfile},
        {btnDescription, btnInvite},
        {btnInstructions}
    };

    keyboard->resizeKeyboard = true;  // Уменьшаем клавиатуру, чтобы она не занимала весь экран
    keyboard->oneTimeKeyboard = true; // Убираем клавиатуру после выбора

    return keyboard;
}


TgBot::InlineKeyboardMarkup::Ptr botLanguageMenu() {
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup());

    TgBot::InlineKeyboardButton::Ptr ruBtn(new TgBot::InlineKeyboardButton());
    ruBtn->text = "Русский";
    ruBtn->callbackData = "lang_ru";

    TgBot::InlineKeyboardButton::Ptr enBtn(new TgBot::InlineKeyboardButton());
    enBtn->text = "English";
    enBtn->callbackData = "lang_en";

    keyboard->inlineKeyboard = {{ruBtn, enBtn}};

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
    write_db.createTableUsersLanguage();

    TgBot::Bot bot(TG_BOT_TOKEN);

    
    bot.getEvents().onCommand("start", [&bot,  &write_db, &read_db](TgBot::Message::Ptr message) {
        int64_t user_id = message->from->id;
        std::string user_language = read_db.getUserLanguage(user_id);

        bot.getApi().sendMessage(message->chat->id, getText("start_message", user_language), nullptr, 0, botButtons(bot, user_language, user_id));
        bot.getApi().sendMessage(message->chat->id, getText("language_selection", user_language), nullptr, 0, botLanguageMenu()); 
        //bot.getApi().sendMessage(message->chat->id, "Ваша ссылка: https://t.me/" + bot.getApi().getMe()->username + "?start=" + std::to_string(user_id));

        std::string name = message->from->firstName + " " + message->from->lastName;
        std::string username = message->from->username;
        if(!read_db.doesUserExist(user_id)) {
            if(message->text.find(" ") != std::string::npos) {
                std::string param = message->text.substr(message->text.find(" ") + 1);
                
                if(!param.empty() && std::all_of(param.begin(), param.end(), ::isdigit)){
                    int64_t referrer_id = std::stoll(param);
                    write_db.newUser(user_id, name, username, referrer_id);
                }
                else{
                    write_db.newUser(user_id, name, username);
                }
            }
            else{
                write_db.newUser(user_id, name, username);
            }
        }
    });


    bot.getEvents().onCallbackQuery([&bot, &write_db](TgBot::CallbackQuery::Ptr query) {
        if (query->data == "lang_ru") {
            write_db.UserLanguageToQueue(query->from->id, "ru");
            bot.getApi().sendMessage(query->message->chat->id, "Вы выбрали Русский язык.", nullptr, 0, botButtons(bot, "ru", query->from->id));
        } else if (query->data == "lang_en") {
            write_db.UserLanguageToQueue(query->from->id, "en");
            bot.getApi().sendMessage(query->message->chat->id, "You selected English.", nullptr, 0, botButtons(bot, "en", query->from->id));
        }
    
        // Ответить на callback обязательно
        bot.getApi().answerCallbackQuery(query->id);
    });


    bot.getEvents().onAnyMessage([&bot, &write_db, &read_db](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());

        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }

        std::string key = getButtonKeyFromText(read_db.getUserLanguage(message->from->id), message->text);


        if (key == "btn_briefing") {
            bot.getApi().sendMessage(message->chat->id, "📋 Вот ваш инструктаж...");
        } 
        else if (key == "btn_profile") {
            bot.getApi().sendMessage(message->chat->id, "👤 Здесь ваш профиль...");
        } 
        else if (key == "btn_description_of_algoritms") {
            bot.getApi().sendMessage(message->chat->id, "🧠 Подробности об алгоритмах...");
        } 
        else if (key == "btn_instructions") {
            bot.getApi().sendMessage(message->chat->id, "🔌 Инструкция по подключению...");
        } 
        else if (key == "btn_invite_friend") {
            bot.getApi().sendMessage(message->chat->id, "🤝 Пригласите друга по этой ссылке...");
        } 
        else {
            bot.getApi().sendMessage(message->chat->id, "❓ Неизвестная команда: " + message->text);
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