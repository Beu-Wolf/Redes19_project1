all: user FS

user: user.c
	gcc user.c -o user

FS: FS.c
	gcc FC.c -o FC