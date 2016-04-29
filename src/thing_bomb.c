/*
 * Copyright (C) 2011-2017 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"

thingp level_place_bomb (levelp level, 
                         thingp owner,
                         double x, double y)
{
    if (!owner->bombs) {
        return (0);
    }
    owner->bombs--;

    widp w = thing_place(level,
                         owner,
                         id_to_tp(THING_BOMB));
    if (!w) {
        ERR("could not place bomb");
        return (0);
    }

    thingp t = wid_get_thing(w);

    /*
     * Set no owner. Bombs should kill their owner too!
     *
    thing_set_owner(t, owner);
     */

    thing_wake(level, t);

    return (t);
}
