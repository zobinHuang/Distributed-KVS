TEST-OBJ = test.o kv.o
CC = g++

test: $(TEST-OBJ)
	$(CC) -o test $(TEST-OBJ)

test.o: test.cpp kv.h
	$(CC) -c test.cpp

kv.o: kv.cpp
	$(CC) -c kv.cpp

#clean
.PHONY : clean
clean:
	rm test *.o