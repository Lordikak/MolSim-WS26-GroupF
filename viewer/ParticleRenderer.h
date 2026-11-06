#pragma once
#include <string>

#include "raylib.h"
#include "trajectory/MSBFReader.h"

class ParticleRenderer {
    std::string dir_;
    Shader shader_{};
    int loc_view_ = -1, loc_proj_ = -1, loc_radius_ = -1;
    unsigned vao_ = 0, ssbo_ = 0;
    std::size_t capacity_ = 0, count_ = 0;
    std::vector<float> staging_;

public:
    explicit ParticleRenderer(std::string shader_dir);
    ~ParticleRenderer();

    void Upload(const msb::Frame& frame);
    void Draw(float radius) const;
    void ReloadShader();
};

