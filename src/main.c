///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include <ijo.h>

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

ecs_world_t *world = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

ecs_entity_t PhaseProcessInputs = 0;
ecs_entity_t PhaseBeforeUpdate = 0;
ecs_entity_t PhasePreUpdate = 0;
ecs_entity_t PhaseUpdate = 0;
ecs_entity_t PhaseAfterUpdate = 0;
ecs_entity_t PhaseBeginDrawing = 0;
ecs_entity_t PhaseBegin3D = 0;
ecs_entity_t PhaseDraw3D = 0;
ecs_entity_t PhaseEnd3D = 0;
ecs_entity_t PhaseBegin2D = 0;
ecs_entity_t PhaseDraw2D = 0;
ecs_entity_t PhaseDrawDebug = 0;
ecs_entity_t PhaseEnd2D = 0;
ecs_entity_t PhaseEndDrawing = 0;

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

Font main_font;

double sim_time = 0.0;
double sim_delta_time = 0.0;
double sim_speed = 1.0;

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void toggle_fullscreen (void)
{
    if (IsWindowMaximized ())
    {
        ToggleBorderlessWindowed ();
    }
    else
    {
        ToggleBorderlessWindowed ();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct KeyBinding {
    KeyboardKey key;
    Modifier mod;
    void (*callback) (void);
} KeyBinding;

#define MAX_KEY_BINDINGS 1024

int num_key_bindings = 0;

KeyBinding key_bindings[MAX_KEY_BINDINGS] = {0};

void set_key_binding (KeyboardKey key, Modifier mod, void (*callback) (void))
{
    for (int i = 0; i < num_key_bindings; i ++)
    {
        if ((key_bindings[i].key == key) && (key_bindings[i].mod == mod))
        {
            if (callback == NULL)
            {
                num_key_bindings -= 1;
                key_bindings[i].key = key_bindings[num_key_bindings].key;
                key_bindings[i].mod = key_bindings[num_key_bindings].mod;
                key_bindings[i].callback = key_bindings[num_key_bindings].callback;
                return;
            }
            else
            {
                key_bindings[i].callback = callback;
                return;
            }
        }
    }

    if (callback == NULL)
    {
        return;
    }

    if (num_key_bindings < MAX_KEY_BINDINGS)
    {
        key_bindings[num_key_bindings].key = key;
        key_bindings[num_key_bindings].mod = mod;
        key_bindings[num_key_bindings].callback = callback;
        num_key_bindings += 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void process_inputs_system (ecs_iter_t *it)
{
    int key = GetKeyPressed ();
    int mod = 0;
    if (IsKeyDown (KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT))
    {
        mod |= 1;
    }
    if (IsKeyDown (KEY_LEFT_CONTROL) || IsKeyDown (KEY_RIGHT_CONTROL))
    {
        mod |= 2;
    }
    while (key)
    {
        for (int i = 0; i < num_key_bindings; i ++)
        {
            if ((key_bindings[i].key == key) && (key_bindings[i].mod == mod))
            {
                key_bindings[i].callback ();
                break;
            }
        }
        key = GetKeyPressed ();
    }

    int ch = GetCharPressed ();
    while (ch)
    {
        printf ("%08x\n", ch);
        ch = GetCharPressed ();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init_phases (void)
{
    struct {
        const char *name;
        ecs_entity_t *entity;
    } phases[] = {
        { "PhaseProcessInputs", &PhaseProcessInputs, },
        { "PhaseBeforeUpdate", &PhaseBeforeUpdate, },
        { "PhasePreUpdate", &PhasePreUpdate, },
        { "PhaseUpdate", &PhaseUpdate, },
        { "PhaseAfterUpdate", &PhaseAfterUpdate, },
        { "PhaseBeginDrawing", &PhaseBeginDrawing, },
        { "PhaseBegin3D", &PhaseBegin3D, },
        { "PhaseDraw3D", &PhaseDraw3D, },
        { "PhaseEnd3D", &PhaseEnd3D, },
        { "PhaseBegin2D", &PhaseBegin2D, },
        { "PhaseDraw2D", &PhaseDraw2D, },
        { "PhaseEnd2D", &PhaseEnd2D, },
        { "PhaseDrawDebug", &PhaseDrawDebug, },
        { "PhaseEndDrawing", &PhaseEndDrawing, },
    };

    ecs_entity_t last_phase = 0;

    for (int i = 0; i < sizeof (phases) / sizeof (phases[0]); i ++)
    {
        ecs_entity_t phase = ecs_new_w_id (world, EcsPhase);
        ecs_set_name (world, phase, phases[i].name);
        *phases[i].entity = phase;

        if (last_phase)
        {
            ecs_add_pair (world, phase, EcsDependsOn, last_phase);
        }
        else
        {
            ecs_add_pair (world, phase, EcsDependsOn, EcsOnUpdate);
        }
        last_phase = phase;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void update_sim_time_system (ecs_iter_t *it)
{
    sim_delta_time = it->delta_time * sim_speed;
    sim_time += sim_delta_time;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void debug_sim_time_system (ecs_iter_t *it)
{
    char buffer[100];

    uint64_t int_sim_time = sim_time;
    uint64_t sec = int_sim_time % 60; int_sim_time /= 60;
    uint64_t min = int_sim_time % 60; int_sim_time /= 60;
    uint64_t hr = int_sim_time % 24; int_sim_time /= 24;
    uint64_t day = int_sim_time % 7; int_sim_time /= 7;

    snprintf (buffer, sizeof (buffer), "%ld:%02ld:%02ld:%02ld (x%0.3f)", day, hr, min, sec, sim_speed);
    DrawTextEx (main_font, buffer, (Vector2) {4, 34}, 30, 0, (Color) {0, 0, 0, 255});
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init_world (void)
{
    world = ecs_init ();

    ecs_set_threads (world, 8);

    ecs_singleton_set (world, EcsRest, {0});
    ECS_IMPORT (world, FlecsMonitor);

    init_phases ();
    init_components ();
    init_geocodes ();

    ECS_SYSTEM (world, update_sim_time_system, EcsOnLoad, );
    ECS_SYSTEM (world, debug_sim_time_system, PhaseDrawDebug, );

    ECS_SYSTEM (world, process_inputs_system, PhaseProcessInputs, );

    init_fps_counter ();
    init_trees ();

    ECS_SYSTEM (world, move_system, PhaseUpdate, Position, Velocity);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void create_default_state (void)
{
    ecs_entity_t e = ecs_new_id (world);

    ecs_set (world, e, Position, {1, 2});
    ecs_set (world, e, Velocity, {3, 4});
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void deinit_world (void)
{
    ecs_fini (world);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void set_speed_zero (void) { sim_speed = 0; }
void set_speed_one (void) { sim_speed = 1; }
void set_speed_two (void) { sim_speed = 2; }
void set_speed_three (void) { sim_speed = 4; }
void set_speed_four (void) { sim_speed = 8; }
void set_speed_five (void) { sim_speed = 15; }
void set_speed_six (void) { sim_speed = 60; }
void set_speed_seven (void) { sim_speed = 0.125; }
void set_speed_eight (void) { sim_speed = 0.25; }
void set_speed_nine (void) { sim_speed = 0.5; }

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv)
{
    srand (time (0));

    SetTraceLogLevel (LOG_WARNING);
    SetConfigFlags (FLAG_MSAA_4X_HINT);
    InitWindow (1280, 720, "ijo");
    SetWindowState (
        FLAG_WINDOW_RESIZABLE |
        FLAG_VSYNC_HINT |
        0);

    main_font = LoadFontFromMemory (".otf", atkinson_regular, atkinson_regular_len, 30, NULL, 0);

    init_world ();

    create_default_state ();

    set_key_binding (KEY_F1, None, toggle_fps_counter);
    set_key_binding (KEY_F11, None, toggle_fullscreen);
    set_key_binding (KEY_ZERO, None, set_speed_zero);
    set_key_binding (KEY_ONE, None, set_speed_one);
    set_key_binding (KEY_TWO, None, set_speed_two);
    set_key_binding (KEY_THREE, None, set_speed_three);
    set_key_binding (KEY_FOUR, None, set_speed_four);
    set_key_binding (KEY_FIVE, None, set_speed_five);
    set_key_binding (KEY_SIX, None, set_speed_six);
    set_key_binding (KEY_SEVEN, None, set_speed_seven);
    set_key_binding (KEY_EIGHT, None, set_speed_eight);
    set_key_binding (KEY_NINE, None, set_speed_nine);

    while (!WindowShouldClose ())
    {
        BeginDrawing ();
        ClearBackground ((Color) {255,255,255,255});
        // double ecs_start_time = GetTime ();
        ecs_progress (world, 0);
        // double ecs_time = GetTime () - ecs_start_time;
        // printf ("%f : %f%%\n", ecs_time, 100 * ecs_time / 0.0166666);
        EndDrawing ();
    }

    deinit_world ();
    CloseWindow ();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
