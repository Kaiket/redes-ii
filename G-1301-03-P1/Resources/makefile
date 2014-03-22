all:: chat prsound

chat.o : chat.c
	@echo "\033[32;01mCompilando chat.c\033[00m"
	gcc -Wall chat.c -c `pkg-config --cflags gtk+-2.0`

chat_funcs.o : chat_funcs.c
	@echo "\033[32;01mCompilando chat_funcs.c\033[00m"
	gcc -Wall chat_funcs.c -c `pkg-config --cflags gtk+-2.0`

chat : chat.o chat_funcs.o
	@echo "\033[32;01mEnlazando\033[00m"
	gcc -o chat chat.o chat_funcs.o `pkg-config --libs gtk+-2.0`

sound.o : sound.c
	@echo "\033[32;01mCompilando sound.c\033[00m"
	gcc -Wall sound.c -c

prsound : prsound.c sound.o
	@echo "\033[32;01mEnlazando\033[00m"
	gcc -o prsound prsound.c sound.o `pkg-config --libs libpulse-simple`
