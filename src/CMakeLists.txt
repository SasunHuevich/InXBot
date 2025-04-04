set(BOT inx_bot)

set(jwt-cpp_DIR "${CMAKE_SOURCE_DIR}/build")

find_package(OpenSSL REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(jwt-cpp REQUIRED)


add_executable(${BOT}
    main.cpp
)


target_link_libraries(${BOT} PRIVATE 
    TgBot
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLiteCpp
    SQLite::SQLite3
    CURL:libcurl
    jwt-cpp
)


install(TARGETS ${BOT} RUNTIME DESTINATION bin)