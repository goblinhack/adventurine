/*
 * Copyright (C) 2011-2017 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"
#include "time_util.h"

void thing_torch_update_count (levelp level, thingp t, int force)
{
    if (force || time_have_x_ticks_passed_since(30 * TICKS_PER_SEC, t->tick_torch)) {
        t->tick_torch = game.tick; 

        /*
         * Use up one torch unit.
         */
        if (t->torches) {
            t->torches--;
            if (!t->torches) {
                MSG_SHOUT_AT(INFO, t, 0, 0, "Your light fizzles out");
            }
        }
    }

    /*
     * Work out the torch light radius. Each torch lights 0.5 radius units.
     */
    float torch_light_radius = (double) t->torches / 2.0;

    if (torch_light_radius > tp_get_light_radius(thing_tp(t))) {
        torch_light_radius = tp_get_light_radius(thing_tp(t));
    }

    if (torch_light_radius != t->torch_light_radius) {
        t->torch_light_radius = torch_light_radius;
        t->torch_light_radius_set = true;
    }
}
