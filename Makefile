all: user FS

user: client/*.c lib/*.c
	gcc -g client/*.c lib/*.c -o user

FS: server/*.c lib/*.c
	gcc -g server/*.c lib/*.c -o FS
