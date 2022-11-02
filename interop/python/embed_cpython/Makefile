all: main

main: main.o
	gcc main.o $$(python3.9-config --ldflags --embed) -pie -o main

main.o:
	gcc -c $$(python3.9-config --cflags) -fPIE main.c

clean:
	rm -f main
	rm -f *.o

