.PHONY: all run refresh clean gdb

ARGS = 16 16 25

main.exe:
	g++ -std=c++11 -Ltermios -o main.exe main.cpp

run: main.exe
	./main.exe $(ARGS)

refresh: clean main.exe run

clean:
	rm -f *.o *.exe

gdb:
	g++ -std=c++11 -g -Ltermios -o main.o main.cpp
	gdb main.o
