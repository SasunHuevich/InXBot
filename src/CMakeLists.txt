set(BOT inx_bot)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(${BOT}
    main.cpp
)


target_link_libraries(${BOT} PRIVATE 
    TgBot
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    SQLiteCpp
    SQLite::SQLite3
)
install(TARGETS ${BOT} RUNTIME DESTINATION bin)