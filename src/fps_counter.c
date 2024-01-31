///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "ijo.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_FRAMES 32

uint64_t frame_counter = 0;
uint32_t frame_index = 0;
double last_update = 0;
float frame_delta_times[MAX_FRAMES] = {0};
float average_delta_time = 0;
bool frames_valid = false;

typedef struct DebugFPS
{
    bool flag;
} DebugFPS;

ECS_COMPONENT_DECLARE (DebugFPS);

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void update_fps_system (ecs_iter_t *it)
{
    frame_counter += 1;
    float dt = it->delta_time;

    frame_delta_times[frame_index] = dt;
    frame_index = frame_index + 1;
    if (frame_index >= MAX_FRAMES)
    {
        frame_index = 0;
        frames_valid = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void draw_fps_system (ecs_iter_t *it)
{
    char buffer[100];

    const DebugFPS *debug_fps = ecs_singleton_get (world, DebugFPS);

    if ((debug_fps->flag) && (frames_valid))
    {
        double now = GetTime ();

        if (now - last_update >= 0.5)
        {
            average_delta_time = 0;
            for (int i = 0; i < MAX_FRAMES; i ++)
            {
                average_delta_time += frame_delta_times[i];
            }
            average_delta_time /= MAX_FRAMES;
            last_update = now;
        }

        int x = 4;

        snprintf (buffer, sizeof (buffer), "%5.1f ms", average_delta_time * 1000);
        DrawTextEx (main_font, buffer, (Vector2) {x, 4}, 30, 0, (Color) {0, 0, 0, 255});
        Vector2 ext = MeasureTextEx (main_font, "888.8 ms ", 30, 0);
        x += ext.x;

        snprintf (buffer, sizeof (buffer), "%.1f Hz", 1 / average_delta_time);
        DrawTextEx (main_font, buffer, (Vector2) {x, 4}, 30, 0, (Color) {0, 0, 0, 255});
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init_fps_counter (void)
{
    ECS_SYSTEM (world, update_fps_system, PhaseUpdate, );
    ECS_SYSTEM (world, draw_fps_system, PhaseDrawDebug, );

    ECS_COMPONENT_DEFINE (world, DebugFPS);

    ecs_singleton_set (world, DebugFPS, { false });
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void toggle_fps_counter (void)
{
    DebugFPS *debug_fps = ecs_singleton_get_mut (world, DebugFPS);
    debug_fps->flag = !debug_fps->flag;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
