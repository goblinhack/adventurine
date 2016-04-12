/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file.
 */

#pragma once

#include "thing_template.h"
#include "thing.h"
#include "map.h"

typedef struct {
    uint32_t id[MAP_THINGS_PER_CELL];
    uint8_t count;
} thing_map_cell;

typedef struct {
    thing_map_cell cells[MAP_WIDTH][MAP_HEIGHT];
} thing_map_t;

typedef struct {
    int8_t walls[MAP_WIDTH][MAP_HEIGHT];
} level_walls;

typedef struct level_map_tile_ {
    tpp tp;

    /*
     * Data associated with individual tiles.
     */
    thing_template_data data;
} level_map_tile;

typedef struct level_map_grid_ {
    level_map_tile tile[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH];
    uint8_t lit[MAP_WIDTH][MAP_HEIGHT];
} level_map_grid;

typedef struct level_trigger_ {
    color c;
    int activate_exists;
    int activated;
} level_trigger;

enum {
    FLUID_IS_AIR,
    FLUID_IS_SOLID,
    FLUID_IS_LAVA,
    FLUID_IS_ACID,
    FLUID_IS_WATER,
};

enum {
    FLUID_DIR_NONE,
    FLUID_DIR_LEFT,
    FLUID_DIR_RIGHT,
};

typedef uint8_t fluid_mass_t;

typedef struct {
    fluid_mass_t mass;
    uint8_t type:3;
    uint8_t is_surface:3;
} fluid_t;

typedef struct level_t_ {
    uint32_t level_no;
    uint32_t seed;

    /*
     * All things on this level.
     */
    thing things[MAX_THINGS_PER_LEVEL];

    thing_place_context_t timers[MAX_TIMERS_PER_LEVEL];

    /*
     * Cellular automatom for fluid flow.
     */
    fluid_t fluid[FLUID_WIDTH][FLUID_HEIGHT];

    uint32_t next_thing_id;

    /*
     * Quick look op for initial things on the map and for finding what to 
     * spawn on a trigger for ex.
     */
    level_map_grid map_grid;

    thing_map_t thing_map;

    level_walls dmap[DMAP_MAP_MAX];
    level_walls walls;
    level_walls doors;

    /*
     * Possible places for player to start.
     */
    fpoint player_start_position;
    uint8_t player_start_max;
    uint8_t player_start_at;

    /*
     * On screen name for level.
     */
    char title[MAXSTR];

    /*
     * When the level began being played.
     */
    int32_t tick_started;

#define MAX_TRIGGERS 256
    level_trigger trigger[MAX_TRIGGERS];

    /*
     * Level has a shop on it
     */
    uint8_t has_shop;

    /*
     * Being used in level editor.
     */
    uint8_t is_editor:1;
    uint8_t is_map_editor:1;

    /*
     * Walls or doors modified implies AI rethinking
     */
    uint8_t needs_updating:1;

    /*
     * Death...
     */
    uint8_t death_is_coming_soon:1;
    uint8_t death_is_coming:1;

    /*
     * Usually level TEST_LEVEL
     */
    uint8_t is_test_level:1;

    uint8_t is_zzz1:1;
    uint8_t is_zzz2:1;
    uint8_t is_zzz3:1;
    uint8_t is_zzz4:1;
    uint8_t is_zzz5:1;
    uint8_t is_zzz6:1;
    uint8_t is_zzz7:1;
    uint8_t is_zzz8:1;
    uint8_t is_zzz9:1;
    uint8_t is_zzz10:1;
    uint8_t is_zzz11:1;
    uint8_t is_zzz12:1;
    uint8_t is_zzz13:1;

    /*
     * Some walls have changed. Need to redo AI algorithims.
     */
    uint8_t needs_fixup:1;

} level_t;

static inline thing_map_t *level_map (levelp level)
{
    return (&level->thing_map);
}

uint8_t level_init(void);
void level_fini(void);

levelp level_reinit(uint32_t, 
                 int is_editor, 
                 int is_map_editor);
