all: user FS

user: user.c
	gcc -g user.c -o user

FS: FS.c
	gcc -g FS.c -o FS