all: server client
server:
	gcc -o server server.c
client:
	gcc -o client client.c
clean:
	rm  server client