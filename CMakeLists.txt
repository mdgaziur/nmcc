cmake_minimum_required(VERSION 3.31)
project(nmcc C)

set(CMAKE_C_STANDARD 90)
add_compile_options(-Wall -g)

include_directories(include)

option(DEBUG "Enable debug information" on)

if(DEBUG)
    add_compile_definitions(-DDEBUG)
endif(DEBUG)

add_executable(nmcc main.c
        include/nmcc/nmstring.h
        libnmcc/string.c
        include/nmcc/nmerror.h
        lex.c
        parse.c
        include/nmcc/nmast.h
        type.c
        validate.c
        codegen.c
        preprocess1.c
        libnmcc/error.c
        libnmcc/file.c
        include/nmcc/nmfile.h
        include/nmcc/nmmust.h
        include/nmcc/preprocess1.h
        include/nmcc/nmdebug.h
        diagnostics.c
        include/nmcc/nmdiagnostics.h
        include/nmcc/nmcolors.h
        include/nmcc/nmutils.h
        libnmcc/utils.c
        include/nmcc/nmfmt.h
        libnmcc/fmt.c
        include/nmcc/nmlex.h
        include/nmcc/nmspan.h
        span.c)
