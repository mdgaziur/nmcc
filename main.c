#include <stdio.h>
#include <stdlib.h>

#include <nmmust.h>
#include <nmfile.h>

int main(const int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [filename]\n", argv[0]);
        return EXIT_FAILURE;
    }

    NMFile *file = nmfile_open(argv[1]);
    NOT_NULL(file, "Failed to open source file");
    nmfile_exit_if_error(file);

    NMString *str = nmfile_read_to_string(file);
    NOT_NULL(str, "Failed to read source string");

    printf("Content: \n%s\n", nmstring_get_inner(str));

    nmstring_free(str);
    nmfile_close(file);

    return 0;
}
