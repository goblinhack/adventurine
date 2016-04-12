/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing_timer.h"
#include "timer.h"
#include "level.h"
#include "wid_game_map.h"
#include "thing_shop.h"
#include "math_util.h"

static int someone_is_inside_a_shop = false;

static int thing_tick_all_things (levelp level)
{
//    int count = 0;
    thingp t;

    FOR_ALL_THINGS(level, t) {

        tpp tp;
        widp w;

        /*
         * Sanity checks.
         */
        THING_SANITY(level, t);
        tp = thing_tp(t);
        if (tp_is_inactive(tp)) {
            continue;
        }

//    count++;
        w = t->wid;
        if (likely(w != 0)) {
            verify(w);
        } else {
            THING_ERR(t, "has no widget");
            thingp owner = thing_owner(level, t);
            if (owner) {
                THING_ERR(t, "has no widget, owner %s", thing_logname(owner));
                THING_ERR(t, "owner weapon_carry_anim_thing_id %d",
                          owner->weapon_carry_anim_thing_id);
                THING_ERR(t, "owner weapon_swing_anim_thing_id %d",
                          owner->weapon_swing_anim_thing_id);
            }

            continue;
        }

        if (t->momentum) {
            thing_slide(level, t);
        }

        if (t->jump_speed) {
            thing_jump(level, t);
        }

        if (tp_can_fall(tp)) {
            thing_fall(level, t);
        }

        someone_is_inside_a_shop = false;

        if (thing_is_player(t)) {
            /*
             * Count our torches.
             */
            thing_torch_update_count(level, t, false /* force */);

            thing_health_tick(level, t);

            /*
             * If any player is in the shop, show the prices.
             */
            someone_is_inside_a_shop += shop_inside(level, t);
        }

        /*
         * Thing has croaked it?
         */
        if (thing_is_dead(t)) {
            thing_destroy(level, t, "died");
            continue;
        }

        if (thing_is_mob_spawner(t)) {
            /*
             * Time to spawn a thing?
             */
            uint32_t delay = tp_get_mob_spawn_delay_tenths(tp);
            if (!delay) {
                DIE("mob spawner %s with no delay", thing_logname(t));
            }

            if (time_have_x_tenths_passed_since(delay, 
                                                t->timestamp_mob_spawn)) {
                /*
                 * Skip first time around else new born things spawn in a 
                 * loop.
                 */
                thing_mob_spawn(level, t);

                /*
                 * Some things like corpses, spawn once and then die
                 */
                if (thing_is_dead(t)) {
                    continue;
                }

                /*
                 * Add some jitter.
                 */
                uint32_t delta = time_get_time_ms() - t->timestamp_mob_spawn;

                t->timestamp_mob_spawn = 
                                time_get_time_ms() + myrand() % (delta / 2);
            }
        }

        if (tp_get_lifespan_ticks(tp)) {
            if (time_have_x_ticks_passed_since(
                    tp_get_lifespan_ticks(tp) +
                        (myrand() % tp_get_lifespan_ticks(tp)) / 2,
                            t->tick_born)) {
                thing_dead(level, t, 0, "out of life"); 
                continue;
            }
        }

        /*
         * If a projectile, move it by the delta
         */
        int need_collision_test = false;

        if (!wid_is_moving(w)) {
            /*
             * Only if it finished moving the last delta.
             */
            if ((t->dx != 0) || (t->dy != 0)) {
                /*
                 * Clean up missiles that have hit the edge and stopped 
                 * moving.
                 */
                if ((t->x <= 0) || 
                    (t->y <= 0) || 
                    (t->x >= MAP_WIDTH - 1) ||
                    (t->y >= MAP_HEIGHT - 1)) {
                    thing_dead(level, t, 0, "missile at edge of level"); 
                    continue;
                }

                need_collision_test = true;
            }
        }

        /*
         * Is this a thing that fires at player?
         */
        uint32_t delay = tp_get_weapon_fire_delay_hundredths(tp);
        if (delay) {
            if (time_have_x_hundredths_passed_since(delay, 
                                                    t->timestamp_fired)) {
                t->timestamp_fired = time_get_time_ms();
                thing_fire_at_target(level, t);
            }
        }

        /*
         * Need this for explosions and other non moving things to interact 
         * and KILL THINGS!
         */
        if (time_have_x_tenths_passed_since(DELAY_TENTHS_THING_COLLISION_TEST,
                                            t->timestamp_collision)) {
            need_collision_test = true;
        }

        /*
         * Do expensive collision tests less often. But for weapons do them
         * all the time else you can have weapons speed through walls.
         */
        if (need_collision_test) {

            if (!thing_handle_collisions(level, t)) {
                return (false);
            }

            t->timestamp_collision = time_get_time_ms() + (myrand() % 100);

            /*
             * Died in a collision ? 8( Handle it next time around.
             */
            if (thing_is_dead(t)) {
                continue;
            }
        } else if (t->one_shot_move) {
            /*
             * Always try and move; might be wanting to jump
             */
        } else {
            /*
             * If not moving then we need to do a check to see if we can move
             * else we get laggy steps when a thing stops moving and waits for
             * its next hop.
             */
            if (wid_is_moving(w)) {
                continue;
            }
        }

        if (!wid_is_moving(w)) {
            /*
             * Only if it finished moving the last delta.
             */
            if ((t->dx != 0) || (t->dy != 0)) {
                double fnexthop_x = t->x + t->dx;
                double fnexthop_y = t->y + t->dy;

                thing_move(level, t,
                           fnexthop_x,
                           fnexthop_y,
                           fnexthop_y < t->y,
                           fnexthop_y > t->y,
                           fnexthop_x < t->x,
                           fnexthop_x > t->x,
                           false /* fire */);
            }
        }

        /*
         * If stopped moving, look for a next hop.
         */
        w = t->wid;

        /*
         * If waiting to update this thing to the client, like if it was newly 
         * born, then do not move it before the client gets a chance to find 
         * out.
         */
        float speed = tp_get_speed(tp);

        int look_for_new_hop = false;

        if (t->one_shot_move) {
            /*
             * Forced look for a new hop.
             */
            look_for_new_hop = true;
        } else if (thing_is_monst(t) && !wid_is_moving(w) && w && speed) {
            /*
             * Look for a new hpp.
             */
            look_for_new_hop = true;
        }

        if (look_for_new_hop) {
            int32_t nexthop_x = -1;
            int32_t nexthop_y = -1;

            if (thing_find_nexthop(level,
                                   t, 
                                   &nexthop_x, &nexthop_y)) {

                /*
                 * Let the slimes jump!
                 */
                if (t->one_shot_move) {
                    t->one_shot_move = false;
                }

                /*
                 * Add some jitter.
                 */
                t->timestamp_ai = time_get_time_ms() + (myrand() % 100);
            }
        }

        if (w) {
            switch (t->dir) {
            case THING_DIR_TL:
            case THING_DIR_BL:
            case THING_DIR_LEFT:
                if (tp_is_effect_rotate_2way(tp)) {
                    wid_flip_horiz(w, true);
                }
                break;

            case THING_DIR_TR:
            case THING_DIR_BR:
            case THING_DIR_RIGHT:
                if (tp_is_effect_rotate_2way(tp)) {
                    wid_flip_horiz(w, false);
                }
                break;

            case THING_DIR_UP:
            case THING_DIR_DOWN:
                break;
            }
        }
    }
    FOR_ALL_THINGS_END

    return (true);
}

