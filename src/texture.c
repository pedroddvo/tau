#include "texture.h"
#include "resource.h"
#include "stb_ds.h"
#include "util.h"
#include <assert.h>

GLenum global_texture_unit = GL_TEXTURE0;

static stbi_uc* texture_load_from_resource(const char* path, int* x, int* y,
                                           int* channels,
                                           int desired_channels) {
    char qualified[64] = {0};
    assert(strlen(path) < 64);
    resource_qualify_path(path, qualified);

    stbi_set_flip_vertically_on_load(true);
    return stbi_load(qualified, x, y, channels, desired_channels);
}

GLenum texture_unit_next() { return global_texture_unit++; }

texture_array_t texture_array_init(size_t clamp_width, size_t clamp_height,
                                   size_t clamp_components) {
    assert(clamp_components == STBI_rgb || clamp_components == STBI_rgb_alpha);
    return (texture_array_t){
        .images = NULL,
        .clamp_width = clamp_width,
        .clamp_height = clamp_height,
        .clamp_components = clamp_components,
    };
}

void texture_array_deinit(texture_array_t* texture_array) {
    for (size_t i = 0; i < arrlen(texture_array->images); i++) {
        stbi_image_free(texture_array->images[i]);
    }

    arrfree(texture_array->images);
}

texture_instance_t texture_array_add(texture_array_t* arr,
                                     const char* texture_path) {
    int img_width = 0, img_height = 0, img_components = 0;
    stbi_uc* img =
        texture_load_from_resource(texture_path, &img_width, &img_height,
                                   &img_components, arr->clamp_components);

    assert_fatalf(img != NULL, "error loading image %s", texture_path);
    assert_fatalf(img_width == arr->clamp_width &&
                      img_height == arr->clamp_height,
                  "error loading image %s into array: array expects "
                  "%d x %d texture, got %d x %d",
                  texture_path, arr->clamp_width, arr->clamp_height, img_width,
                  img_height);
    assert_fatalf(img_components == arr->clamp_components,
                  "error loading image %s into array: array expects "
                  "%d components, got %d",
                  texture_path, arr->clamp_components, img_components);

    arrpush(arr->images, img);
    return arrlen(arr->images) - 1;
}

texture_object_t texture_array_to_object(texture_array_t* arr) {
    texture_object_t object = (texture_object_t){
        .texture_unit = texture_unit_next(),
        .kind = GL_TEXTURE_2D_ARRAY,
    };
    glGenTextures(1, &object.object);

    glActiveTexture(object.texture_unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, object.object);

    GLint format = GL_RGB + arr->clamp_components - STBI_rgb;
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, arr->clamp_width,
                 arr->clamp_height, arrlen(arr->images), 0, format,
                 GL_UNSIGNED_BYTE, NULL);

    for (size_t i = 0; i < arrlen(arr->images); i++) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, arr->clamp_width,
                        arr->clamp_height, 1, format, GL_UNSIGNED_BYTE,
                        arr->images[i]);
    }

    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return object;
}
