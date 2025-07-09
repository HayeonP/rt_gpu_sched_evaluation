#!/bin/bash

g++ -std=c++11 -o 1_periodic_task 1_periodic_task.cpp \
    -I../../cuMiddleWare/include \
    -L../../cuMiddleWare/lib \
    -lcarss