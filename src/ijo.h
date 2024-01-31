#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <flecs.h>

#include <raylib.h>

extern void *atkinson_regular;
extern uint64_t atkinson_regular_len;

typedef struct Position
{
    float x;
    float y;
} Position;

typedef struct Velocity
{
    float dx;
    float dy;
} Velocity;

ECS_COMPONENT_DECLARE (Position);
ECS_COMPONENT_DECLARE (Velocity);

ECS_SYSTEM_DECLARE (move_system);
ECS_SYSTEM_DECLARE (fps_system);
