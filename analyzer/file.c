int get_fd(char *path, char *dir, int flag)
{
	char fname[512];
	int fd;

	memset(fname, '\0', 512 * sizeof(char));
	strcpy(fname, dir);
	strcat(fname, path);

	if (flag == OPEN_READ) {
		if ((fd = open(fname, O_RDONLY)) < 0) {
			perror("open");
			return -1;
		}
	}
	else {
		if ((fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
			perror("open");
			return -1;
		}
	}

	return fd;
}