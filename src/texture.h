#pragma once
#include <cstddef>
#include <glad/glad.h>
#include <memory>
#include <stb_image.h>
#include <vector>

class texture {
  public:
    texture(const std::string& path, int desired_channels = 0)
        : m_raw(std::make_unique<raw>(path, desired_channels)) {}

    int height() const { return m_raw->m_height; }
    int width() const { return m_raw->m_width; }
    const stbi_uc* const data() const { return m_raw->m_data; }

  private:
    struct raw {
        raw(const std::string& path, int desired_channels = 0);
        ~raw() { stbi_image_free(m_data); }

        stbi_uc* m_data;
        int m_height, m_width, m_channels;
    };

    std::unique_ptr<raw> m_raw;
};

class texture_array {
  public:
    struct object {
        GLuint texture_object, texture_offset;

        void bind() const;
        static GLuint global_texture_offset;
    };

    using id = std::size_t;
    texture_array(std::size_t width, std::size_t height, int channels)
        : m_width(width), m_height(height), m_channels(channels) {}

    id add_texture(const std::string& path);
    object render_to_object(std::initializer_list<std::pair<GLenum, GLenum>> options);

    const std::size_t m_width, m_height;
    const int m_channels;

  private:
    std::vector<texture> m_textures;
};
