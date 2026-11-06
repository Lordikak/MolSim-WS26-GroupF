#include <cfloat>
#include <string>

#include "ParticleRenderer.h"
#include "raylib.h"
#include "trajectory/MSBFReader.h"

void FrameParticles(Camera3D &cam, const msb::Frame &f) {
    if (f.positions.empty()) return;
    Vector3 min{ FLT_MAX, FLT_MAX, FLT_MAX }, max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (const Vec3 &x : f.positions) {
        min = { std::min(min.x, (float)x[0]), std::min(min.y, (float)x[1]), std::min(min.z, (float)x[2]) };
        max = { std::max(max.x, (float)x[0]), std::max(max.y, (float)x[1]), std::max(max.z, (float)x[2]) };
    }
    const Vector3 c{ (min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2 };
    const float hw = (max.x - min.x) / 2, hh = (max.y - min.y) / 2;
    const float aspect = (float)GetScreenWidth() / GetScreenHeight();
    const float half = std::max(hh, hw / aspect) * 1.15f;
    const float dist = half / tanf(cam.fovy * DEG2RAD / 2) + (max.z - min.z) / 2;
    cam.target = c;
    cam.position = { c.x, c.y, c.z + dist };
    cam.up = { 0, 1, 0 };
}

int main (int argc, char* argv[]) {
    msb::MSBFReader reader(argv[1]);
    const auto &h = reader.header();

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(1280, 720, "MolSimViewer");

    const std::string font_path = std::string(GetApplicationDirectory()) + "assets/external/inter/Inter-Regular.ttf";
    Font font = LoadFontEx(font_path.c_str(), 36, nullptr, 0);
    if (font.texture.id == GetFontDefault().texture.id)
        TraceLog(LOG_WARNING, "VIEWER: font not found at %s", font_path.c_str());
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);


    SetTargetFPS(60);
    Camera3D cam{.position = {0,0,-50}, .target = {0,0,0}, .up = {0,1,0}, .fovy = 45, .projection = CAMERA_PERSPECTIVE};

    double sim_time = h.start_time;
    double speed = 1.0;
    bool paused = false;

    ParticleRenderer renderer(std::string(GetApplicationDirectory()) + "assets/shaders/");
    std::uint32_t uploaded = msb::Frame::kNone;

    while (!WindowShouldClose()) {
        reader.Refresh();
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
            if (!paused) sim_time += GetFrameTime() * speed;
        }
        if (IsKeyPressed(KEY_UP)) speed *= 2;
        if (IsKeyPressed(KEY_DOWN)) speed /= 2;
        if (IsKeyPressed(KEY_R)) sim_time = h.start_time;

        const msb::Frame &f = reader.LoadAtTime(sim_time);

        if (IsKeyPressed(KEY_F)) FrameParticles(cam, f);
        if (f.step != uploaded) { renderer.Upload(f); uploaded = f.step; }
        if (IsKeyPressed(KEY_F5)) renderer.ReloadShader();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(cam);
        renderer.Draw(.5);
        EndMode3D();
        DrawTextEx(font, TextFormat("step %u  t=%.3f  x%.2f", f.step, sim_time, speed), {10, 10}, 18, .5, RAYWHITE);
        EndDrawing();

        if (!paused) sim_time += GetFrameTime() * speed;
    }
    CloseWindow();

}