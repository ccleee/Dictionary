CC=gcc
CFLAG=-Wall -g -O2 -lsqlite3
ALL=server client 
OBJECT=server.o client.o
all: server client
server:server.o
	$(CC) $< -o $@ $(CFLAG)
client:client.o
	$(CC) $< -o $@ $(CFLAG)
%*.o:%*.c
	$(CC) -c $< -o $@ $(CFALG)
.PHONY:clean
clean:
	rm -rf $(ALL) $(OBJECT)
