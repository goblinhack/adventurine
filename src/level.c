/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>
#include "slre.h"

#include "main.h"
#include "string_util.h"
#include "wid.h"
#include "level.h"
#include "thing.h"
#include "command.h"
#include "time_util.h"
#include "marshal.h"
#include "wid_game_map.h"
#include "wid_map.h"
#include "map.h"
#include "timer.h"
#include "sound.h"
#include "wid_editor.h"
#include "map_jigsaw.h"
#include "file.h"
#include "map.h"
#include "thing_shop.h"
#include "fluid.h"

static uint8_t level_init_done;
static uint8_t level_init_done;
static void level_reset_player(levelp level);
static void level_set_walls(levelp level);
static void level_update_incremental(levelp level, int force);

uint8_t level_init (void)
{
    if (level_init_done) {
        return (true);
    }

    level_init_done = true;

    return (true);
}

static uint8_t level_side_one_time_init (void)
{
    if (level_init_done) {
        return (true);
    }

    level_init_done = true;

    return (true);
}

void level_fini (void)
{
    if (level_init_done) {
        level_init_done = false;
    }
}

levelp level_reinit (uint32_t level_no, 
                     int is_editor, 
                     int is_map_editor)
{
    levelp level;

    level = &game.level;
    newptr(level, "level");
    memset(level, 0, sizeof(*level));

    if (!is_editor && !is_map_editor) {
        wid_game_map_grid_create(level);
    }

    if (level_no == TEST_LEVEL) {
        level->is_test_level = true;
    }

    level_set_level_no(level, level_no);
    level_set_is_editor(level, is_editor);
    level_set_is_map_editor(level, is_map_editor);

    LEVEL_LOG(level, "Reset fresh level %d", level_no);

    return (level);
}

void level_destroy (levelp *plevel, uint8_t keep_player)
{
    levelp level;

    if (!plevel) {
        ERR("no plevel");
    }

    level = *plevel;
    if (!level) {
        ERR("no level");
    }

    if (keep_player) {
        LEVEL_LOG(level, "Destroy level but keep player");
    } else {
        LEVEL_LOG(level, "Destroy level and kill player too");
    }

    /*
     * Kill all humans!
     */
    things_level_destroyed(level, keep_player);

    level->title[0] = 0;

    *plevel = 0;

    oldptr(level);

    level = 0;
}

void level_update_slow (levelp level)
{
    map_fixup(level);

    shop_fixup(level);

    level_set_walls(level);

    /*
     * One time generate of expensive wander map
     */
    dmap_generate_map_wander(level);

    level_update_incremental(level, true /* force */);
}

static void level_update_incremental (levelp level, int force)
{
    level_set_walls(level);

    /*
     * Regenerate player dmaps as things like doors may have been opened.
     */
    dmap_generate(level, force);
}

levelp level_load_new (int level_no)
{
    levelp level;
    level = &game.level;

    LOG("New level, %d", level_no);

    char *tmp = dynprintf("%s%d", LEVELS_PATH, level_no);

    /*
     * Mostly random levels.
     */
    int fixed = false;

    if (game.level_no == TEST_LEVEL) {
        fixed = true;
    }

    int r = (myrand() % 100);
    if ((r < 20) && file_exists(tmp)) {
        fixed = true;
    }

    if (fixed) {
        level = level_load(game.level_no,
                           false /* is_editor */,
                           false /* is_map_editor */);
    } else {
        LOG("Level %s does not exist, create random level", tmp);

        level = level_load_random(game.level_no,
                                  false /* is_editor */,
                                  false /* is_map_editor */);
    }

    myfree(tmp);

    if (!level) {
        ERR("failed to load level %d", game.level_no);
    }

    level_update_slow(level);

    return (level);
}

/*
 * Routines done for both random and static levels.
 */
static void level_loaded_common (levelp level)
{
    /*
     * Activate any triggers that have no activators.
     */
    if (!level->is_editor && !level->is_map_editor) {
        level_trigger_activate_default_triggers(level);
    }

    fluid_init(level);
}

