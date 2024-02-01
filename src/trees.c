///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#include "ijo.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

ecs_query_t *tree_query = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct TreeCensus
{
    uint32_t count;
} TreeCensus;

ECS_COMPONENT_DECLARE (TreeCensus);
ECS_TAG_DECLARE (MatureTree);

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

ecs_entity_t mk_tree (float x, float y)
{
    ecs_entity_t e = ecs_new_id (world);
    ecs_set (world, e, Tree, {
        .growth = 0.01,
        .growth_rate = 0.1 + frand () * 0.1,
        .dob = GetTime (),
        .max_age = sim_time + 120 + frand () * 120,
    });
    ecs_set (world, e, Position, {.x = x, .y = y});
    ecs_add_id (world, e, get_geocode (x, y));
    return e;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

bool is_under_a_tree (float x, float y)
{
    float r = 45 * 45;

    ecs_entity_t code = get_geocode (x, y);

    ecs_filter_t *filter = ecs_filter (world, {
        .terms = {
            { .id = ecs_id (Position), .inout = EcsIn },
            { .id = ecs_id (Tree), .inout = EcsIn },
            { .id = code, .inout = EcsIn },
        },
    });

    ecs_iter_t it = ecs_filter_iter (world, filter);

    while (ecs_filter_next (&it))
    {
        const Position *p = ecs_field (&it, Position, 1);

        for (int i = 0; i < it.count; i ++)
        {
            float dx = p[i].x - x;
            float dy = p[i].y - y;

            float d = dx * dx + dy * dy;

            if (d < r)
            {
                ecs_filter_fini (filter);
                return true;
            }
        }
    }
    ecs_filter_fini (filter);
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void reset_census_system (ecs_iter_t *ignored_it)
{
    ecs_singleton_set (world, TreeCensus, {.count = 0});
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void tree_census_system (ecs_iter_t *ignored_it)
{
    uint32_t total_trees = 0;

    ecs_iter_t it = ecs_query_iter (world, tree_query);
    while (ecs_query_next (&it))
    {
        total_trees += it.count;
    }

    ecs_singleton_set (world, TreeCensus, {.count = total_trees});
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void update_trees_system (ecs_iter_t *it)
{
    Tree *t = ecs_field(it, Tree, 1);

    for (int i = 0; i < it->count; i ++) {
        t[i].growth += t[i].growth_rate * sim_delta_time;
        if (t[i].growth > 1)
        {
            t[i].growth = 1;
            ecs_add (world, it->entities[i], MatureTree);
        }
        else if (t[i].growth < 0)
        {
            ecs_delete (world, it->entities[i]);
        }

        float age = sim_time - t[i].dob;
        if (age > t[i].max_age)
        {
            if (t[i].growth_rate > 0)
            {
                t[i].growth_rate = -t[i].growth_rate * 0.1;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void tree_reproduction_system (ecs_iter_t *it)
{
    Tree *t = ecs_field(it, Tree, 1);

    for (int i = 0; i < it->count; i ++) {
        if (t[i].growth == 1)
        {
            float a = frand () * M_PI * 2;
            float dx = sin (a) * frand () * 200;
            float dy = cos (a) * frand () * 200;

            const Position *p = ecs_get (it->world, it->entities[i], Position);

            if
            (
                (p->x + dx < 0) ||
                (p->x + dx > 3 * 1024) ||
                (p->y + dy < 0) ||
                (p->y + dy > 3 * 1024)
            )
            {
            }
            else if (!is_under_a_tree (p->x + dx, p->y + dy))
            {
                mk_tree (p->x + dx, p->y + dy);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void draw_trees_system (ecs_iter_t *it)
{
    const Position *p = ecs_field(it, Position, 1);
    const Tree *t = ecs_field(it, Tree, 2);

    for (int i = 0; i < it->count; i ++) {
        float x = p[i].x;
        float y = p[i].y;
        float g = t[i].growth;
        g = sqrt (g);

        if (t[i].growth_rate > 0)
        {
            DrawCircleV ((Vector2){x, y}, g * 30, (Color){0, 255, 0, 255});
        }
        else
        {
            DrawCircleV ((Vector2){x, y}, g * 30, (Color){192, 255, 128, 255});
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void debug_trees_system (ecs_iter_t *it)
{
    char buffer[100];

    const TreeCensus *tree_census = ecs_singleton_get (world, TreeCensus);

    snprintf (buffer, sizeof (buffer), "%d trees", tree_census->count);
    DrawTextEx (main_font, buffer, (Vector2) {4, 64}, 30, 0, (Color) {0, 0, 0, 255});
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init_trees (void)
{
    ECS_COMPONENT_DEFINE (world, Tree);
    ECS_COMPONENT_DEFINE (world, TreeCensus);
    ECS_TAG_DEFINE (world, MatureTree);

    // ECS_SYSTEM (world, update_trees_system, PhaseUpdate, Tree);
    // ECS_SYSTEM (world, tree_reproduction_system, PhaseUpdate, Tree);
    ECS_SYSTEM (world, reset_census_system, PhaseBeforeUpdate, );
    ECS_SYSTEM (world, tree_census_system, PhaseUpdate, Tree);

    ecs_system (world, {
        .entity = ecs_entity(world, {
            .name = "update_trees_system",
            .add = { ecs_dependson(PhaseUpdate) } // run in OnUpdate phase
        }),
        .query.filter.terms = {
            { .id = ecs_id (Tree) },
        },
        // .interval = 1.0,
        .multi_threaded = false,
        .callback = update_trees_system,
    });

    // ecs_system (world, {
        // .entity = ecs_entity(world, {
            // .name = "tree_reproduction_system",
            // .add = { ecs_dependson(PhaseUpdate) } // run in OnUpdate phase
        // }),
        // .query.filter.terms = {
            // { .id = ecs_id (Tree), .inout = EcsIn },
        // },
        // // .interval = 1.0,
        // .multi_threaded = true,
        // .callback = tree_reproduction_system,
    // });

    // ECS_SYSTEM (world, draw_trees_system, PhaseDraw2D, Position, Tree);
    ECS_SYSTEM (world, debug_trees_system, PhaseDrawDebug, TreeCensus);

    tree_query = ecs_query (world, {
        .filter.terms = {
            { ecs_id (Tree) },
        }
    });

    ecs_singleton_set (world, TreeCensus, {.count = 0});

    for (int i = 0; i < 40000; i ++)
    {
        float a = frand () * M_PI * 2;
        float dx = sin (a) * frand () * 400;
        float dy = cos (a) * frand () * 400;
        mk_tree (600 + dx, 400 + dy);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
