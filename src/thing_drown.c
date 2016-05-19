/*
 * Copyright (C) 2011-2017 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"
#include "wid_game_map.h"
#include "tile.h"
#include "math_util.h"
#include "time_util.h"
#include "player.h"

/*
 * Try to fall down
 */
int thing_drown (levelp level, thingp t)
{
    if (thing_is_submerged(level, t)) {

        if (!t->timestamp_last_breath) {
            t->timestamp_last_breath = time_get_time_ms();
        }

        if (t->timestamp_last_breath) {
            if (time_have_x_tenths_passed_since(10, t->timestamp_last_breath)) {

                if ((myrand() % 100) < 10) {
                    if (thing_submerged_depth(level, t) > 10) {
                        level_place_bubbles(level, t, t->x, t->y - 0.5);
                    }
                }
            }

            if (time_have_x_tenths_passed_since(50, t->timestamp_last_breath)) {

                if (t->breath > 8) {
                    level_place_blood_crit(level, t, t->x, t->y);
                } if (t->breath > 7) {
                    level_place_blood_crit(level, t, t->x, t->y);
                } else {
                    if (thing_submerged_depth(level, t) > 10) {
                        level_place_bubbles(level, t, t->x, t->y - 0.5);
                    }
                }

                t->timestamp_last_breath = time_get_time_ms();

                t->breath++;
                if (player == t) {
                    player_wid_update(level);
                }
            }

            if (t->breath >= ARRAY_SIZE(game.wid_drown_icon)) {
                thing_dead(level, t, 0, "drowned");
                return (false);
            }
        }

        t->is_submerged = true;
        t->is_partially_submerged = true;

    } else if (thing_is_partially_or_fully_submerged(level, t)) {
        t->is_partially_submerged = true;
        t->timestamp_last_breath = 0;
        t->breath = 0;

    } else {
        t->timestamp_last_breath = 0;
        t->breath = 0;

        if (t->is_submerged) {
            t->jump_speed = 0.1;

        } else if (t->is_partially_submerged) {
            t->jump_speed = 0.05;
        }

        t->is_submerged = false;
        t->is_partially_submerged = false;
    }

    if (t->is_submerged) {
        t->fall_speed /= 4.0;
if (thing_can_roll(t)) {
THING_CON(t,"set fall speed %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}
        t->jump_speed /= 4.0;

    } else if (t->is_partially_submerged) {
        t->fall_speed /= 2.0;
if (thing_can_roll(t)){
THING_CON(t,"set fall speed %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}
        t->jump_speed /= 2.0;
    }

    return (true);
}
