FILE *get_fd(char *path, char *dir, int flag)
{
	char fname[512];
	FILE *fp;

	memset(fname, '\0', 512 * sizeof(char));
	strcpy(fname, dir);
	strcat(fname, path);

	if (flag == OPEN_READ) {
		if ((fp = fopen(fname, "r")) == NULL) {
			perror("fopen");
			return NULL;
		}
	}
	else {
		if ((fp = fopen(fname, "w+")) == NULL) {
			perror("fopen");
			return NULL;
		}
	}

	return fp;
}