void level_destroy(levelp *, uint8_t keep_player);
levelp level_finished(levelp, int keep_player);
void level_finished_all(void);
int level_tick(levelp);
void level_update_now(levelp);
levelp level_load_new(int level_no);
levelp level_load(uint32_t,
                  int is_editor, 
                  int is_map_editor);
levelp level_load_random(uint32_t level, 
                         int is_editor, 
                         int is_map_editor);
void level_update_slow(levelp);
void marshal_level(marshal_p ctx, levelp);
uint8_t demarshal_level(demarshal_p ctx, levelp l);
const char *level_get_title(levelp);
void level_set_title(levelp, const char *val);
const char *level_get_destroy_reason(levelp);
void level_set_destroy_reason(levelp, const char *val);
widp level_get_map(levelp);
void level_set_map(levelp, widp val);

uint32_t level_get_level_no(levelp);
void level_set_level_no(levelp, uint32_t val);

uint32_t level_get_seed(levelp);
void level_set_seed(levelp, uint32_t val);

const char *level_get_logname(levelp);
uint32_t level_get_tick_started(levelp);
void level_set_tick_started(levelp, uint32_t val);
uint8_t level_needs_updating(levelp);
void level_set_needs_updating(levelp, uint8_t val);
uint8_t level_death_is_coming(levelp);
void level_set_death_is_coming(levelp, uint8_t val);
uint8_t level_death_is_coming_soon(levelp);
void level_set_death_is_coming_soon(levelp, uint8_t val);
uint8_t level_is_zzz1(levelp);
void level_set_is_zzz1(levelp, uint8_t val);
uint8_t level_is_zzz2(levelp);
void level_set_is_zzz2(levelp, uint8_t val);
uint8_t level_is_zzz3(levelp);
void level_set_is_zzz3(levelp, uint8_t val);
uint8_t level_is_zzz4(levelp);
void level_set_is_zzz4(levelp, uint8_t val);
uint8_t level_is_zzz5(levelp);
void level_set_is_zzz5(levelp, uint8_t val);
uint8_t level_is_zzz6(levelp);
void level_set_is_zzz6(levelp, uint8_t val);
uint8_t level_is_zzz7(levelp);
void level_set_is_zzz7(levelp, uint8_t val);
uint8_t level_is_zzz8(levelp);
void level_set_is_zzz8(levelp, uint8_t val);
uint8_t level_is_zzz9(levelp);
void level_set_is_zzz9(levelp, uint8_t val);
uint8_t level_is_zzz10(levelp);
void level_set_is_zzz10(levelp, uint8_t val);
uint8_t level_is_zzz11(levelp);
void level_set_is_zzz11(levelp, uint8_t val);
uint8_t level_is_zzz12(levelp);
void level_set_is_zzz12(levelp, uint8_t val);
uint8_t level_is_zzz13(levelp);
void level_set_is_zzz13(levelp, uint8_t val);
uint8_t level_zzz16(levelp);
void level_set_zzz16(levelp, uint8_t val);
uint8_t level_is_exit_open(levelp);
void level_set_is_exit_open(levelp, uint8_t val);
uint8_t level_is_editor(levelp);
void level_set_is_editor(levelp, uint8_t val);
uint8_t level_is_map_editor(levelp);
void level_set_is_map_editor(levelp, uint8_t val);
void level_set_exit_has_been_reached(levelp, uint8_t val);
uint8_t level_exit_has_been_reached(levelp);
void level_set_needs_fixup(levelp, uint8_t val);
uint8_t level_needs_fixup(levelp);
void level_set_is_ready_to_be_destroyed(levelp, uint8_t val);
uint8_t level_is_ready_to_be_destroyed(levelp);
void level_set_is_ready_to_fade_out(levelp, uint8_t val);
uint8_t level_is_ready_to_fade_out(levelp);

void level_set_has_shop(levelp, uint8_t val);
uint8_t level_has_shop(levelp);

/*
 * level_trigger.c
 */
int level_trigger2slot(levelp, color c);
void level_trigger_activate(levelp, color c);
void level_trigger_alloc(levelp, color c);
int level_trigger_is_activated(levelp, color c);
void level_trigger_activate_default_triggers(levelp);
void level_trigger_move_thing(levelp, tpp me, thingp t);
levelp current_level(void);
