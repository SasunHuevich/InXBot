set(BOT inx_bot)


find_package(OpenSSL REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(TgBot REQUIRED)
find_package(nlohmann_json REQUIRED)


add_executable(${BOT}
    main.cpp
)


target_link_libraries(${BOT} PRIVATE 
    TgBot
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLiteCpp
    SQLite::SQLite3
    CURL::libcurl
    jwt-cpp::jwt-cpp
    nlohmann_json::nlohmann_json
)


install(TARGETS ${BOT} RUNTIME DESTINATION bin)