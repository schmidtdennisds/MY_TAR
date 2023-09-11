#include "my_tar.h"

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
        string[SIZE_OF_FILESIZE - 1 - i] = (fsize % 10) + '0';
        fsize /= 10;
    }
    for (int k = 0; k < SIZE_OF_FILESIZE - 1 - index; k++) {
        string[k] = '0';
    }
    string[SIZE_OF_FILESIZE - 1] = '\0';
    return string;
}



posix_header* initialize_header(int fd, arguments* argumentNode) {
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

void appendToArchive(int fd_archive, int fd_file, arguments* argumentNode) {
    posix_header* header = initialize_header(fd_file, argumentNode);
    write(fd_archive, header->name, SIZE_OF_NAME);
    write(fd_archive, header->spacer1, SIZE_OF_SPACER_ONE);
    write(fd_archive, header->size, SIZE_OF_FILESIZE);
    write(fd_archive, header->mtime, SIZE_OF_FILESIZE);
    write(fd_archive, header->spacer2, SIZE_OF_SPACER_TWO);

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
            clearBlock(block);
            write(fd_archive, block, BLOCKSIZE - file_size);
        }
        clearBlock(block);
        file_size -= BLOCKSIZE;
    }

    free(block);
    free(header->name);
    free(header->spacer1);
    free(header->size);
    free(header->mtime);
    free(header->spacer2);
    free(header);
}

void write_end_block(int fd_archive){
    char zero_block[BLOCKSIZE] = {'\0'};
    for (int i = BLOCKSIZE; i <= ENDBLOCKSIZE; i += BLOCKSIZE) {
        write(fd_archive, zero_block, BLOCKSIZE);
    }
}

