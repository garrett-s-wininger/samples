all: libffi.so

clean:
	rm -rf *.o
	rm -rf *.so

libffi.so: ffi.o
	ld -shared ffi.o -o libffi.so

ffi.o: ffi.c
	gcc -c -fPIC ffi.c
