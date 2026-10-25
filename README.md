# InXBot

Telegram-бот на C++ для клиентов InX — сервиса автоматической торговли криптовалютой.
Бот ведёт базу пользователей с реферальной системой и синхронизирует её с Google Таблицей,
в которой администраторы ведут данные по клиентам (балансы, подключённые стратегии, прибыль, комиссии).

## Возможности

- **Главное меню**: брифинг, описание торговых алгоритмов, инструкция, приглашение друга.
- **Профиль**: баланс, рефералы, смена языка, передача номера телефона (через кнопку «Поделиться контактом»).
- **Реферальная система**: персональная ссылка `https://t.me/<bot>?start=<user_id>`, учёт рефереров по уровням.
- **Локализация**: русский и английский, тексты лежат в [include/localization.hpp](include/localization.hpp).
- **Админ-панель**: доступна пользователям из списка `adminIds`, позволяет вручную запустить синхронизацию с таблицей.
- **Синхронизация с Google Sheets** (лист `clients`):
  - новые пользователи бота дописываются в таблицу;
  - данные из таблицы (баланс, комиссия, статусы стратегий EASY / ELITE / SPOT) периодически подтягиваются в локальную БД.

## Как устроено

| Файл | Что делает |
|---|---|
| [src/main.cpp](src/main.cpp) | Точка входа: клавиатуры, обработчики команд и сообщений, long polling |
| [include/database_connector.hpp](include/database_connector.hpp) | SQLite (`db/users.db`): таблицы пользователей, стратегий, рефералов, языков; очередь записи в отдельном потоке |
| [include/sheet_connector.hpp](include/sheet_connector.hpp) | Google Sheets API: авторизация сервисного аккаунта через JWT, чтение и запись листа `clients` |
| [include/localization.hpp](include/localization.hpp) | Тексты сообщений и кнопок на RU/EN |

Стек: C++20, [tgbot-cpp](https://github.com/reo7sp/tgbot-cpp), SQLiteCpp, libcurl, jwt-cpp, RapidJSON, nlohmann/json, OpenSSL.
Зависимости (кроме tgbot-cpp) ставятся через [Conan](https://conan.io/).

## Настройка

### 1. Telegram-бот

Создайте бота у [@BotFather](https://t.me/BotFather) и получите токен.

### 2. Google Таблица

1. В Google Cloud Console создайте проект, включите **Google Sheets API** и создайте **сервисный аккаунт**.
2. Скачайте JSON-ключ сервисного аккаунта и положите его в корень проекта как `service_account.json`.
3. Создайте таблицу с листом `clients` и выдайте email сервисного аккаунта права редактора.
4. ID таблицы — это часть URL: `https://docs.google.com/spreadsheets/d/<SPREADSHEET_ID>/edit`.

### 3. Файл `include/config.hpp`

Файл в `.gitignore`, создайте его вручную:

```cpp
#pragma once

#include <string>
#include <vector>
#include <cstdint>

std::string TG_BOT_TOKEN = "<токен от BotFather>";
std::string SERVICE_ACCOUNT_FILE = "service_account.json";
std::string SPREADSHEET_ID = "<ID Google Таблицы>";

// Telegram user_id администраторов
std::vector<int64_t> adminIds = {123456789};
```

> Никогда не коммитьте `config.hpp` и `service_account.json` — оба уже добавлены в `.gitignore`.

## Сборка (Ubuntu/Debian)

Системные зависимости:

```shell
sudo apt update
sudo apt install -y build-essential cmake pipx \
    libboost-all-dev libssl-dev zlib1g-dev libcurl4-openssl-dev
pipx install conan
conan profile detect
```

tgbot-cpp подключается как подпроект из `libs/`:

```shell
git clone https://github.com/reo7sp/tgbot-cpp.git libs/tgbot-cpp
```

Остальные зависимости через Conan и сборка:

```shell
conan install . --output-folder=build --build=missing
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Запуск

Запускайте из корня проекта, чтобы бот нашёл `service_account.json` и создал папку `db/`:

```shell
./build/src/inx_bot
```

База данных создаётся автоматически в `db/users.db`.