int thing_tick_all (levelp level)
{
    if (!level) {
        return (false);
    }


    /*
     * Do per tick stuff for the level
     */
    if (!level_tick(level)) {
        return (false);
    }

    game.tick++;

#ifdef ENABLE_MAP_SANITY
    thing_map_sanity();
#endif

    if (!thing_tick_all_things(level)) {
        return (false);
    }

    return (true);
}

/*
 * Things like shop prices that respawn occasionally.
 */
static void thing_animate_slow (levelp level)
{
    static uint32_t last_slow_tick;
    int slow_tick = false;

    /*
     * Not too slow or shop items show prices too quickly.
     */
    if (time_have_x_secs_passed_since(2, last_slow_tick)) {
        slow_tick = true;
        last_slow_tick = time_get_time_ms();
    }

    if (!slow_tick) {
        return;
    }

    thingp t;

    FOR_ALL_THINGS(level, t) {

        /*
         * Sanity checks.
         */
        THING_SANITY(level, t);
        tpp tp = thing_tp(t);

        if (thing_is_treasure(t)) {
            if (someone_is_inside_a_shop) {
                thing_shop_item_tick(level, t);
            }
        }

        /*
         * Random utterance?
         */
        uint32_t delay = tp_get_sound_random_delay_secs(tp);
        if (delay) {
            if (time_have_x_secs_passed_since(myrand() % delay, 
                                                t->timestamp_sound_random)) {
                t->timestamp_sound_random = time_get_time_ms();
                MSG_SHOUT_AT(SOUND, t, t->x, t->y, "%s", tp->sound_random);
            }
        }
    }
    FOR_ALL_THINGS_END
}

void thing_animate_all (levelp level)
{
    thingp t;

    if (!level) {
        return;
    }

    FOR_ALL_THINGS(level, t) {

        THING_SANITY(level, t);

        if (t->wid && thing_is_animated(t)) {
            thing_animate(level, t);
        }

        if (t->destroy_in_ms > 0) {
            t->destroy_in_ms -= MAIN_LOOP_DELAY;
            if (t->destroy_in_ms <= 0) {
                thing_dead(level, t, 0, "timer expired");
                continue;
            }
        }
    }

    FOR_ALL_THINGS_END

    thing_animate_slow(level);
}
