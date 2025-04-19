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


bool isAdmin(int64_t userId) {
    return std::find(adminIds.begin(), adminIds.end(), userId) != adminIds.end();
}


TgBot::ReplyKeyboardMarkup::Ptr botButtons(TgBot::Bot &bot, const std::string& language, bool isAdmin){
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup());

    if (isAdmin) {
        TgBot::KeyboardButton::Ptr btnAdminPanel(new TgBot::KeyboardButton());
        btnAdminPanel->text = getText("btn_admin_panel", language);

        keyboard->keyboard.push_back({btnAdminPanel});
    }

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

    keyboard->keyboard.push_back({btnBriefing, btnProfile});
    keyboard->keyboard.push_back({btnDescription, btnInvite});
    keyboard->keyboard.push_back({btnInstructions});

    keyboard->resizeKeyboard = true;  // Уменьшаем клавиатуру, чтобы она не занимала весь экран
    keyboard->oneTimeKeyboard = true; // Убираем клавиатуру после выбора

    return keyboard;
}


TgBot::ReplyKeyboardMarkup::Ptr botProfileButtons(TgBot::Bot &bot, const std::string& language){
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup());

    TgBot::KeyboardButton::Ptr btnBalance(new TgBot::KeyboardButton());
    btnBalance->text = getText("btn_balance", language);

    TgBot::KeyboardButton::Ptr btnChangeLanguage(new TgBot::KeyboardButton());
    btnChangeLanguage->text = getText("btn_change_language", language);

    TgBot::KeyboardButton::Ptr btnReferrals(new TgBot::KeyboardButton());
    btnReferrals->text = getText("btn_referrals", language);

    TgBot::KeyboardButton::Ptr btnChangeData(new TgBot::KeyboardButton());
    btnChangeData->text = getText("btn_change_data", language);

    TgBot::KeyboardButton::Ptr btnBack(new TgBot::KeyboardButton());
    btnBack->text = getText("btn_back", language);

    keyboard->keyboard = {
        {btnBalance, btnChangeLanguage},
        {btnReferrals, btnChangeData},
        {btnBack}
    };

    keyboard->resizeKeyboard = true;  // Уменьшаем клавиатуру, чтобы она не занимала весь экран
    keyboard->oneTimeKeyboard = true; // Убираем клавиатуру после выбора

    return keyboard;
}


TgBot::InlineKeyboardMarkup::Ptr botLanguageMenu() {
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup());

    TgBot::InlineKeyboardButton::Ptr ruBtn(new TgBot::InlineKeyboardButton());
    ruBtn->text = "🇷🇺 Русский";
    ruBtn->callbackData = "lang_ru";

    TgBot::InlineKeyboardButton::Ptr enBtn(new TgBot::InlineKeyboardButton());
    enBtn->text = "🇬🇧 English";
    enBtn->callbackData = "lang_en";

    keyboard->inlineKeyboard = {{ruBtn, enBtn}};

    return keyboard;
}


TgBot::ReplyKeyboardMarkup::Ptr botAdminPanel(TgBot::Bot &bot, const std::string& language){
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup());

    TgBot::KeyboardButton::Ptr btnUpdateGoogleTable(new TgBot::KeyboardButton());
    btnUpdateGoogleTable->text = getText("btn_update_database", language);

    TgBot::KeyboardButton::Ptr btnBack(new TgBot::KeyboardButton());
    btnBack->text = getText("btn_back", language);

    keyboard->keyboard.push_back({btnUpdateGoogleTable});
    keyboard->keyboard.push_back({btnBack});

    keyboard->resizeKeyboard = true;  // Уменьшаем клавиатуру, чтобы она не занимала весь экран
    keyboard->oneTimeKeyboard = true; // Убираем клавиатуру после выбора

    return keyboard;
}


