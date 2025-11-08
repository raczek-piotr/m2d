clear
g++ ./m2d/main.cpp -o m2d-client -lsfml-graphics -lsfml-window -lsfml-system
g++ ./server/server.cpp -o m2d-server -pthread -lsqlite3


