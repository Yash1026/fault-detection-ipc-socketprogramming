CC = gcc
CFLAGS = -Wall -pthread

all: machine supervisor alert_server client

machine: machine.c common.h
	$(CC) $(CFLAGS) machine.c -o machine

supervisor: supervisor.c common.h
	$(CC) $(CFLAGS) supervisor.c -o supervisor

alert_server: alert_server.c common.h
	$(CC) $(CFLAGS) alert_server.c -o alert_server

client: client.c common.h
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f machine supervisor alert_server client *.o faultsys.log
