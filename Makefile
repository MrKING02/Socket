ClientMake: main.c client.c Serialize.c UI.c
	gcc -o Client_Chat main.c client.c Serialize.c UI.c -lpthread -lncurses