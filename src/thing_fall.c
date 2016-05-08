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
 * Hit the ground hard?
 */
static void thing_hit_ground (levelp level, thingp t, thingp it)
{
    if (t->falling_too_fast) {
        t->falling_too_fast = 0;
        (void) thing_hit(level, t, it, 1);
    }

    t->fall_speed = 0;
}

/*
 * Try to fall down
 */
int thing_fall (levelp level, thingp t)
{
    double x = t->x;
    double y = t->y + 0.015;
    thingp it;

    if (thing_is_rope(t)) {
        if (!thing_hit_fall_obstacle(level, t,  t->x, (int) t->y + 1.0)) {
            thing_place_rope(level, t, t->x, (int) t->y + 1.0);
            return (false);
        }
    }

    if (thing_is_monst(t)  ||
        thing_is_ladder(t) ||
        thing_is_player(t)) {

        it = thing_overlaps(level, t, t->x, t->y, thing_is_ladder);
        if (it) {
            thing_hit_ground(level, t, it);
            return (false);
        }
    }

    if (t->jump_speed) {
        t->fall_speed = 0;
        return (false);
    }

    if (thing_is_player(t) || 
        thing_is_monst(t)) {

        if (thing_is_submerged(level, t)) {

            if (!t->timestamp_last_breath) {
                t->timestamp_last_breath = time_get_time_ms();
            }

            if (t->timestamp_last_breath) {
                if (time_have_x_tenths_passed_since(10, t->timestamp_last_breath)) {
                    t->timestamp_last_breath = time_get_time_ms();

                    t->breath++;
                    if (player == t) {
                        player_wid_update(level);
                    }
                }

                if (t->breath >= 10) {
                    thing_dead(level, t, 0, "drowned");
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
            t->jump_speed /= 4.0;

        } else if (t->is_partially_submerged) {
            t->fall_speed /= 2.0;
            t->jump_speed /= 2.0;
        }
    }

    if (thing_is_monst(t)  ||
        thing_is_player(t)) {

        if (t->fall_speed > 0.1) {
            it = thing_overlaps(level, t, t->x, t->y, thing_is_spikes);
            if (it) {
                (void) thing_hit(level, t, it, 1);
            }
        }
    }

    it = thing_hit_fall_obstacle(level, t, x, y);
    if (it) {
        thing_hit_ground(level, t, it);
        return (false);
    }

    t->fall_speed += 0.010;

    if (t->fall_speed > 0.4) {
        t->falling_too_fast = true;
    }

    if (t->fall_speed > 1) {
        t->fall_speed = 1;
    }

    y = t->y + t->fall_speed;
    it = thing_hit_fall_obstacle(level, t, x, y);
    if (it) {
        t->fall_speed /= 2;

        y = t->y + t->fall_speed;
        it = thing_hit_fall_obstacle(level, t, x, y);
        if (it) {
            t->fall_speed /= 2;

            y = t->y + t->fall_speed;
            it = thing_hit_fall_obstacle(level, t, x, y);
            if (it) {
                t->fall_speed /= 2;

                y = t->y + t->fall_speed;
                it = thing_hit_fall_obstacle(level, t, x, y);
                if (it) {
                    thing_hit_ground(level, t, it);
                    return (false);
                }
            }
        }
    }

    thing_wid_update(level, t, x, y, true, false /* is new */);

    thing_handle_collisions(level, t);

    return (true);
}
