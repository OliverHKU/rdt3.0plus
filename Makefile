all: tclient tserver

tclient: test-client.c rdt-part3.h
	g++ test-client.c -o tclient -Wall

tserver: test-server.c rdt-part3.h
	g++ test-server.c -o tserver -Wall

clean:
	rm -f tclient tserver