TgBot::ReplyKeyboardMarkup::Ptr botChangeDataPanel(TgBot::Bot &bot, const std::string& language){
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup());

    TgBot::KeyboardButton::Ptr btnContact(new TgBot::KeyboardButton());
    btnContact->text = getText("btn_share_contact", language);
    btnContact->requestContact = true;

    TgBot::KeyboardButton::Ptr btnBack(new TgBot::KeyboardButton());
    btnBack->text = getText("btn_back", language);

    keyboard->keyboard = {
        {btnContact},
        {btnBack}
    };

    keyboard->resizeKeyboard = true;  // Уменьшаем клавиатуру, чтобы она не занимала весь экран
    keyboard->oneTimeKeyboard = true; // Убираем клавиатуру после выбора

    return keyboard;
}



int main() {
    //TODO: Не должно открываться две версии одновременно

    SheetConnector sheet(SPREADSHEET_ID);

    std::filesystem::create_directory("db");
    DatabaseConnector write_db("db/users.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_FULLMUTEX);
    DatabaseConnector read_db("db/users.db", SQLite::OPEN_READONLY | SQLite::OPEN_FULLMUTEX);

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

        bot.getApi().sendMessage(message->chat->id, getText("start_message", user_language), nullptr, 0, botButtons(bot, user_language, isAdmin(user_id)));
        bot.getApi().sendMessage(message->chat->id, getText("language_selection", user_language), nullptr, 0, botLanguageMenu()); 

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
            bot.getApi().sendMessage(query->message->chat->id, getText("language_selected", "ru"), nullptr, 0, botButtons(bot, "ru", isAdmin(query->from->id)));
        } else if (query->data == "lang_en") {
            write_db.UserLanguageToQueue(query->from->id, "en");
            bot.getApi().sendMessage(query->message->chat->id, getText("language_selected", "en"), nullptr, 0, botButtons(bot, "en", isAdmin(query->from->id)));
        }
    
        // Ответить на callback обязательно
        bot.getApi().answerCallbackQuery(query->id);
    });


    bot.getEvents().onAnyMessage([&bot, &write_db, &read_db, &sheet](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());

        std::string user_language = read_db.getUserLanguage(message->from->id);
        std::string key = getButtonKeyFromText(user_language, message->text);

        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        else if(message->contact != nullptr) {
            write_db.setPhone(message->contact->userId, message->contact->phoneNumber);
            bot.getApi().sendMessage(message->chat->id, getText("contact_saved_message", user_language));
        }
        else if (key == "btn_briefing") {
            bot.getApi().sendMessage(message->chat->id, getText("briefing", user_language));
        } 
        else if (key == "btn_profile") {
            bot.getApi().sendMessage(message->chat->id, getText("profile", user_language), nullptr, 0, botProfileButtons(bot, user_language));
        } 
        else if (key == "btn_description_of_algoritms") {
            bot.getApi().sendMessage(message->chat->id, getText("description_of_algoritms", user_language));
        } 
        else if (key == "btn_instructions") {
            bot.getApi().sendMessage(message->chat->id, getText("instructions", user_language));
        } 
        else if (key == "btn_invite_friend") {
            bot.getApi().sendMessage(message->chat->id, getText("invite_friend", user_language) + 
            "https://t.me/" + bot.getApi().getMe()->username + "?start=" + std::to_string(message->from->id));
        } 
        else if(key == "btn_change_language") {
            bot.getApi().sendMessage(message->chat->id, getText("language_selection", user_language), nullptr, 0, botLanguageMenu()); 
        }
        else if(key == "btn_back") {
            bot.getApi().sendMessage(message->chat->id, getText("back_message", user_language), nullptr, 0, botButtons(bot, user_language, isAdmin(message->from->id)));
        }
        else if(key == "btn_change_data") {
            bot.getApi().sendMessage(message->chat->id, getText("edit_profile_message", user_language), nullptr, 0, botChangeDataPanel(bot, user_language));
        }
        else if(isAdmin(message->from->id)) {
            if(key == "btn_admin_panel") {
                bot.getApi().sendMessage(message->chat->id, getText("admin_message", user_language), nullptr, 0, botAdminPanel(bot, user_language)); 
            }
            else if(key == "btn_update_database") {
                sheet.triggerUpdate();
                bot.getApi().sendMessage(message->chat->id, getText("database_update_message", user_language)); 
            }
            
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