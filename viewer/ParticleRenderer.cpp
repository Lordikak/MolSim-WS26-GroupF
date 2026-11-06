#include "ParticleRenderer.h"

#include "rlgl.h"

ParticleRenderer::ParticleRenderer(std::string shader_dir) : dir_(std::move(shader_dir)) {
    vao_ = rlLoadVertexArray();
    ReloadShader();
}

ParticleRenderer::~ParticleRenderer() {
    rlUnloadShaderBuffer(ssbo_);
    rlUnloadVertexArray(vao_);
    UnloadShader(shader_);
}

void ParticleRenderer::ReloadShader() {
    auto shader = LoadShader((dir_ + "particles.vert").c_str(), (dir_ + "particles.frag").c_str());
    if (shader.id == rlGetShaderIdDefault()) return;
    UnloadShader(shader_);
    shader_ = shader;
    loc_view_ = GetShaderLocation(shader_, "uView");
    loc_proj_ = GetShaderLocation(shader_, "uProj");
    loc_radius_ = GetShaderLocation(shader_, "uRadius");
}

void ParticleRenderer::Upload(const msb::Frame &frame) {
    count_ = frame.n;
    if (count_ == 0) return;

    staging_.resize(count_ * 4);
    for (std::size_t i = 0; i < count_; ++i) {
        staging_[4*i + 0] = static_cast<float>(frame.positions[i][0]);
        staging_[4*i + 1] = static_cast<float>(frame.positions[i][1]);
        staging_[4*i + 2] = static_cast<float>(frame.positions[i][2]);
        staging_[4*i + 3] = 0;
    }
    const auto bytes = static_cast<unsigned>(staging_.size() * sizeof(float));
    if (count_ > capacity_) {
        rlUnloadShaderBuffer(ssbo_);
        ssbo_ = rlLoadShaderBuffer(bytes, staging_.data(), RL_DYNAMIC_DRAW);
        capacity_ = count_;
    } else {
        rlUpdateShaderBuffer(ssbo_, staging_.data(), bytes, 0);
    }
}

void ParticleRenderer::Draw(float radius) const {
    if (count_ == 0) return;
    rlDrawRenderBatchActive();
    rlEnableShader(shader_.id);
    rlSetUniformMatrix(loc_view_, rlGetMatrixModelview());
    rlSetUniformMatrix(loc_proj_, rlGetMatrixProjection());
    rlSetUniform(loc_radius_, &radius, RL_SHADER_UNIFORM_FLOAT, 1);
    rlBindShaderBuffer(ssbo_, 0);
    rlEnableVertexArray(vao_);
    rlDrawVertexArray(0, static_cast<int>(6 * count_));
    rlDisableVertexArray();
    rlDisableShader();
}
