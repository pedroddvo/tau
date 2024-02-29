#pragma once
#include <stddef.h>

void resource_qualify_path(const char* path, char* dest);
char* resource_read_file(const char* path, size_t* buf_size);
