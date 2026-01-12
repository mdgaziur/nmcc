#include <stdio.h>
#include <unistd.h>

#include <nmcc/nmfile.h>
#include <nmcc/nmmust.h>
// #include <nmcc/preprocess1.h>

#include "nmcc/nmcolors.h"
#include "nmcc/nmdebug.h"
#include "nmcc/nmdiagnostics.h"
#include "nmcc/nmlex.h"
#include "nmcc/preprocess.h"

void print_usage(const char *progname) {
  fprintf(stderr, "Usage: %s [options] [filename]\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "-I\t\t\t\tInclude directory\n");
}

int main(const int argc, char *argv[]) {
  const char *progname;
  if (argc >= 1) {
    progname = argv[0];
  } else {
    progname = "nmcc";
  }

  int opt;
  while ((opt = getopt(argc, argv, "I:h")) != -1) {
    switch (opt) {
      case 'I':
        NMDEBUG("-I=%s\n", optarg);
        preprocess_add_include_directory(nmstring_new_from_str(optarg));
        break;
      case 'h':
        print_usage(progname);
        return 0;
      case '?':
        print_usage(progname);
        return EXIT_FAILURE;
    }
  }

  if (optind == argc) {
    fprintf(stderr, DIAG_CERROR "error:" ANSI_RESET " No file specified\n");
    print_usage(progname);
    return EXIT_FAILURE;
  }

  if (optind < argc - 1) {
    fprintf(stderr, DIAG_CERROR "error:" ANSI_RESET
                                " only one file is currently supported!\n");
    print_usage(progname);
    return EXIT_FAILURE;
  }

  NMFile *file = nmfile_open(argv[optind]);
  NOT_NULL(file, "Failed to open source file");
  nmfile_exit_if_error(file);

  NMString *file_data = nmfile_read_to_string(file);

  NMString *preprocessed_code = nmstring_new();
  bool has_error = preprocess_code(file, preprocessed_code);
  NMDEBUG("has_error: %s\n", has_error ? "true" : "false");
  printf("%s\n", S(preprocessed_code));

  nmstring_free(preprocessed_code);
  nmstring_free(file_data);
  nmfile_close(file);

  return 0;
}
