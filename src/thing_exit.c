/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "level.h"
#include "sound.h"
#include "wid_game_map.h"

void thing_reached_exit (levelp level, thingp t, thingp exit)
{
    thing_leave_level(level, t);

    if (exit->data.exit_set) {
        game.level_no = exit->data.exit;

        THING_LOG(t, "Exit jump to level %d", game.level_no);
    } else {
        game.level_no++;

        THING_LOG(t, "Jump to level %d", game.level_no);
    }

    verify(t);

    t->score += 1000 * level->level_no;

    int next_level = level->level_no + 1;

    game.level_no = next_level;

    level = level_load_new(next_level);

        wid_game_map_replace_tile(level,
                                  0, 0, 
                                  t, /* thing */
                                  thing_tp(t),
                                  0 /* tpp_data */);

    thing_join_level(level, t);

    sound_play_level_end();

    /*
     * To allow the player to be centered in the new level if it is a
     * different size.
     */
    if (game.wid_grid &&
        game.wid_grid->grid) {
        game.wid_grid->grid->bounds_locked = 0;
    }

    thing_move(level, t, t->x, t->y, false, false, false, false, false);

    /*
     * Need one tick for dmap light source generation.
     */
    level_tick(level);

    /*
     * Don't want to call tick all things, just tick the players torch.
     */
    thing_torch_update_count(level, t, false /* force */);

    wid_visible(game.wid_grid, wid_fade_delay);
}
