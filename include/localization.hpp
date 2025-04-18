#pragma once

#include <unordered_map>
#include <string>
#include <map>
#include <unordered_map>


static std::unordered_map<std::string, std::map<std::string, std::string>> translations {
    {"start_message", {
        {"en", "👋 Hi! Welcome to InX — the platform for automated cryptocurrency trading.\n\n"
            "🤖 Our algorithms generate up to 📈 15% monthly returns without transferring funds for management.\n\n"
            "🚀 Ready to start? Choose an action from the menu below or check out the 📘 quick guide."},

        {"ru", "👋 Привет! Добро пожаловать в InX — платформу автоматической торговли криптовалютой.\n\n"
                "🤖 Наши алгоритмы дают до 📈 15% доходности в месяц без передачи средств в управление.\n\n"
                "🚀Готов начать? Выбирай действие из меню ниже или изучи 📘 краткий инструктаж."}
    }},

    {"language_selection", {
        {"en", "🌐 Please choose a language:"},
        {"ru", "🌐 Пожалуйста, выберите язык:"}
    }},

    {"btn_briefing", {
        {"en", "📋 Briefing"},
        {"ru", "📋 Инструктаж"}
    }},
    
    {"btn_profile", {
        {"en", "👤 Profile"},
        {"ru", "👤 Профиль"}
    }},
    
    {"btn_description_of_algoritms", {
        {"en", "🧠 Algorithm Overview"},
        {"ru", "🧠 Описание алгоритмов"}
    }},
    
    {"btn_instructions", {
        {"en", "🔌 Connection Guide"},
        {"ru", "🔌 Инструкции по подключению"}
    }},
    
    {"btn_invite_friend", {
        {"en", "🤝 Invite a Friend"},
        {"ru", "🤝 Пригласить друга"}
    }}
};


std::string& getText(const std::string& key, const std::string lang) {
    return translations[key][lang];
}


std::string getButtonKeyFromText(const std::string& lang, const std::string& text) {
    for (const auto& item : translations) {
        if (item.second.count(lang) && item.second.at(lang) == text) {
            return item.first;
        }
    }

    return "";
}