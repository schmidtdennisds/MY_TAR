#include "my_tar.h"


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
        argumentNode->name = malloc(strlen(argv[i]));
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
        for (int j = 1; j < strlen(argv[i]); j++) {
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



void initialize_part_of_header(char* part, int size) {
    part = malloc(sizeof(char) * size);
    for (int i = 0; i < size; i++) {
        part[i] = '\0';
    }
}

char* my_itoa(int fsize) {
    char* string = malloc(sizeof(char) * SIZE_OF_FILESIZE);
    int fsizeCopy = fsize;
    int index = 0;
    while (fsizeCopy % 10 != 0) {
        fsizeCopy /= 10;
        index++;
    }
    for (int i = 1; i <= index && i < SIZE_OF_FILESIZE; i++) {
        string[index - i] = (fsize % 10) + '0';
        fsize /= 10;
    }
    for (int k = index; k < SIZE_OF_FILESIZE; k++) {
        string[k] = '\0';
    }
    return string;
}



posix_header* initialize_header(int fd, arguments* argumentNode) {
    posix_header* header = malloc(sizeof(posix_header));
    initialize_part_of_header(header->name, SIZE_OF_NAME);
    initialize_part_of_header(header->size, SIZE_OF_FILESIZE);
    initialize_part_of_header(header->mtime, SIZE_OF_TIME);

    strncpy(header->name, argumentNode->name, SIZE_OF_NAME);
    int fsize = lseek(fd, 0, SEEK_END);
    char* file_size_string = my_itoa(fsize);
    strncpy(header->size, file_size_string, SIZE_OF_FILESIZE);

    return header;
}

bool appendToArchive(int fd_archive, int fd_file, arguments* argumentNode) {
    posix_header* header = initialize_header(fd_file, argumentNode);

    


}

bool createArchive(arguments* argumentNode) {
    if (argumentNode == NULL){
        printf("Error: No arguments while f flag is active\n");
        return true;
    } else {
        int fd = open(argumentNode->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (fd == -1) {
            printf("Error: something went wrong while creating the archive");
            return true;
        }
        
        printf("Archive %s created with following arguments:\n", argumentNode->name);

        while(argumentNode->next) {
            arguments* next_argument = argumentNode->next;

            printf("Argument: %s\n", next_argument->name);
            int fd_file = open(argumentNode->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd_file == -1) {
                printf("Error: something went wrong while opening the file");
                return true;
            }
            appendToArchive(fd, fd_file, argumentNode);

            argumentNode = next_argument;
        }
    }
    return false;
}

int main(int argc, char** argv) {

    data* data = parsing(argc, argv);
    options* op = data->options;
    arguments* argumentNode = data->arguments;

    printf("Options: \nc:%d, r:%d, f:%d, t:%d, u:%d, x:%d\n", op->c, op->r, op->f, op->t, op->u, op->x);

    if (op->c && op->f) {
        if (!createArchive(argumentNode)) {
            return 1;
        }
    }
}


