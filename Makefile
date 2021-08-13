CC = g++
CFLAGS = -c -std=c++11
OBJ = dns.o 
MAIN = dns.cpp

all: dns

dns: $(OBJ)
	$(CC) $(OBJ) -o dns -lpcap

dns.o: $(MAIN)
	$(CC) $(CFLAGS) $(MAIN)

clean:
	rm -rf *o dns