levelp level_load (uint32_t level_no, 
                   int is_editor,
                   int is_map_editor)
{
    levelp level;

    level_side_one_time_init();

    level = level_reinit(level_no, is_editor, is_map_editor);

    level_set_tick_started(level, time_get_time_ms());

    char *dir_and_file;

    dir_and_file = dynprintf("%s%d", LEVELS_PATH, level_no);

    if (!file_exists(dir_and_file)) {
        myfree(dir_and_file);

        dir_and_file = dynprintf("adventurine-data/levels/%d", level_no);
    }

    LEVEL_LOG(level, "Level %s: loading", dir_and_file);

    int pass;
    int max_pass;

    if (level_is_map_editor(level) || level_is_editor(level)) {
        max_pass = 1;
    } else {
        /*
         * Need 2 passes for levels being read into the game. First pass is 
         * just to learn where the action triggers are for spawned things and 
         * the like.
         *
         * Second pass creates the things.
         */
        max_pass = 2;
    }

    for (pass = 0; pass < max_pass; pass++) {
        demarshal_p in;

        game.level_is_being_loaded = pass + 1;

        if (!(in = demarshal(dir_and_file))) {
            /*
             * Fail
             *
             * I'm assuming this means the end of the game
             *
            char *popup_str = dynprintf("Failed to load level %s: %s",
                                        dir_and_file,
                                        strerror(errno));

            MSG_BOX("%s", popup_str);
            myfree(popup_str);
             */
            myfree(dir_and_file);

            return (0);
        } else {
            /*
             * Success
             */
            if (!demarshal_level(in, level)) {
                char *popup_str = dynprintf("There were some errors "
                                            "while loading level %s: %s",
                                            dir_and_file,
                                            strerror(errno));

                MSG_BOX("%s", popup_str);
                myfree(popup_str);
            }

            demarshal_fini(in);
        }
    }

    game.level_is_being_loaded = 0;

    myfree(dir_and_file);

    if (!level_is_map_editor(level) &&
        !level_is_editor(level)) {
        level_update_slow(level);
    }

    if (!level_is_map_editor(level) &&
        !level_is_editor(level)) {
        level_reset_player(level);
    }

    level_loaded_common(level);

    return (level);
}

levelp level_load_random (uint32_t level_no, 
                          int is_editor,
                          int is_map_editor)
{
    levelp level;

    level_side_one_time_init();

    level = level_reinit(level_no, 
                         is_editor, 
                         is_map_editor);

    level_set_tick_started(level, time_get_time_ms());

    LEVEL_LOG(level, "Level generating");

    map_jigsaw_generate(level, 0, wid_game_map_replace_tile);

    level_update_slow(level);

    level_reset_player(level);

    level_loaded_common(level);

    return (level);
}

const char *level_get_title (levelp level)
{

    return (level->title);
}

void level_set_title (levelp level, const char *val)
{

    strncpy(level->title, val, sizeof(level->title));
}

widp level_get_map (levelp level)
{

    return (game.wid_grid);
}

