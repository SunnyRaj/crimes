#!/bin/bash

for f in $(ls *.c)
do
    $(gcc "$f" `pkg-config --cflags --libs glib-2.0 libvmi` -g -lm -o "${f%.*}")
done
