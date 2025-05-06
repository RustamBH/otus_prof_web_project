# Otus_prof_web_project

Реализация программы асинхронного https сервера с пулом потоков.

* Многопоточность: Сервер использует пул потоков для обработки соединений.
* Асинхронность: Используется асинхронный ввод-вывод с помощью Boost.Asio.
* HTTPS поддержка: Реализовано с помощью OpenSSL через Boost.Beast.
* Статические файлы: Сервер может обслуживать статические файлы из указанной директории.
* Пул соединений: Каждое соединение обрабатывается в отдельном потоке из пула.
* Выбранная IDE: VSCode
* Выбранный компилятор : CMake (C/С++ IntelliSense)


# Сборка и запуск

1. Установите зависимости:
sudo apt-get install libboost-all-dev libssl-dev

2. Создайте сертификаты для сервера:
openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes

3. Соберите сервер:
g++ -std=c++17 -o server server.cpp -lboost_system -lboost_thread -lssl -lcrypto -pthread

4. Соберите клиент:
g++ -std=c++17 -o client client.cpp -lboost_system -lssl -lcrypto

5. Запустите сервер:
./https_server 127.0.0.1 4433 ./www 3

6. Запустите клиент:
./https_client localhost 4433 /index.html