static void level_set_walls (levelp level)
{
    int32_t x;
    int32_t y;
    int32_t z;

    widp w = game.wid_grid;
    if (!w) {
        return;
    }

    if (!level) {
        return;
    }


    int i;
    for (i = 0; i < DMAP_MAP_MAX; i++) {
        memset(level->dmap[i].walls, ' ',
            sizeof(level->dmap[i].walls));
        memset(level->dmap[i].walls, ' ',
            sizeof(level->dmap[i].walls));
    }

    memset(level->walls.walls, ' ',
           sizeof(level->walls.walls));
    memset(level->doors.walls, ' ',
           sizeof(level->doors.walls));

    for (z = MAP_DEPTH_WALL; z < MAP_DEPTH; z++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            for (y = 0; y < MAP_HEIGHT; y++) {
                tree_root **tree = 
                    w->grid->grid_of_trees[z] + (y * w->grid->width) + x;
                widgridnode *node;

                TREE_WALK_REVERSE_UNSAFE_INLINE(
                                    *tree, 
                                    node,
                                    tree_prev_tree_wid_compare_func) {

                    widp w = node->wid;

                    thingp t = wid_get_thing(w);
                    if (!t) {
                        continue;
                    }

                    /*
                     * Identify obstacles.
                     */
                    if (thing_is_wall(t)        ||
                        thing_is_rock(t)        ||
                        thing_is_door(t)        ||
                        thing_is_mob_spawner(t) ||
                        thing_is_teleport(t)    ||
                        thing_is_lava(t)        ||
                        thing_is_water(t)       ||
                        thing_is_acid(t)        ||
                        thing_is_exit(t)) {
                        level->dmap[DMAP_MAP_PLAYER_TARGET_TREAT_DOORS_AS_WALLS].walls[x][y] = '+';
                    }

                    /*
                     * Same as above, but treat doors as passable.
                     */
                    if (thing_is_wall(t)        ||
                        thing_is_rock(t)        ||
                        thing_is_mob_spawner(t) ||
                        thing_is_teleport(t)    ||
                        thing_is_lava(t)        ||
                        thing_is_water(t)       ||
                        thing_is_acid(t)        ||
                        thing_is_exit(t)) {
                        level->dmap[DMAP_MAP_PLAYER_TARGET_TREAT_DOORS_AS_PASSABLE].walls[x][y] = '+';
                    }

                    if (thing_is_wall(t) ||
                        thing_is_rock(t) ||
                        thing_is_door(t)) {
                        level->doors.walls[x][y] = '+';
                    }
                }
            }
        }
    }
}

/*
 * New level; update player.
 */
void level_reset_player (levelp level)
{
    thingp t;

    /*
     * Rewield weapons at start.
     */
    LEVEL_LOG(level, "Reset player at start of level");

    FOR_ALL_THINGS(level, t) {
        if (thing_is_player(t)) {
            tpp weapon = thing_weapon(t);
            if (weapon) {
                thing_wield(level, t, weapon);
            }

            /*
             * Need this else we are in darkness at level start
             */
            thing_torch_update_count(level, t, true);
        }
    }
    FOR_ALL_THINGS_END
}

/*
 * Clean up the level. It's over! the exit was reached and a delay passed to 
 * warn the other player.
 */
levelp level_finished (levelp level, int keep_player)
{

    /*
     * Is this needed ? Kind of obvious and overlaps with spending points 
     * update often.
     */
    if (level->is_test_level) {
        LEVEL_LOG(level, "Test level finished");
        level_destroy(&level, false /* keep player */);
        return (0);
    }

    thingp t;

    /*
     * Force the death of all things on the level.
     */
    FOR_ALL_THINGS(level, t) {
        if (!thing_is_player(t)) {
            thing_leave_level(level, t);
            thing_set_is_dead(t, true);
        }
    }
    FOR_ALL_THINGS_END

    if (keep_player) {
        LOG("Destroy level but keep player");
    } else {
        LOG("Destroy level and kill player");
    }

    wid_detach_from_grid(game.wid_grid);
    wid_destroy_grid(game.wid_grid);
    wid_destroy(&game.wid_grid);

    int next_level = level->level_no + 1;

    level_destroy(&level, keep_player);

    /*
     * LEVEL CAN BE NULL HERE
     */

    if (game.game_over) {
        return (0);
    }

    if (!keep_player) {
        return (0);
    }

    /*
     * Create the new level, random or otherwise.
     */
    level = level_load_new(next_level);

    /*
     * Move player to the new level.
     */
    FOR_ALL_THINGS(level, t) {
        if (!thing_is_player(t)) {
            continue;
        }

        thing_map_remove(level, t);

        wid_game_map_replace_tile(
                level,
                0, 0,
                t,
                thing_tp(t),
                0 /* tpp_data */);

        thing_join_level(level, t);
    }
    FOR_ALL_THINGS_END

    level_update_slow(level);

    return (level);
}

void level_finished_all (void)
{
    LOG("Destroy level");

    levelp level = &game.level;
    level_finished(level, false /* keep_player */);
}

