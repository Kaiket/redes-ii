#!/bin/bash
echo "LIMPIANDO"
make -f G-2301-03-P2-makefile clean
clear
clear
echo
echo "COMPILANDO..."
make -f G-2301-03-P3-makefile all
