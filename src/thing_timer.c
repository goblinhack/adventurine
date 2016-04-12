/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "timer.h"
#include "level.h"
#include "wid_game_map.h"

static void thing_action_timer_callback_dead (levelp level, 
                                              thing_place_context_t *place)
{
    thingp thing = id_to_thing(place->thing_id);

    if (thing) {
        verify(thing);

        thing_dead(level, thing, 0, "callback");
    }

    place->thing_id = 0;
}

static void thing_timer_place_and_destroy_callback (levelp level,
                                                    thing_place_context_t *place)
{
    if (!place->tp) {
        ERR("no thing to place");
    }

    widp w;

    w = wid_game_map_replace_tile(level,
                                  place->x,
                                  place->y,
                                  0, /* thing */
                                  place->tp,
                                  0 /* tpp data */);

    /*
     * Some things like projectiles can go offscrenn.
     */
    if (!w) {
        return;
    }

    /*
     * Just pass the same context along as it has the expire time but add
     * the newborn thing.
     */
    thingp t = wid_get_thing(w);

    if (place->is_epicenter) {
        t->is_epicenter = 1;
    }

    place->thing_id = t->thing_id;

    /*
     * Save the owner of this new thing. This could be who cast a spell.
     */
    thing_set_owner_id(level, t, place->owner_id);

    THING_LOG(t, "Destroy newborn thing via timer");
}

static void thing_timer_place_callback (levelp level,
                                        thing_place_context_t *place)
{
    widp w;

    w = wid_game_map_replace_tile(level,
                                  place->x,
                                  place->y,
                                  0, /* thing */
                                  place->tp,
                                  0 /* tpp data */);

    /*
     * Some things like projectiles can go offscrenn.
     */
    if (!w) {
        return;
    }

    thingp t = wid_get_thing(w);

    /*
     * Save the owner of this new thing. This could be who cast a spell.
     */
    thing_set_owner_id(level, t, place->owner_id);
}

/*
 * Various thing timers.
 */
void thing_timers_destroy (levelp level, thingp t)
{
    uint32_t id = thing_id(t);
    thing_place_context_t *p;
    int i;

    if (!id) {
        DIE("no thing id to remove");
    }

    for (i = 0; i < MAX_TIMERS_PER_LEVEL; i++) {
        p = &level->timers[i];
        if (p->thing_id == id) {
            memset(p, 0, sizeof(*p));
        }

        if (p->owner_id == id) {
            memset(p, 0, sizeof(*p));
        }
    }
}

void thing_timers_tick (levelp level)
{
    thing_place_context_t *p;
    int i;

    for (i = 0; i < MAX_TIMERS_PER_LEVEL; i++) {
        p = &level->timers[i];
        if (p->fire_in) {
            p->fire_in -= MAIN_LOOP_DELAY;

            if (!p->fire_in) {
                if (p->destroy_in) {
                    thing_timer_place_and_destroy_callback(level, p);
                } else {
                    thing_timer_place_callback(level, p);
                }
            }
            continue;
        }

        if (p->destroy_in) {
            p->destroy_in -= MAIN_LOOP_DELAY;

            if (!p->destroy_in) {
                thing_action_timer_callback_dead(level, p);
            }
            continue;
        }
    }
}

/*
 * Place a thing after a delay.
 */
void thing_place_timed (levelp level,
                        tpp tp,
                        double x,
                        double y,
                        uint32_t ms,
                        uint32_t jitter)
{
    thing_place_context_t context = {0};

    context.x = x;
    context.y = y;
    context.tp = tp;
    context.fire_in = ms + jitter;

    thing_place_context_t *p;
    int i;

    for (i = 0; i < MAX_TIMERS_PER_LEVEL; i++) {
        p = &level->timers[i];
        if (!p->fire_in && !p->destroy_in) {
            memcpy(p, &context, sizeof(*p));
            return;
        }
    }
    ERR("out of timers");
}

/*
 * Place a thing after a delay.
 */
void thing_place_and_destroy_timed (levelp level,
                                    tpp tp,
                                    thingp owner,
                                    double x,
                                    double y,
                                    uint32_t ms,
                                    uint32_t destroy_in,
                                    uint32_t jitter,
                                    uint8_t is_epicenter)
{
    thing_place_context_t context = {0};

    context.x = x;
    context.y = y;
    context.fire_in = ms + jitter;
    context.destroy_in = destroy_in;
    context.tp = tp;
    context.is_epicenter = is_epicenter ? 1 : 0;

    LOG("Place newborn %s via timer", tp_short_name(tp));

    if (owner) {
        context.owner_id = owner->thing_id;
    }

    thing_place_context_t *p;
    int i;

    for (i = 0; i < MAX_TIMERS_PER_LEVEL; i++) {
        p = &level->timers[i];
        if (!p->fire_in && !p->destroy_in) {
            memcpy(p, &context, sizeof(*p));
            return;
        }
    }
    ERR("out of timers");
}
