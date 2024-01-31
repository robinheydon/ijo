#include <ijo.h>

ecs_world_t *world = NULL;

uint64_t frame_counter = 0;
Font main_font;

void draw_frame_counter (void)
{
    char buffer[100];
    snprintf (buffer, sizeof (buffer), "%ld", frame_counter);
    DrawTextEx (main_font, buffer, (Vector2) {4, 4}, 30, 0, (Color) {0, 0, 0, 255});
}

void move_system (ecs_iter_t *it)
{
    float dt = it->delta_time;
    Position *p = ecs_field(it, Position, 1);
    const Velocity *v = ecs_field(it, Velocity, 2);

    for (int i = 0; i < it->count; i ++) {
        p[i].x += v[i].dx * dt;
        p[i].y += v[i].dy * dt;
    }
}

void fps_system (ecs_iter_t *it)
{
    float dt = it->delta_time;
    printf ("fps system %f %f\n", dt, 1/dt);
}

void init_world (void)
{
    world = ecs_init ();

    ECS_COMPONENT_DEFINE (world, Position);
    ECS_COMPONENT_DEFINE (world, Velocity);

    ECS_SYSTEM_DEFINE (world, move_system, EcsOnUpdate, Position, Velocity);
    ECS_SYSTEM_DEFINE (world, fps_system, EcsOnUpdate, );
}

void create_default_state (void)
{
    ecs_entity_t e = ecs_new_id (world);

    ecs_set (world, e, Position, {1, 2});
    ecs_set (world, e, Velocity, {3, 4});
}

void deinit_world (void)
{
    ecs_fini (world);
}

void process_inputs (void)
{
    int key = GetKeyPressed ();
    while (key)
    {
        printf ("Key %d\n", key);
        key = GetKeyPressed ();
    }

    int ch = GetCharPressed ();
    while (ch)
    {
        printf ("Char %d\n", ch);
        ch = GetCharPressed ();
    }
}

int main (int argc, char **argv)
{
    SetTraceLogLevel (LOG_WARNING);
    SetConfigFlags (FLAG_MSAA_4X_HINT);
    InitWindow (1280, 720, "ijo");
    SetWindowState (FLAG_WINDOW_RESIZABLE |
                    FLAG_VSYNC_HINT);

    init_world ();

    create_default_state ();

    main_font = LoadFontFromMemory (".otf", atkinson_regular, atkinson_regular_len, 30, NULL, 0);

    while (!WindowShouldClose ())
    {
        frame_counter += 1;
        ClearBackground ((Color) {255,255,255,255});

        BeginDrawing ();
        draw_frame_counter ();
        EndDrawing ();

        ecs_progress (world, 0);

        process_inputs ();
    }

    deinit_world ();
    CloseWindow ();

    return 0;
}
