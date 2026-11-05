#include "raylib.h"
#include "trajectory/MSBFReader.h"

int main (int argc, char* argv[]) {
    msb::MSBFReader reader(argv[1]);
    const auto &h = reader.header();

    InitWindow(1280, 720, "MolSimViewer");
    SetTargetFPS(60);
    Camera3D cam{.position = {0,0,-10}, .target = {0,0,0}, .up = {0,1,0}, .fovy = 45, .projection = CAMERA_PERSPECTIVE};

    double sim_time = h.start_time;
    double speed = 1.0;
    bool paused = false;

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

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(cam);
        for (const Vec3 &p : f.positions)
            DrawSphereEx({static_cast<float>(p[0]), static_cast<float>(p[1]), static_cast<float>(p[2])}, 0.1f, 6, 6, SKYBLUE);
        DrawGrid(20, 1.0f);
        EndMode3D();
        DrawText(TextFormat("step %u  t=%.3f  x%.2f", f.step, sim_time, speed), 10, 10, 20, RAYWHITE);
        EndDrawing();

        if (!paused) sim_time += GetFrameTime() * speed;
    }
    CloseWindow();

}