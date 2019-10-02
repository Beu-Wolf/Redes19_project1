all: user FS

user: client/*.c lib/*.c
	gcc -g -Wall client/*.c lib/*.c -o user

FS: server/*.c lib/*.c
	gcc -g -Wall server/*.c lib/*.c -o FS

clean:
	rm -f user FS
