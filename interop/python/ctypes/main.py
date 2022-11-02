#! /usr/bin/env python3

import ctypes

if __name__ == "__main__":
    so = ctypes.cdll.LoadLibrary("./libffi.so")
    so.print_hello()
