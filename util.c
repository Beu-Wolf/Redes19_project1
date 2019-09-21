#include "util.h"
#include <stdlib.h>
#include <string.h>

char **tokenize(char *string) {
	int numArgs = 10;
	int i = 0;

	char **args = (char **)malloc(numArgs * sizeof(char *));
	if (!args) exit(1);

	args[i] = strtok(string, " ");

	while (args[i] != NULL) {
		i++;
		if (i == numArgs) {
			numArgs *= 2;
			args = realloc(args, numArgs * sizeof(char *));
			if (!args) exit(1);
		}
		args[i] = strtok(NULL, " ");
	}

	return args;
}
