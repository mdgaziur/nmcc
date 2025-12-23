cmake_minimum_required(VERSION 3.31)
project(nmcc C)

set(CMAKE_C_STANDARD 90)
add_compile_options(nmcc -Wall)

include_directories(include)

add_executable(nmcc main.c
        include/nmstring.h
        libnmcc/string.c
        include/nmerror.h
        lex.c
        parse.c
        include/nmast.h
        type.c
        validate.c
        codegen.c
        preprocess.c
        error.c
        file.c
        include/nmfile.h
        include/nmmust.h)
