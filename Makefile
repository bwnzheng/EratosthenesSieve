main: main.o
	mpicxx -o main main.o
main.o: main.cpp
	mpicxx -c main.cpp

clean:
	rm -f *.o
	rm -f main

.PHONY: clean