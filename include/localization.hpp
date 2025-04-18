#pragma once

#include <unordered_map>
#include <string>
#include <map>
#include <unordered_map>


static std::unordered_map<std::string, std::map<std::string, std::string>> translations {
    {"start_message", {
        {"en", "Welcome to the bot! Choose an option:"},
        {"ru", "👋 Привет! Добро пожаловать в InX — платформу автоматической торговли криптовалютой.\n\n"
                "🤖 Наши алгоритмы дают до 📈 15% доходности в месяц без передачи средств в управление.\n\n"
                "🚀Готов начать? Выбирай действие из меню ниже или изучи 📘 краткий инструктаж."}
    }},
    {"btn_settings", {
        {"en", "Settings"},
        {"ru", "Настройки"}
    }},
    {"btn_language", {
        {"en", "Language"},
        {"ru", "Язык"}
    }}
};


std::string getText(const std::string& key, const std::string lang) {
    

    return translations[key][lang];
}
