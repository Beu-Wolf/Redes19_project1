all: user FS

user: user.c
	gcc -g user.c util.c -o user

FS: FS.c
	gcc -g FS.c util.c -o FS
