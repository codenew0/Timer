#!/bin/sh
gcc `pkg-config --cflags gtk+-3.0` -o main main.c sound.c `pkg-config --libs gtk+-3.0` -pthread -lasound -Wall
