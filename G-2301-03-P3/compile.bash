#!/bin/bash
echo "LIMPIANDO"
make -f G-2301-03-P3-makefile clean
rm -f lib/*
clear
clear
echo
echo "COMPILANDO..."
make -f G-2301-03-P3-makefile all
