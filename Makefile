all:
	g++ src/*.cc -o uppc -levent -g

clean:
	rm -f uppc uppc.exe

