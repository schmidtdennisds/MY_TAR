#ifndef MY_TAR_HELPER
#define MY_TAR_HELPER

#include "my_tar.h"

void my_print(int fd, char* string);
void initializeOptions(options* op);
arguments* parsingArguments(int index, int argc, char** argv);
data* parsing(int argc, char** argv);
void initialize_part_of_header(char** part, int size);
char* my_itoa(int fsize);
posix_header* initialize_header(int fd, arguments* argumentNode);
void clearBlock(char* block);

#endif