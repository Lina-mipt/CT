#include "copy_dir.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <pathname> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DIR *dir_source = opendir(argv[1]);

    if (dir_source == NULL)
    {
        perror("Failed to opendir");
        return 1;
    }

    if (mkdir(argv[2], 0777) == -1)
    {
        perror("mkdir");
        return 1;
    }

    DIR *dir_target = opendir(argv[2]);

    full_copy_dir(dir_source, dir_target);

    return 0;
}
