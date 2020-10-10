CLIENT-OBJ = ./obj/client.o ./obj/CConHash.o ./obj/CHashFun.o ./obj/kv_client.o ./obj/CMd5.o ./obj/CMD5HashFun.o ./obj/CNode_s.o ./obj/CRBTree.o ./obj/CVirtualNode_s.o ./obj/SockList.o ./obj/logging.o
CC = g++

client: $(CLIENT-OBJ)
	$(CC) -o client $(CLIENT-OBJ)

./obj/client.o: ./client.cpp ./chash/CNode_s.h ./kv_client/kv_client.h ./chash/CVirtualNode_s.h ./chash/CHashFun.h ./chash/CMD5HashFun.h ./chash/CConHash.h ./socket/SockList.h ./log/logging.h
	$(CC) -c client.cpp
	mv client.o ./obj/client.o

./obj/logging.o: ./log/logging.cpp
	$(CC) -c ./log/logging.cpp
	mv logging.o ./obj/logging.o

./obj/SockList.o: ./socket/SockList.c
	$(CC) -c ./socket/SockList.c
	mv SockList.o ./obj/SockList.o

./obj/kv_client.o: ./kv_client/kv_client.cpp ./log/logging.h
	$(CC) -c ./kv_client/kv_client.cpp
	mv kv_client.o ./obj/kv_client.o

./obj/CConHash.o: ./chash/CConHash.cpp ./chash/CVirtualNode_s.h
	$(CC) -c ./chash/CConHash.cpp
	mv CConHash.o ./obj/CConHash.o

./obj/CHashFun.o: ./chash/CHashFun.cpp
	$(CC) -c ./chash/CHashFun.cpp
	mv CHashFun.o ./obj/CHashFun.o

./obj/CMd5.o: ./chash/CMd5.cpp
	$(CC) -c ./chash/CMd5.cpp
	mv CMd5.o ./obj/CMd5.o

./obj/CMD5HashFun.o: ./chash/CMD5HashFun.cpp ./chash/CMd5.h
	$(CC) -c ./chash/CMD5HashFun.cpp
	mv CMD5HashFun.o ./obj/CMD5HashFun.o

./obj/CNode_s.o: ./chash/CNode_s.cpp
	$(CC) -c ./chash/CNode_s.cpp
	mv CNode_s.o ./obj/CNode_s.o

./obj/CRBTree.o: ./chash/CRBTree.cpp
	$(CC) -c ./chash/CRBTree.cpp
	mv CRBTree.o ./obj/CRBTree.o

./obj/CVirtualNode_s.o: ./chash/CVirtualNode_s.cpp
	$(CC) -c ./chash/CVirtualNode_s.cpp
	mv CVirtualNode_s.o ./obj/CVirtualNode_s.o

#clean
.PHONY : clean
clean:
	rm ./client ./obj/*.o