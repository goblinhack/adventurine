/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "wid_game_map.h"

static widp thing_place_ (levelp level,
                          thingp t, 
                          tpp tp, 
                          const int under, 
                          const int behind)
{
    double dx = 0;
    double dy = 0;

    if (t && under) {
        double x;
        double y;

        thing_real_to_fmap(t, &x, &y);

        /*
         * Try to place in front of the player.
         */
        if (!thing_hit_any_obstacle(level, t, x, y)) {
            widp w = wid_game_map_replace_tile(level, x, y,
                                               0, /* thing */
                                               tp,
                                               0 /* tpp_data */);
            return (w);
        }
    }

    thing_dir(t, &dx, &dy);

    /*
     * Sanity check we got one dir.
     */
    if ((dx == 0.0) && (dy == 0.0)) {
        dx = 1.0;
        dy = 1.0;
    }

    if (behind) {
        dx *= -1.0;
        dy *= -1.0;
    }

    dx /= 2.0;
    dy /= 2.0;

    double x;
    double y;
    thing_real_to_fmap(t, &x, &y);
    x += dx;
    y += dy;

    /*
     * Try to place in front of the player.
     */
    widp grid = game.wid_grid;
    if (!grid) {
        ERR("cannot place thing, no grid map");
    }

    if (!thing_hit_any_obstacle(level, t, x, y)) {
        widp w = wid_game_map_replace_tile(level, x, y,
                                           0, /* thing */
                                           tp,
                                           0 /* tpp_data */);
        return (w);
    }

    dx *= 2.0;
    dy *= 2.0;

    thing_real_to_fmap(t, &x, &y);

    x += dx;
    y += dy;

    /*
     * Try to place in front of the player.
     */
    if (!thing_hit_any_obstacle(level, t, x, y)) {
        widp w = wid_game_map_replace_tile(level, x, y,
                                           0, /* thing */
                                           tp,
                                           0 /* tpp_data */);
        return (w);
    }

    /*
     * Just place anywhere free.
     */
    for (dx = -1.0; dx <= 1.0; dx += 1.0) {
        for (dy = -1.0; dy <= 1.0; dy += 1.0) {
            if ((dx == 0.0) && (dy == 0.0)) {
                continue;
            }

            double x;
            double y;
            thing_real_to_fmap(t, &x, &y);
            x += dx;
            y += dy;

            if (x < 0) {
                continue;
            }
            if (y < 0) {
                continue;
            }

            if (x >= MAP_WIDTH) {
                continue;
            }
            if (y >= MAP_HEIGHT) {
                continue;
            }

            if (map_find_wall_at(level, x, y, 0) ||
                map_find_door_at(level, x, y, 0) ||
                map_find_rock_at(level, x, y, 0)) {
                continue;
            }

            if (!thing_hit_any_obstacle(level, t, x, y)) {
                widp w = wid_game_map_replace_tile(level, x, y, 
                                                   0, /* thing */
                                                   tp,
                                                   0 /* tpp_data */);
                return (w);
            }
        }
    }

    /*
     * Ok place on top of something else other than a wall
     */
    for (dx = -1.0; dx <= 1.0; dx += 1.0) {
        for (dy = -1.0; dy <= 1.0; dy += 1.0) {
            if ((dx == 0.0) && (dy == 0.0)) {
                continue;
            }

            double x;
            double y;
            thing_real_to_fmap(t, &x, &y);
            x += dx;
            y += dy;

            if (map_find_wall_at(level, x, y, 0) ||
                map_find_door_at(level, x, y, 0) ||
                map_find_rock_at(level, x, y, 0)) {
                continue;
            }

            widp w = wid_game_map_replace_tile(level, x, y, 
                                               0, /* thing */
                                               tp,
                                               0 /* tpp_data */);
            return (w);
        }
    }

    thing_real_to_fmap(t, &x, &y);

    /*
     * Last resort, just place on the player.
     */
    widp w = wid_game_map_replace_tile(level, x, y, 
                                       0, /* thing */
                                       tp,
                                       0 /* tpp_data */);
    return (w);
}

widp thing_place (levelp level, thingp t, tpp tp)
{
    return (thing_place_(level,
                         t, tp, false /* under */, false /* behind */));
}

widp thing_place_behind (levelp level, thingp t, tpp tp)
{
    return (thing_place_(level,
                         t, tp, false /* under */, true /* behind */));
}

widp thing_place_behind_or_under (levelp level, thingp t, tpp tp)
{
    return (thing_place_(level,
                         t, tp, true /* under */, true /* behind */));
}

