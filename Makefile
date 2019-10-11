all: user FS

user: client/*.c lib/*.c
	gcc -g -Wall -Wextra client/*.c lib/*.c -o user

FS: server/*.c lib/*.c
	gcc -g -Wall -Wextra server/*.c lib/*.c -o FS

clean:
	rm -f user FS
