ServerMake: Main.c server.c Queue.c Serialize.c
	gcc -o Server_Chat Main.c server.c Queue.c Serialize.c -lpthread -lncurses