bool createArchive(arguments* argumentNode) {
    if (argumentNode == NULL){
        my_print(2,"Error: No arguments while f flag is active\n");
        return false;
    } else {
        int fd = open(argumentNode->name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (fd == -1) {
            my_print(2,"my_tar: Cannot open ");
            my_print(2, argumentNode->name);
            my_print(2, "\n");
            return false;
        }
        
        //printf("Archive %s created with following arguments:\n", argumentNode->name);

        while(argumentNode->next) {
            arguments* next_argument = argumentNode->next;

            //printf("Argument: %s\n", next_argument->name);
            int fd_file = open(next_argument->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd_file == -1) {
                my_print(2,"my_tar: ");
                my_print(2, next_argument->name);
                my_print(2,": Cannot stat: No such file or directory\n");
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
        my_print(2,"my_tar: Cannot open ");
        my_print(2, argumentNode->name);
        my_print(2, "\n");
        return false;
    }

    char file_name[SIZE_OF_NAME];
    char file_size[SIZE_OF_FILESIZE];
    read(fd, file_name, SIZE_OF_NAME);

    while(strlen(file_name) > 0) {
        my_print(1, file_name);
        my_print(1, "\n");
        lseek(fd, SIZE_OF_SPACER_ONE, SEEK_CUR);
        read(fd, file_size, SIZE_OF_FILESIZE);
        lseek(fd, SIZE_OF_TIME, SEEK_CUR);
        lseek(fd, SIZE_OF_SPACER_TWO, SEEK_CUR);
        //printf("Size: %s\n", file_size);
        int file_size_number = atoi(file_size);
        lseek(fd, atoi(file_size), SEEK_CUR);
        int remaining = BLOCKSIZE - (file_size_number % BLOCKSIZE);
        lseek(fd, remaining, SEEK_CUR);
        read(fd, file_name, SIZE_OF_NAME);
    }

    close(fd);

    return true;
}

bool extractArchive(arguments* argumentNode) {
    int fd = open(argumentNode->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd == -1) {
        my_print(2,"my_tar: Cannot open ");
        my_print(2, argumentNode->name);
        my_print(2, "\n");
        return false;
    }

    char file_name[SIZE_OF_NAME];
    char file_size[SIZE_OF_FILESIZE];
    char file_time[SIZE_OF_TIME];
    read(fd, file_name, SIZE_OF_NAME);

    while(strlen(file_name) > 0) {
        my_print(1, file_name);
        my_print(1, "\n");
        lseek(fd, SIZE_OF_SPACER_ONE, SEEK_CUR);
        read(fd, file_size, SIZE_OF_FILESIZE);
        read(fd, file_time, SIZE_OF_TIME);
        lseek(fd, SIZE_OF_SPACER_TWO, SEEK_CUR);
        //printf("Size: %s\n", file_size);

        bool created = false;
        int fd_file = open(file_name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd_file == -1) {
            fd_file = open(file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            created = true;
            if (fd_file == -1) {
                my_print(2,"my_tar: ");
                my_print(2, file_name);
                my_print(2,": Cannot stat: No such file or directory\n");
                return false;
            }
        }

        struct stat attr;
        lstat(file_name, &attr);
        int seconds = attr.st_mtim.tv_sec;

        int file_size_number = atoi(file_size);
        char buffer[file_size_number];
        read(fd, buffer, file_size_number);
        int remaining = BLOCKSIZE - (file_size_number % BLOCKSIZE);
        lseek(fd, remaining, SEEK_CUR);

        if (created || !created && atoi(file_time) < seconds) {
            write(fd_file, buffer, file_size_number);
        }

        close(fd_file);

        read(fd, file_name, SIZE_OF_NAME);
    }

    close(fd);

    return true;
}

void changeOffsetOfArchive(int fd, char* archive_name) {
    struct stat st;
    stat(archive_name, &st);
    off_t size = st.st_size;

    lseek(fd, size - ENDBLOCKSIZE, SEEK_CUR);
}

bool is_file_newer_than_in_archive(int fd_archive, char* new_file_name) {
    char file_time[SIZE_OF_TIME];
    char file_size[SIZE_OF_FILESIZE];
    char file_name[SIZE_OF_NAME];

    bool is_file_in_archive = false;

    lseek(fd_archive, 0, SEEK_CUR);
    read(fd_archive, file_name, SIZE_OF_NAME);
    //printf("Archive_file_name: %s\n", file_name);

    while(strlen(file_name) > 0) {
        //printf("Archive_file_name: %s\n", file_name);
        if (strcmp(file_name, new_file_name) == 0) {
            is_file_in_archive = true;
            //printf("File found\n");
            lseek(fd_archive, SIZE_OF_SPACER_ONE, SEEK_CUR);
            read(fd_archive, file_size, SIZE_OF_FILESIZE);
            read(fd_archive, file_time, SIZE_OF_TIME);

            struct stat attr;
            lstat(new_file_name, &attr);
            int seconds = attr.st_mtim.tv_sec;

            if (seconds > atoi(file_time)) {
                //printf("Time of new file: %d, time of archive-file: %d\n", seconds, atoi(file_time));
                return true;
            } else {
                lseek(fd_archive, SIZE_OF_SPACER_TWO, SEEK_CUR);
            }
        } else {
            lseek(fd_archive, SIZE_OF_SPACER_ONE, SEEK_CUR);
            read(fd_archive, file_size, SIZE_OF_FILESIZE);
            lseek(fd_archive, SIZE_OF_TIME, SEEK_CUR);
            lseek(fd_archive, SIZE_OF_SPACER_TWO, SEEK_CUR);
        }
        int file_size_number = atoi(file_size);
        lseek(fd_archive, atoi(file_size), SEEK_CUR);
        int remaining = BLOCKSIZE - (file_size_number % BLOCKSIZE);
        lseek(fd_archive, remaining, SEEK_CUR);
        read(fd_archive, file_name, SIZE_OF_NAME);
    }

    if (is_file_in_archive) {
        return false;
    } else {
        return true;
    }
}

bool appendArchive(arguments* argumentNode, bool searchFirst) {
    if (argumentNode == NULL){
        my_print(2,"Error: No arguments while f flag is active\n");
        return false;
    } else {
        char archive_name[SIZE_OF_NAME] = {'\0'};
        strcpy(archive_name, argumentNode->name);

        int fd = open(archive_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (fd == -1) {
            my_print(2,"my_tar: Cannot open ");
            my_print(2, argumentNode->name);
            my_print(2, "\n");
            return false;
        }
        
        //printf("Archive %s appended with following arguments:\n", argumentNode->name);

        while(argumentNode->next) {
            arguments* next_argument = argumentNode->next;

            //printf("Argument: %s\n", next_argument->name);
            int fd_file = open(next_argument->name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd_file == -1) {
                my_print(2,"my_tar: ");
                my_print(2, next_argument->name);
                my_print(2,": Cannot stat: No such file or directory\n");
                return false;
            }
            if (!searchFirst || searchFirst && is_file_newer_than_in_archive(fd, next_argument->name)) {
                lseek(fd, 0, SEEK_CUR);
                changeOffsetOfArchive(fd, archive_name);
                appendToArchive(fd, fd_file, next_argument);
            }
            
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
        if (!appendArchive(argumentNode, false)) {
            result = true;
        }
    } else if (op->u && op->f) {
        if (!appendArchive(argumentNode, true)) {
            result = true;
        }
    }

    free(op);
    free(p_data);
    return result;
}


