#include "my_tar.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>


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
    initialize_part_of_header(&(header->name), SIZE_OF_NAME);
    initialize_part_of_header(&(header->size), SIZE_OF_FILESIZE);
    initialize_part_of_header(&(header->mtime), SIZE_OF_TIME);

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

void appendToArchive(int fd_archive, int fd_file, arguments* argumentNode) {
    posix_header* header = initialize_header(fd_file, argumentNode);
    write(fd_archive, header->name, SIZE_OF_NAME);
    write(fd_archive, header->size, SIZE_OF_FILESIZE);
    write(fd_archive, header->mtime, SIZE_OF_FILESIZE);
    write(fd_archive, "\n", 1);

    char* block = malloc(sizeof(char) * BLOCKSIZE);
    clearBlock(block);
    struct stat st;
    stat(argumentNode->name, &st);
    off_t file_size = st.st_size;

    while(file_size > 0) {
        if (file_size / BLOCKSIZE > 0) {
            read(fd_file, block, BLOCKSIZE);
            write(fd_archive, block, BLOCKSIZE);
        } else {
            read(fd_file, block, file_size);
            write(fd_archive, block, file_size);
            write(fd_archive, "\n", 1);
        }
        clearBlock(block);
        file_size -= BLOCKSIZE;
    }

    free(block);
    free(header->name);
    free(header->size);
    free(header->mtime);
    free(header);
}

void write_end_block(int fd_archive){
    char zero_block[BLOCKSIZE] = {'\0'};
    for (int i = BLOCKSIZE; i <= ENDBLOCKSIZE; i += BLOCKSIZE) {
        write(fd_archive, zero_block, BLOCKSIZE);
        if (i != ENDBLOCKSIZE) {
            write(fd_archive, "\n", 1);
        }
    }
}

bool createArchive(arguments* argumentNode) {
    if (argumentNode == NULL){
        printf("Error: No arguments while f flag is active\n");
        return false;
    } else {
        int fd = open(argumentNode->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (fd == -1) {
            printf("Error: something went wrong while creating the archive");
            return false;
        }
        
        printf("Archive %s created with following arguments:\n", argumentNode->name);

        while(argumentNode->next) {
            arguments* next_argument = argumentNode->next;

            printf("Argument: %s\n", next_argument->name);
            int fd_file = open(next_argument->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd_file == -1) {
                printf("Error: something went wrong while opening the file");
                return false;
            }
            appendToArchive(fd, fd_file, next_argument);
            close(fd_file);
            free(argumentNode->name);
            free(argumentNode);
            argumentNode = next_argument;
        }
        write_end_block(fd);
        free(argumentNode->name);
        free(argumentNode);
        close(fd);
    }
    return true;
}

bool readArchive(arguments* argumentNode) {
    int fd = open(argumentNode->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1) {
        printf("Error: something went wrong while creating the archive");
        return false;
    }

    char file_name[SIZE_OF_NAME];
    char file_size[SIZE_OF_FILESIZE];
    read(fd, file_name, SIZE_OF_NAME);

    while(strlen(file_name) > 0) {
        printf("%s\n", file_name);
        read(fd, file_size, SIZE_OF_FILESIZE);
        lseek(fd, SIZE_OF_TIME, SEEK_CUR);
        //printf("Size: %s\n", file_size);
        int file_size_number = atoi(file_size);
        if (file_size_number == 0) {
            lseek(fd, 1, SEEK_CUR);
        } else {
            lseek(fd, atoi(file_size) + 2, SEEK_CUR);
        }
        
        read(fd, file_name, SIZE_OF_NAME);
    }

    close(fd);

    return true;
}

bool extractArchive(arguments* argumentNode) {
    int fd = open(argumentNode->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1) {
        printf("Error: something went wrong while creating the archive");
        return false;
    }

    char file_name[SIZE_OF_NAME];
    char file_size[SIZE_OF_FILESIZE];
    char file_time[SIZE_OF_TIME];
    read(fd, file_name, SIZE_OF_NAME);

    while(strlen(file_name) > 0) {
        printf("%s\n", file_name);
        read(fd, file_size, SIZE_OF_FILESIZE);
        read(fd, file_time, SIZE_OF_TIME);
        //printf("Size: %s\n", file_size);
        lseek(fd, 1, SEEK_CUR);

        bool created = false;
        int fd_file = open(file_name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd_file == -1) {
            fd_file = open(file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            created = true;
            if (fd_file == -1) {
                printf("Error: something went wrong while opening the file");
                return false;
            }
        }

        struct stat attr;
        lstat(file_name, &attr);
        int seconds = attr.st_mtim.tv_sec;

        int file_size_number = atoi(file_size);
        char buffer[file_size_number];
        read(fd, buffer, file_size_number);

        if (created || !created && atoi(file_time) < seconds) {
            write(fd_file, buffer, file_size_number);
        }

        close(fd_file);

        if (file_size_number != 0) {
            lseek(fd, 1, SEEK_CUR);
        }

        read(fd, file_name, SIZE_OF_NAME);
    }

    close(fd);

    return true;
}

void changeOffsetOfArchive(int fd, char* archive_name) {
    struct stat st;
    stat(archive_name, &st);
    off_t size = st.st_size;

    lseek(fd, size - ENDBLOCKSIZE - 1, SEEK_CUR);
}

bool appendArchive(arguments* argumentNode) {
    if (argumentNode == NULL){
        printf("Error: No arguments while f flag is active\n");
        return false;
    } else {
        int fd = open(argumentNode->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (fd == -1) {
            printf("Error: something went wrong while opening the archive");
            return false;
        }

        changeOffsetOfArchive(fd, argumentNode->name);
        
        printf("Archive %s appended with following arguments:\n", argumentNode->name);

        while(argumentNode->next) {
            arguments* next_argument = argumentNode->next;

            printf("Argument: %s\n", next_argument->name);
            int fd_file = open(next_argument->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd_file == -1) {
                printf("Error: something went wrong while opening the file");
                return false;
            }
            appendToArchive(fd, fd_file, next_argument);
            close(fd_file);
            free(argumentNode->name);
            free(argumentNode);
            argumentNode = next_argument;
        }
        write_end_block(fd);
        free(argumentNode->name);
        free(argumentNode);
        close(fd);
    }
    return true;
}

int main(int argc, char** argv) {

    data* p_data = parsing(argc, argv);
    options* op = p_data->options;
    arguments* argumentNode = p_data->arguments;

    //printf("Options: \nc:%d, r:%d, f:%d, t:%d, u:%d, x:%d\n", op->c, op->r, op->f, op->t, op->u, op->x);

    bool result = false;
    if (op->c && op->f) {
        if (!createArchive(argumentNode)) {
            result = true;
        }
    } else if (op->t && op->f) {
        if (!readArchive(argumentNode)) {
            result = true;
        }
    } else if (op->x && op->f) {
        if (!extractArchive(argumentNode)) {
            result = true;
        }
    } else if (op->r && op->f) {
        if (!appendArchive(argumentNode)) {
            result = true;
        }
    }

    free(op);
    free(p_data);
    return result;
}


