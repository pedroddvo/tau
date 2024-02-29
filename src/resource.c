#include "resource.h"
#include "log.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void resource_qualify_path(const char* path, char* dest) {
    strcat(dest, "resource/");
    strcat(dest, path);
}

char* resource_read_file(const char* path, size_t* buf_size) {
    char qualified[64] = {0};
    assert(strlen(path) < 64);
    resource_qualify_path(path, qualified);

    FILE* file = fopen(qualified, "rb");
    if (file == NULL) {
        log_error("failed to read resource %s", qualified);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t _buf_size = ftell(file);
    rewind(file);

    char* buf = malloc(_buf_size + 1);
    assert(buf != NULL);
    buf[_buf_size] = 0;
    fread(buf, 1, _buf_size, file);

    fclose(file);
    if (buf_size != NULL) *buf_size = _buf_size;
    return buf;
}
