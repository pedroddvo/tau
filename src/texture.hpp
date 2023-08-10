#if !defined(TEXTURE_H)
#define TEXTURE_H

#include <glad/glad.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <vector>

typedef GLuint Texture_Offset;
const Texture_Offset MISSING_TEXTURE = 0;

struct Texture {
    std::shared_ptr<std::vector<uint8_t>> data;
    int height, width, channels;
};

Texture load_texture(const std::string& file);

class Texture_2D_Array {
  public:
    Texture_2D_Array(const std::vector<Texture>& textures);

    void bind() const;

  private:
    GLuint id;
};

class Texture_Registry {
  public:
    Texture_Registry() : textures() {
        textures.push_back(load_texture("block/missing_tile.png"));
    }

    Texture_Offset load_texture_from_file(const std::string& file);
    Texture_2D_Array build_texture_2d_array();
    const Texture& index_texture(Texture_Offset offset);

  private:
    std::vector<Texture> textures;
};

#endif // TEXTURE_H
