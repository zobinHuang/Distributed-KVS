SERVER-OBJ = ./obj/server.o ./obj/kv_server.o ./obj/SockList.o ./obj/logging.o
CC = g++

server: $(SERVER-OBJ)
	$(CC) -o server $(SERVER-OBJ)

./obj/server.o: server.cpp ./kv_server/kv_server.h ./socket/SockList.h ./log/logging.h
	$(CC) -c server.cpp
	mv server.o ./obj/server.o

./obj/logging.o: ./log/logging.cpp
	$(CC) -c ./log/logging.cpp
	mv logging.o ./obj/logging.o

./obj/kv_server.o: ./kv_server/kv_server.cpp
	$(CC) -c ./kv_server/kv_server.cpp
	mv kv_server.o ./obj/kv_server.o

./obj/SockList.o: ./socket/SockList.cpp
	$(CC) -c ./socket/SockList.cpp
	mv SockList.o ./obj/SockList.o

#clean
.PHONY : clean
clean:
	rm server ./obj/*.o