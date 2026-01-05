//
// Created by MD Gaziur Rahman Noor on 24/12/25.
//

#ifndef NMCOLORS_H
#define NMCOLORS_H

/* --- Standard ANSI Modifiers --- */
#define ANSI_RESET      "\x1b[0m"
#define ANSI_BOLD       "\x1b[1m"
#define ANSI_DIM        "\x1b[2m"
#define ANSI_ITALIC     "\x1b[3m"
#define ANSI_UNDERLINE  "\x1b[4m"

/* --- Standard ANSI Foreground Colors --- */
#define ANSI_FG_BLACK   "\x1b[30m"
#define ANSI_FG_RED     "\x1b[31m"
#define ANSI_FG_GREEN   "\x1b[32m"
#define ANSI_FG_YELLOW  "\x1b[33m"
#define ANSI_FG_BLUE    "\x1b[34m"
#define ANSI_FG_MAGENTA "\x1b[35m"
#define ANSI_FG_CYAN    "\x1b[36m"
#define ANSI_FG_WHITE   "\x1b[37m"

/* --- Semantic Diagnostic Colors --- */
/* Use these in your actual code to keep styles consistent */

#define DIAG_CERROR      ANSI_BOLD ANSI_FG_RED     /* Fatal errors */
#define DIAG_CWARN       ANSI_BOLD ANSI_FG_YELLOW  /* Warnings */
#define DIAG_CINFO       ANSI_BOLD ANSI_FG_CYAN    /* Informational notes */
#define DIAG_CSUCCESS    ANSI_BOLD ANSI_FG_GREEN   /* Success messages */
#define DIAG_CFILE       ANSI_BOLD ANSI_FG_WHITE   /* Filenames */
#define DIAG_CLINE       ANSI_FG_BLUE              /* Line numbers */
#define DIAG_CCODE       ANSI_FG_WHITE             /* Source code snippets */
#define DIAG_CCARET      ANSI_BOLD ANSI_FG_GREEN   /* The ^ pointing to error */

#endif //NMCOLORS_H
