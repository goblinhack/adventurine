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
static int thing_hit_ground (levelp level, 
                             thingp t, 
                             double nx,
                             double ny,
                             thingp it)
{
if (thing_can_roll(t)) {
THING_CON(t,"hit ground %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}
    if (t->falling_too_fast) {
        t->falling_too_fast = 0;

        if (thing_is_player(t) ||
            thing_is_monst(t) ||
            thing_is_bomb(t)) {
            (void) thing_hit(level, t, it, 1);
        }
    }

    if (thing_can_roll(t)) {
//        if (t->fall_speed) {
            if (things_handle_impact(level, t, 
                                     nx,
                                     ny,
                                     it)) {
                CON("impact");
                return (true);
            } else {
                return (false);
            }


if (0) {
            if (!t->momentum) {
                if (t->jump_speed) {
//                    t->momentum = 0.05;
//                t->momentum = gauss(0.0, 0.1);
                }
            }
                
//        }
        }
    } else {
        t->fall_speed = 0;
if (thing_can_roll(t)) {
THING_CON(t,"set fall speed %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}
        return (true);
    }
}

/*
 * Try to fall down
 */
int thing_fall (levelp level, thingp t)
{
    double x = t->x;
    double y;
    thingp it;

    if (t->fall_speed == 0) {
        y = t->y + 0.05;
    } else if (t->fall_speed > 0) {
        y = t->y + 0.05;
    } else {
        y = t->y - 0.05;
    }

if (thing_can_roll(t)) {
THING_CON(t,"fall test %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}
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
            thing_hit_ground(level, t, t->x, t->y, it);
            return (false);
        }
    }

    if (t->jump_speed) {
        t->fall_speed = 0;
if (thing_can_roll(t)) {
THING_CON(t,"set fall speed %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}
        t->falling_too_fast = false;
        return (false);
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
        if (!thing_hit_ground(level, t, t->x, t->y, it)) {
            t->fall_speed += 0.005;
        }
    } else {
        t->fall_speed += 0.005;
    }

if (thing_can_roll(t)) {
THING_CON(t,"set fall speed %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}

    if (t->fall_speed > 0.4) {
        t->falling_too_fast = true;
    }

    if (t->fall_speed > 1) {
        t->fall_speed = 1;
if (thing_can_roll(t)) {
THING_CON(t,"set fall speed %s %d %f",__FUNCTION__, __LINE__,t->fall_speed);
}
    }

    y = t->y + t->fall_speed;
if (!thing_can_roll(t)) {
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
                    thing_hit_ground(level, t, 
                                     t->x, 
                                     t->y + t->fall_speed,
                                     it);
                    return (false);
                }
            }
        }
    }
}

    thing_wid_update(level, t, x, y, true, false /* is new */);

    thing_handle_collisions(level, t);

    return (true);
}
