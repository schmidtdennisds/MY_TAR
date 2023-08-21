#ifndef MY_TAR
#define MY_TAR

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


typedef struct s_options {
    bool c;
    bool r;
    bool f;
    bool t;
    bool u;
    bool x;
} options;

typedef struct s_posix_header
{ 
  char* name;
  char* size;
  char* mtime;
} posix_header;

typedef struct s_arguments {
    char* name;
    struct s_arguments* next;
} arguments;

typedef struct s_data {
    options* options;
    arguments* arguments;
} data;

#define SIZE_OF_NAME 100
#define SIZE_OF_FILESIZE 12
#define SIZE_OF_TIME 12
#define BLOCKSIZE 512
#define ENDBLOCKSIZE 1024

#endif