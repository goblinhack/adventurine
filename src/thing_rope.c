/*
 * Copyright (C) 2011-2017 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"

thingp level_place_ropetop (levelp level, 
                            thingp owner,
                            double x, double y)
{
    widp w = thing_place(level,
                         owner,
                         id_to_tp(THING_ROPETOP));
    if (!w) {
        ERR("could not place rope");
        return (0);
    }

    thingp t = wid_get_thing(w);

    thing_wake(level, t);

    t->jump_speed = 0.5;

    return (t);
}

thingp level_place_rope (levelp level, 
                         thingp owner,
                         double x, double y)
{
    widp w = thing_place(level,
                         owner,
                         id_to_tp(THING_ROPE));
    if (!w) {
        ERR("could not place rope");
        return (0);
    }

    thingp t = wid_get_thing(w);

    thing_wake(level, t);

    return (t);
}
