#include "my_tar_helper.h"

void my_print(int fd, char* string) {
    write(fd, string, strlen(string));
}


void initializeOptions(options* op) {
    op->c = false;
    op->r = false;
    op->f = false;
    op->t = false;
    op->u = false;
    op->x = false;
}

arguments* parsingArguments(int index, int argc, char** argv) {
    arguments* argumentNode = NULL;
    arguments* argumentFirstNode;

    for (int i = index; i < argc; i++) {
        if (argumentNode == NULL) {
            //printf("argumentNode is NULL\n");
            argumentNode = malloc(sizeof(arguments));
            argumentFirstNode = argumentNode;
        } else {
            //printf("New argument after first one\n");
            arguments* new_argument = malloc(sizeof(arguments));
            argumentNode->next = new_argument;
            argumentNode = new_argument;
        }
        //printf("argv equals %s\n", argv[i]);
        argumentNode->name = malloc(strlen(argv[i]) + 1);
        strcpy(argumentNode->name, argv[i]);
        argumentNode->next = NULL;

        //printf("name equals %s\n", argumentNode->name);
    }

    return argumentFirstNode;
}

data* parsing(int argc, char** argv) {

    options* options = malloc(sizeof(options));
    initializeOptions(options);
    arguments* argumentList = NULL;
    

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            argumentList = parsingArguments(i, argc, argv);
            break;
        }
        for (size_t j = 1; j < strlen(argv[i]); j++) {
            switch (argv[i][j]) {
                case 'c': 
                    options->c = true;
                    break;
                case 'r': 
                    options->r = true;
                    break;
                case 'f': 
                    options->f = true;
                    break;
                case 't': 
                    options->t = true;
                    break;
                case 'u': 
                    options->u = true;
                    break;
                case 'x': 
                    options->x = true;
                    break;
            }
        }
    }

    data* result = malloc(sizeof(data));
    result->options = options;
    result->arguments = argumentList;
    return result;
}



void initialize_part_of_header(char** part, int size) {
    *part = malloc(sizeof(char) * size);
    for (int i = 0; i < size; i++) {
        (*part)[i] = '\0';
    }
}

char* my_itoa(int fsize) {
    char* string = malloc(sizeof(char) * SIZE_OF_FILESIZE);
    int fsizeCopy = fsize;
    int index = 0;
    if (fsize == 0) {
        index = 1;
    }
    while (fsizeCopy > 0) {
        fsizeCopy /= 10;
        index++;
    }
    for (int i = 1; i <= index && i < SIZE_OF_FILESIZE; i++) {
        string[SIZE_OF_FILESIZE - 1 - i] = (fsize % 10) + '0';
        fsize /= 10;
    }
    for (int k = 0; k < SIZE_OF_FILESIZE - 1 - index; k++) {
        string[k] = '0';
    }
    string[SIZE_OF_FILESIZE - 1] = '\0';
    return string;
}



posix_header* initialize_header(arguments* argumentNode) {
    posix_header* header = malloc(sizeof(posix_header));
    initialize_part_of_header(&(header->name), SIZE_OF_NAME);
    initialize_part_of_header(&(header->spacer1), SIZE_OF_SPACER_ONE);
    initialize_part_of_header(&(header->size), SIZE_OF_FILESIZE);
    initialize_part_of_header(&(header->mtime), SIZE_OF_TIME);
    initialize_part_of_header(&(header->spacer2), SIZE_OF_SPACER_TWO);

    strncpy(header->name, argumentNode->name, strlen(argumentNode->name));
    struct stat st;
    stat(argumentNode->name, &st);
    off_t size = st.st_size;

    char* file_size_string = my_itoa(size);
    strncpy(header->size, file_size_string, strlen(file_size_string));
    free(file_size_string);


    struct stat attr;
    lstat(argumentNode->name, &attr);
    char* seconds = my_itoa(attr.st_mtim.tv_sec);
    strncpy(header->mtime, seconds, strlen(seconds));

    // time is missing
    return header;
}

void clearBlock(char* block) {
    for (int i = 0; i < BLOCKSIZE; i++) {
        block[i] = '\0';
    }
}