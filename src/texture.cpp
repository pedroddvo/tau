#include "texture.hpp"

#include "resource.hpp"

Texture_Offset
Texture_Registry::load_texture_from_file(const std::string& file) {
    auto texture = load_texture(file);

    if (texture.data == nullptr) return MISSING_TEXTURE;

    textures.push_back(texture);
    return textures.size() - 1;
}

const Texture& Texture_Registry::index_texture(Texture_Offset offset) {
    assert(offset >= 0 && offset < textures.size());
    return textures[offset];
}

Texture load_texture(const std::string& file) {
    const auto path = resource::canonicalize_path(file);

    Texture texture;
    uint8_t* data = stbi_load(path.c_str(), &texture.width, &texture.height,
                              &texture.channels, 0);

    if (data == NULL) {
        spdlog::error("failed to load texture {}", path);
        texture.data = nullptr;
    } else {
        texture.data = std::make_shared<std::vector<uint8_t>>(
            data, data + (texture.width * texture.height * texture.channels));
    }

	assert(texture.channels == 3 || texture.channels == 4);

    return texture;
}

Texture_2D_Array::Texture_2D_Array(const std::vector<Texture>& textures) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (textures.size() == 0) return;

    const auto texture_width = textures[0].width,
               texture_height = textures[0].height,
			   texture_channels = textures[0].channels;

    std::vector<uint8_t> data;

    for (auto&& texture : textures) {
        assert(texture.width == texture_width);
        assert(texture.height == texture_height);
        assert(texture.channels == texture_channels);

        data.insert(data.end(), texture.data->begin(), texture.data->end());
    }

	const auto texture_kind = texture_channels == 3 ? GL_RGB : GL_RGBA;

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, texture_kind, texture_width, texture_height,
                 textures.size(), 0, texture_kind, GL_UNSIGNED_BYTE, data.data());
}

void Texture_2D_Array::bind() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

Texture_2D_Array Texture_Registry::build_texture_2d_array() {
    return Texture_2D_Array(textures);
}
