
prefix = /usr/local

.PHONY: install
install: decals
	mkdir -p $(prefix)/bin
	cp decals $(prefix)/bin

decals: decals.c
	$(CC) -framework Cocoa -o decals decals.c

