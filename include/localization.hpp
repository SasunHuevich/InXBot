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

    {"language_selected", {
        {"en", "🇬🇧 You have selected English."},
        {"ru", "🇷🇺 Вы выбрали Русский язык."}
    }},

    {"admin_message", {
        {"en", "👑 Greetings, Supreme One. How may I serve you today?"},
        {"ru", "👑 Здравствуйте, Наивысший. Чем могу быть полезен?"}
    }},

    {"database_update_message", {
        {"en", "⏳ The database will be updated within 15 minutes."},
        {"ru", "⏳ База данных будет обновлена в течение 15 минут."}
    }},

    {"btn_admin_panel", {
        {"en", "👑 Admin Panel"},
        {"ru", "👑 Панель администратора"}
    }},

    {"btn_update_database", {
        {"en", "📄 Update Database"},
        {"ru", "📄 Обновить базу данных"}
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
    }},

    {"btn_balance", {
        {"en", "💰 Balance"},
        {"ru", "💰 Баланс"}
    }},

    {"btn_change_language", {
        {"en", "🌐 Change Language"},
        {"ru", "🌐 Изменить язык"}
    }},

    {"btn_referrals", {
        {"en", "🤝 My Referrals"},
        {"ru", "🤝 Мои реферралы"}
    }},

    {"btn_change_data", {
        {"en", "✏️ Edit Profile"},
        {"ru", "✏️ Изменить данные"}
    }},

    {"btn_back", {
        {"en", "🔙 Back"},
        {"ru", "🔙 Назад"}
    }},

    {"description_of_algoritms", {
          {"en", "🤖 We offer 3 trading algorithms:\n\n"
                 "1. 🌱 Easy\n"
                 "- Profitability: 7–15%/month\n"
                 "- Deposit: from 500 USDT\n- Type: Futures\n"
                 "- WinRate: 99%\n\n2. ⭐️ Elite\n"
                 "- Profitability: 15–20%/month\n"
                 "- Deposit: from 1000 USDT\n"
                 "- Type: Futures\n- WinRate: 89%\n\n"
                 "3. 💹 Spot\n"
                 "- Profitability: 4–9%/month\n"
                 "- Deposit: from 5000 USDT\n"
                 "- Type: Spot trading\n"
                 "- WinRate: 93%\n\n"
                 "💰 Commission: 30% of profit\n"
                 "🔐 Security: Full control over funds"},

          {"ru", "🤖 Мы предлагаем 3 алгоритма торговли:\n\n"
                 "1. 🌱 Лёгкий\n"
                 "- Доходность: 7–15% в месяц\n"
                 "- Депозит: от 500 USDT\n"
                 "- Тип: фьючерсы\n- WinRate: 99%\n\n"
                 "2. ⭐️ Elite\n- Доходность: 15–20% в месяц\n"
                 "- Депозит: от 1000 USDT\n- Тип: фьючерсы\n"
                 "- WinRate: 89%\n\n"
                 "3. 💹 Spot\n"
                 "- Доходность: 4–9% в месяц\n"
                 "- Депозит: от 5000 USDT\n"
                 "- Тип: спотовая торговля\n"
                 "- WinRate: 93%\n\n"
                 "💰 Комиссия: 30% от прибыли\n"
                 "🔐 Безопасность — полный контроль над средствами"}
        }
    },

    {"invite_friend", {
            {"en", "💸 Want to earn more?\n\n"
                   "🤝 Invite a friend and get rhythms with up to 15% monthly profit!\n\n"
                   "Here’s your personal link:\n\n"},
            
            {"ru", "💸 Хочешь зарабатывать больше?\n\n"
                   "🤝 Пригласи друга и получай ритмы с доходностью до 15% в месяц!\n\n"
                   "Вот твоя персональная ссылка:\n\n"},
        }
    },

    {"instructions", {
        {"en", "⚙️ To connect to the algorithm, follow these 2 simple steps:\n\n"
               "1️⃣ Register on Bybit and complete KYC.\n"
               "2️⃣ Connect your API keys or use one-click copy trading.\n\n"
               "🔑 Don’t forget to add your keys in the profile to start trading.\n\n"
               "📄 Here’s the PDF guide:\n"},

        {"ru", "⚙️ Для подключения к алгоритму следуй двум простым шагам:\n\n"
               "1️⃣ Зарегистрируйся на бирже Bybit и пройди KYC.\n"
               "2️⃣ Подключи API-ключи или используй копирование в 1 клик.\n\n"
               "🔑 Не забудь добавить ключи в профиле для начала торговли.\n\n"
               "📄 Вот PDF-инструкции:\n"}
        }
    },
    
    {"briefing", {
        {"en", "📋 Main Menu:\n\n"
               "1️⃣ Check Balance — view your profitability, connected algorithms, and current deposit.\n\n"
               "2️⃣ Algorithm Details — full information about each trading strategy.\n\n"
               "3️⃣ InX Project Guide — step-by-step instructions for exchange registration and API key connection.\n\n"
               "4️⃣ Invite a Friend — earn 30% of your friend's profit."},

        {"ru", "📋 Главное меню:\n\n"
               "1️⃣ Посмотреть баланс — узнать доходность, подключённые алгоритмы и текущий депозит.\n\n"
               "2️⃣ Описание алгоритмов — подробности по каждому торговому решению.\n\n"
               "3️⃣ Гид по проекту InX — пошаговая инструкция регистрации на бирже и подключения API-ключей.\n\n"
               "4️⃣ Пригласить друга — получи 30% от прибыли друга."}
        }
    },

    {"profile", {
        {"en", "👤 This is your profile! What would you like to do?"},
        {"ru", "👤 Это ваш профиль! Что хотите сделать?"}
        }
    },

    {"back_message", {
        {"en", "🏠 You’re back to the main menu"},
        {"ru", "🏠 Вы вернулись в главное меню"}
        }
    }

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