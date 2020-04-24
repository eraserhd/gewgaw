
prefix = /usr/local

.PHONY: install
install: gewgaw
	mkdir -p $(prefix)/bin
	cp gewgaw $(prefix)/bin

gewgaw: gewgaw.c
	$(CC) -framework Cocoa -o gewgaw gewgaw.c

