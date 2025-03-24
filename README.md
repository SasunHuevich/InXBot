# InXBot


## Пример файла   include/config.hpp
```Shell
#include <string>

std::string TG_BOT_TOKEN ="6653096426:AAFS69aAx7gG8JPew5HtjSIPdqUSsQB6bi4";
std::string DB_NAME = "";
```

```Shell
sudo apt update
```

```Shell
sudo apt install cmake
```

```Shell
sudo apt install libcurl4-openssl-dev
```

```Shell
apt update && apt upgrade -y && apt install -y pipx clang cmake ccache libc++-18-dev libc++1-18 gdb && export PATH=/usr/lib/ccache:${PATH} && echo "export PATH=/usr/lib/ccache:\${PATH}" >> ~/.bashrc 
```

```Shell
sudo apt-get install zlib1g-dev
```

```Shell
sudo apt-get install libssl-dev
```

```Shell
sudo apt-get install libboost-all-dev
```
Установка tg-bot-cpp
```Shell
git clone https://github.com/reo7sp/tgbot-cpp.git
cd tgbot-cpp
cmake .
make -j4
sudo make install
```

Установка SQLiteCpp

```Shell
git clone https://github.com/SRombauts/SQLiteCpp.git
cd SQLiteCpp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
sudo make install
```