static void level_map_fixup (levelp level)
{
    if (!level) {
        return;
    }

    /*
     * Allow the first fixup to happen immediately and the rest deferred.
     */
    if (!level_needs_fixup(level)) {
        return;
    }

    map_fixup(level);

    level_set_needs_fixup(level, false);
}

int level_tick (levelp level)
{
    if (!level) {
        return (false);
    }


    /*
     * Every now and again cause the ghosts to look at the level afresh so 
     * that if walls move then they now look through the gaps.
     */
    if (level_needs_updating(level)) {
        level_set_needs_updating(level, false);

        /*
         * For things like walls removed
         */
        level_update_incremental(level, true);
    } else {
        /*
         * Can avoid doing player dmap calc if player not moved
         */
        level_update_incremental(level, false);
    }

    level_map_fixup(level);

    return (true);
}

uint32_t level_get_level_no (levelp level)
{

    return (level->level_no);
}

void level_set_level_no (levelp level, uint32_t val)
{

    level->level_no = val;
}

uint32_t level_get_seed (levelp level)
{

    return (level->seed);
}

void level_set_seed (levelp level, uint32_t val)
{

    level->seed = val;
}

uint32_t level_get_tick_started (levelp level)
{

    return (level->tick_started);
}

void level_set_tick_started (levelp level, uint32_t val)
{

    level->tick_started = val;
}

uint8_t level_needs_updating (levelp level)
{

    return (level->needs_updating);
}

void level_set_needs_updating (levelp level, uint8_t val)
{

    level->needs_updating = val;
}

uint8_t level_death_is_coming (levelp level)
{

    return (level->death_is_coming);
}

void level_set_death_is_coming (levelp level, uint8_t val)
{

    level->death_is_coming = val;
}

uint8_t level_death_is_coming_soon (levelp level)
{

    return (level->death_is_coming_soon);
}

void level_set_death_is_coming_soon (levelp level, uint8_t val)
{

    level->death_is_coming_soon = val;
}

uint8_t level_is_zzz1 (levelp level)
{

    return (level->is_zzz1);
}

void level_set_is_zzz1 (levelp level, uint8_t val)
{

    level->is_zzz1 = val;
}

uint8_t level_is_zzz2 (levelp level)
{

    return (level->is_zzz2);
}

void level_set_is_zzz2 (levelp level, uint8_t val)
{

    level->is_zzz2 = val;
}

uint8_t level_is_zzz3 (levelp level)
{

    return (level->is_zzz3);
}

void level_set_is_zzz3 (levelp level, uint8_t val)
{

    level->is_zzz3 = val;
}

uint8_t level_is_zzz4 (levelp level)
{

    return (level->is_zzz4);
}

void level_set_is_zzz4 (levelp level, uint8_t val)
{

    level->is_zzz4 = val;
}

uint8_t level_is_zzz5 (levelp level)
{

    return (level->is_zzz5);
}

void level_set_is_zzz5 (levelp level, uint8_t val)
{

    level->is_zzz5 = val;
}

uint8_t level_is_zzz6 (levelp level)
{

    return (level->is_zzz6);
}

void level_set_is_zzz6 (levelp level, uint8_t val)
{

    level->is_zzz6 = val;
}

uint8_t level_is_zzz7 (levelp level)
{

    return (level->is_zzz7);
}

void level_set_is_zzz7 (levelp level, uint8_t val)
{

    level->is_zzz7 = val;
}

uint8_t level_is_zzz8 (levelp level)
{

    return (level->is_zzz8);
}

void level_set_is_zzz8 (levelp level, uint8_t val)
{

    level->is_zzz8 = val;
}

uint8_t level_is_zzz9 (levelp level)
{

    return (level->is_zzz9);
}

void level_set_is_zzz9 (levelp level, uint8_t val)
{

    level->is_zzz9 = val;
}

uint8_t level_is_zzz10 (levelp level)
{

    return (level->is_zzz10);
}

void level_set_is_zzz10 (levelp level, uint8_t val)
{

    level->is_zzz10 = val;
}

uint8_t level_is_zzz11 (levelp level)
{

    return (level->is_zzz11);
}

