#include "texture.h"
#include "resource.h"
#include "util.h"

GLuint texture_array::object::global_texture_offset = GL_TEXTURE0;

texture::raw::raw(const std::string& path, int desired_channels) {
    auto qualified = resource::qualify_path(path);
    stbi_set_flip_vertically_on_load(true);
    m_data = stbi_load(qualified.c_str(), &m_width, &m_height, &m_channels,
                       desired_channels);
    assert_fatalf(m_data != NULL, "failed to load image resource/{}", path);
}

texture_array::id texture_array::add_texture(const std::string& path) {
    texture txt(path, m_channels);
    assert_fatalf(txt.width() == m_width && txt.height() == m_height,
                  "texture_array expects dimensions ({}, {}), but texture "
                  "resource/{} has dimensions ({}, {})",
                  m_width, m_height, path, txt.width(), txt.height());
    m_textures.push_back(std::move(txt));
    return m_textures.size() - 1;
}

texture_array::object texture_array::render_to_object(
    std::initializer_list<std::pair<GLenum, GLenum>> options) {
    object obj;
    glGenTextures(1, &obj.texture_object);

    obj.texture_offset = object::global_texture_offset++;
    glActiveTexture(obj.texture_offset);
    glBindTexture(GL_TEXTURE_2D_ARRAY, obj.texture_object);

    assert_fatal(m_channels == STBI_rgb || m_channels == STBI_rgb_alpha,
                 "texture_array can only render into an object with 3 or 4 "
                 "channels (RGB / RGBA)");

    GLint gl_format = GL_RGB + (m_channels - STBI_rgb);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, gl_format, m_width, m_height,
                 m_textures.size(), 0, gl_format, GL_UNSIGNED_BYTE, NULL);

    for (std::size_t i = 0; i < m_textures.size(); i++) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, m_width, m_height, 1,
                        gl_format, GL_UNSIGNED_BYTE, m_textures[i].data());
    }

    for (const auto& [k, v] : options) {
        glTexParameterf(GL_TEXTURE_2D_ARRAY, k, v);
    }

    return obj;
}

void texture_array::object::bind() const {
    glActiveTexture(texture_offset);
}
