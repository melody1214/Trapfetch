

int main(int argc, char *argv[]) {
  if (!startup_child(argc, argv)) {
    perror("startup_child");
    return -1;
  }
}