void level_set_is_zzz11 (levelp level, uint8_t val)
{

    level->is_zzz11 = val;
}

uint8_t level_is_zzz12 (levelp level)
{

    return (level->is_zzz12);
}

void level_set_is_zzz12 (levelp level, uint8_t val)
{

    level->is_zzz12 = val;
}

uint8_t level_is_zzz13 (levelp level)
{

    return (level->is_zzz13);
}

void level_set_is_zzz13 (levelp level, uint8_t val)
{

    level->is_zzz13 = val;
}

uint8_t level_is_editor (levelp level)
{

    return (level->is_editor);
}

void level_set_is_editor (levelp level, uint8_t val)
{

    level->is_editor = val;
}

uint8_t level_is_map_editor (levelp level)
{

    return (level->is_map_editor);
}

void level_set_is_map_editor (levelp level, uint8_t val)
{

    level->is_map_editor = val;
}

uint8_t level_has_shop (levelp level)
{

    return (level->has_shop > 0);
}

void level_set_has_shop (levelp level, uint8_t val)
{

    if (val) {
        level->has_shop++;
    } else {
        if (level->has_shop) {
            level->has_shop--;
        }
    }
}

uint8_t level_needs_fixup (levelp level)
{

    return (level->needs_fixup ? 1 : 0);
}

void level_set_needs_fixup (levelp level, uint8_t val)
{

    level->needs_fixup = val;
}

static uint32_t level_count_is_x (levelp level, map_is_at_callback callback)
{
    uint32_t count;
    int32_t x;
    int32_t y;

    count = 0;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            count += map_count_x_at(level, x, y, callback);
        }
    }

    return (count);
}

uint32_t level_count_is_player (levelp level)
{
    return (level_count_is_x(level, tp_is_player));
}

uint32_t level_count_is_monst (levelp level)
{
    return (level_count_is_x(level, tp_is_monst));
}

uint32_t level_count_is_wall (levelp level)
{
    return (level_count_is_x(level, tp_is_wall));
}

uint32_t level_count_is_key (levelp level)
{
    return (level_count_is_x(level, tp_is_key));
}

uint32_t level_count_is_rock (levelp level)
{
    return (level_count_is_x(level, tp_is_rock));
}

uint32_t level_count_is_shadow_caster (levelp level)
{
    return (level_count_is_x(level, tp_is_shadow_caster));
}

uint32_t level_count_is_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon));
}

uint32_t level_count_is_treasure (levelp level)
{
    return (level_count_is_x(level, tp_is_treasure));
}

uint32_t level_count_is_fragile (levelp level)
{
    return (level_count_is_x(level, tp_is_fragile));
}

uint32_t level_count_is_animated_no_dir (levelp level)
{
    return (level_count_is_x(level, tp_is_animated_no_dir));
}

uint32_t level_count_is_weapon_swing_effect (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon_swing_effect));
}

uint32_t level_count_is_light_source (levelp level)
{
    return (level_count_is_x(level, tp_is_light_source));
}

uint32_t level_count_is_candle_light (levelp level)
{
    return (level_count_is_x(level, tp_is_candle_light));
}

uint32_t level_count_is_cats_eyes (levelp level)
{
    return (level_count_is_x(level, tp_is_cats_eyes));
}

uint32_t level_count_is_fire (levelp level)
{
    return (level_count_is_x(level, tp_is_fire));
}

uint32_t level_count_is_animation (levelp level)
{
    return (level_count_is_x(level, tp_is_animation));
}

uint32_t level_count_is_non_explosive_gas_cloud (levelp level)
{
    return (level_count_is_x(level, tp_is_non_explosive_gas_cloud));
}

uint32_t level_count_is_item_unusable (levelp level)
{
    return (level_count_is_x(level, tp_is_item_unusable));
}

uint32_t level_count_is_door (levelp level)
{
    return (level_count_is_x(level, tp_is_door));
}

uint32_t level_count_is_mob_spawner (levelp level)
{
    return (level_count_is_x(level, tp_is_mob_spawner));
}

uint32_t level_count_is_acid (levelp level)
{
    return (level_count_is_x(level, tp_is_acid));
}

