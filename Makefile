
prefix = /usr/local

.PHONY: install
install: gewgaw
	mkdir -p $(prefix)/bin/
	cp gewgaw gewgaw-yabai-focus-window $(prefix)/bin/

gewgaw: gewgaw.c
	$(CC) -framework Cocoa -o gewgaw gewgaw.c

