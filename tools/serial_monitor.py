#!/usr/bin/env python3
import serial
import sys

def listen():
    with serial.Serial(sys.argv[1], timeout=1) as s:
        print("Serial port opened.")
        try:
            while True:
                try:
                    if (data := s.readline()): print(data.decode(), end = "")
                except UnicodeError:
                    continue
        except KeyboardInterrupt:
            pass
    input("Stopped listening. Press any key to continue...")
    listen()

if __name__ == "__main__":
    listen()

