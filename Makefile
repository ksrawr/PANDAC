pandc: pandc.c
	gcc -I -Wall pandc.c -o pandc -lpthread -lm

clean:
	rm pandc