uint32_t level_count_is_lava (levelp level)
{
    return (level_count_is_x(level, tp_is_lava));
}

uint32_t level_count_is_teleport (levelp level)
{
    return (level_count_is_x(level, tp_is_teleport));
}

uint32_t level_count_is_cobweb (levelp level)
{
    return (level_count_is_x(level, tp_is_cobweb));
}

uint32_t level_count_is_ethereal (levelp level)
{
    return (level_count_is_x(level, tp_is_ethereal));
}

uint32_t level_count_is_variable_size (levelp level)
{
    return (level_count_is_x(level, tp_is_variable_size));
}

uint32_t level_count_is_magical_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_magical_weapon));
}

uint32_t level_count_is_ranged_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_ranged_weapon));
}

uint32_t level_count_is_melee_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_melee_weapon));
}

uint32_t level_count_is_cloud_effect (levelp level)
{
    return (level_count_is_x(level, tp_is_cloud_effect));
}

uint32_t level_count_is_hard (levelp level)
{
    return (level_count_is_x(level, tp_is_hard));
}

uint32_t level_count_is_sleeping (levelp level)
{
    return (level_count_is_x(level, tp_is_sleeping));
}

uint32_t level_count_is_bomb (levelp level)
{
    return (level_count_is_x(level, tp_is_bomb));
}

uint32_t level_count_is_sawblade (levelp level)
{
    return (level_count_is_x(level, tp_is_sawblade));
}

uint32_t level_count_is_visible_on_debug_only (levelp level)
{
    return (level_count_is_x(level, tp_is_visible_on_debug_only));
}

uint32_t level_count_is_action (levelp level)
{
    return (level_count_is_x(level, tp_is_action));
}

uint32_t level_count_can_walk_through (levelp level)
{
    return (level_count_is_x(level, tp_can_walk_through));
}

uint32_t level_count_is_weapon_carry_anim (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon_carry_anim));
}

uint32_t level_count_is_animated_only_when_moving (levelp level)
{
    return (level_count_is_x(level, tp_is_animated_only_when_moving));
}

uint32_t level_count_is_warm_blooded (levelp level)
{
    return (level_count_is_x(level, tp_is_warm_blooded));
}

uint32_t level_count_can_be_enchanted (levelp level)
{
    return (level_count_is_x(level, tp_can_be_enchanted));
}

uint32_t level_count_is_stackable (levelp level)
{
    return (level_count_is_x(level, tp_is_stackable));
}

uint32_t level_count_is_torch (levelp level)
{
    return (level_count_is_x(level, tp_is_torch));
}

uint32_t level_count_is_explosion (levelp level)
{
    return (level_count_is_x(level, tp_is_explosion));
}

uint32_t level_count_is_hidden_from_editor (levelp level)
{
    return (level_count_is_x(level, tp_is_hidden_from_editor));
}

uint32_t level_count_is_combustable (levelp level)
{
    return (level_count_is_x(level, tp_is_combustable));
}

uint32_t level_count_is_projectile (levelp level)
{
    return (level_count_is_x(level, tp_is_projectile));
}

uint32_t level_count_is_inactive (levelp level)
{
    return (level_count_is_x(level, tp_is_inactive));
}

uint32_t level_count_is_food (levelp level)
{
    return (level_count_is_x(level, tp_is_food));
}

uint32_t level_count_is_dungeon_floor (levelp level)
{
    return (level_count_is_x(level, tp_is_dungeon_floor));
}

uint32_t level_count_is_exit (levelp level)
{
    return (level_count_is_x(level, tp_is_exit));
}

static thingp this_door[MAP_WIDTH][MAP_HEIGHT];

static void door_flood (levelp level, int32_t x, int32_t y)
{
    if (this_door[x][y]) {
        return;
    }

    if (!(this_door[x][y] = map_thing_is_door_at(level, x, y))) {
        this_door[x][y] = (thingp) (void*)-1;
        return;
    }

    door_flood(level, x-1, y);
    door_flood(level, x+1, y);
    door_flood(level, x, y-1);
    door_flood(level, x, y+1);
}

