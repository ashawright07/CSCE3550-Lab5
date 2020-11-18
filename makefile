all:client server
client: UDP_Client.c
	gcc UDP_Client.c -o UDP_Client
server: UDP_Server.c
	gcc UDP_Server.c -o UDP_Server -lm
clean:
	rm -f server client