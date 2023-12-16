CC = gcc
CFLAGS = -Wall

all: supermarket cashier customer build_script

supermarket: supermarket.c readConstants.c
	$(CC) $(CFLAGS) -o supermarket supermarket.c readConstants.c

cashier: cashier.c readConstants.c
	$(CC) $(CFLAGS) -o cashier cashier.c readConstants.c

customer: customer.c readConstants.c
	$(CC) $(CFLAGS) -o customer customer.c readConstants.c

build_script: supermarket cashier customer
	bash build.bash

.PHONY: clean

clean:
	rm -f supermarket cashier customer