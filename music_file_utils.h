
#ifndef MUSIC_FILE_UTILS_H
#define MUSIC_FILE_UTILS_H

#include <string.h>
#include <stdlib.h>

typedef struct {
    char *data;
    const char *format;
    int byte_size;
} FileMusic;

long int find_size(const FILE *fp);
const char* get_extension(const char *path);
FileMusic open_file_music(char *path);

#endif

#ifdef FILE_MUSIC_UTILS_IMPLEMENTATION

#include <assert.h>
FileMusic open_file_music(char *audio_path){
    FILE *fptr;
    const char *ext = get_extension(audio_path);
    fptr = fopen(audio_path, "rb");

    if(fptr == NULL){
        assert("File not found");
    }

    long int f_size = find_size(fptr); 

    if (f_size < 0){
        assert("Invalid file");
    }

    char *buffer = malloc(sizeof(char) * f_size);
    fread(buffer, f_size, 1, fptr);

    fclose(fptr);

    return (FileMusic){
        .data = buffer,
        .byte_size = f_size,
        .format = ext 
    };
}

const char* get_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) {
        return NULL;
    }
    return dot; 
}

long int find_size(const FILE *fp) 
{
    // checking if the file exist or not
    if (fp == NULL) {
        printf("File Not Found!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    long int res = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    return res;
}

#endif
