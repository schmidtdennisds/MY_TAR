#include "my_tar.h"
#include <stdlib.h>


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

int main(int argc, char** argv) {

    data* data = parsing(argc, argv);
    options* op = data->options;
    arguments* argumentNode = data->arguments;

    printf("Options: \n c:%d, r:%d, f:%d, t:%d, u:%d, x:%d\n", op->c, op->r, op->f, op->t, op->u, op->x);

    if (op->c && op->f) {
        if (argumentNode == NULL){
            printf("Error: No arguments while f flag is active\n");
        } else {
            int fd = open(argumentNode->name, O_CREAT | O_RDWR);
            printf("Archive %s created with following arguments:\n", argumentNode->name);

            while(argumentNode->next) {
                arguments* next_argument = argumentNode->next;
                printf("Argument: %s\n", next_argument->name);
                argumentNode = next_argument;
            }
        }
    }
}