void level_open_door (levelp level, int32_t ix, int32_t iy)
{
    int32_t x;
    int32_t y;

    memset(this_door, 0, sizeof(this_door));

    door_flood(level, ix, iy);

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (!this_door[x][y]) {
                continue;
            }

            if (this_door[x][y] == (void*)-1) {
                continue;
            }

            thing_dead(level, this_door[x][y], 0 /* killer */, "open");
        }
    }

    level_update_incremental(level, true /* force */);

    /*
     * Removal of doors would have set this flag.
     */
    level_set_needs_updating(level, false);

    MESG(SOUND, "door");
}

void marshal_level (marshal_p ctx, levelp level)
{
    if (!level) {
        return;
    }

    PUT_BRA(ctx);

    if (!level->title[0]) {
        PUT_NAMED_STRING(ctx, "title", "unnamed level");
    } else {
        PUT_NAMED_STRING(ctx, "title", level->title);
    }

    PUT_NAMED_BITFIELD(ctx, "is_zzz1", level->is_zzz1);
    PUT_NAMED_BITFIELD(ctx, "is_zzz2", level->is_zzz2);
    PUT_NAMED_BITFIELD(ctx, "is_zzz3", level->is_zzz3);
    PUT_NAMED_BITFIELD(ctx, "is_zzz4", level->is_zzz4);
    PUT_NAMED_BITFIELD(ctx, "is_zzz5", level->is_zzz5);
    PUT_NAMED_BITFIELD(ctx, "is_zzz6", level->is_zzz6);
    PUT_NAMED_BITFIELD(ctx, "is_zzz7", level->is_zzz7);
    PUT_NAMED_BITFIELD(ctx, "is_zzz8", level->is_zzz8);
    PUT_NAMED_BITFIELD(ctx, "is_zzz9", level->is_zzz9);
    PUT_NAMED_BITFIELD(ctx, "is_zzz10", level->is_zzz10);
    PUT_NAMED_BITFIELD(ctx, "is_zzz11", level->is_zzz11);
    PUT_NAMED_BITFIELD(ctx, "is_zzz12", level->is_zzz12);
    PUT_NAMED_BITFIELD(ctx, "is_zzz13", level->is_zzz13);
}

uint8_t demarshal_level (demarshal_p ctx, levelp level)
{
    uint8_t rc;
    widp wid;

    if (!level) {
        return (false);
    }

    GET_BRA(ctx);

    char *tmp = 0;
    GET_OPT_NAMED_STRING(ctx, "title", tmp);
    if (!tmp) {
        ERR("no level title for level");
    } else {
        level_set_title(level, tmp);
        myfree(tmp);
    }

    wid = level_get_map(level);
    if (!wid) {
        ERR("no map for level");
    }

    do {
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz1", level->is_zzz1);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz2", level->is_zzz2);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz3", level->is_zzz3);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz4", level->is_zzz4);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz5", level->is_zzz5);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz6", level->is_zzz6);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz7", level->is_zzz7);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz8", level->is_zzz8);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz9", level->is_zzz9);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz10", level->is_zzz10);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz11", level->is_zzz11);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz12", level->is_zzz12);
        GET_OPT_NAMED_BITFIELD(ctx, "is_zzz13", level->is_zzz13);
    } while (demarshal_gotone(ctx));

    if (level_is_map_editor(level)) {
        rc = demarshal_wid_grid(level,
                                ctx, wid,
                                wid_editor_level_map_thing_replace_template);
    } else if (level_is_editor(level)) {
        rc = demarshal_wid_grid(level,
                                ctx, wid,
                                wid_editor_replace_template);
    } else {
        rc = demarshal_wid_grid(level,
                                ctx, wid,
                                wid_game_map_replace_tile);
    }

    if (level_is_map_editor(level)) {
        /*
         * No widget to update
         */
    } else if (level_is_editor(level)) {
        /*
         * No widget to update
         */
    } else {
        map_fixup(level);
        wid_update(game.wid_grid);
    }

    GET_KET(ctx);

    return (rc);
}
