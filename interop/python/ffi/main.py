#!/usr/bin/env  python3
import cffi

if __name__ == "__main__":
    ffi = cffi.FFI()
    ffi.cdef("int printf(const char* format, ...);")

    C = ffi.dlopen(None)
    arg = ffi.new("char[]", b"Hello from C!")
    C.printf(b"%s\n", arg)

