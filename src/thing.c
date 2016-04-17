/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing_timer.h"
#include "wid_game_map.h"
#include "string_util.h"
#include "sound.h"
#include "timer.h"
#include "tile.h"
#include "wid_hiscore.h"
#include "thing_shop.h"
#include "thing_tile.h"
#include "player.h"

int things_total;
int monst_things_total;

static uint8_t thing_init_done;

/*
 * What things live on the map.
 */
thing_map_t thing_map;

uint8_t thing_init (void)
{
    thing_init_done = true;

    return (true);
}

void thing_fini (void)
{
    if (thing_init_done) {
        thing_init_done = false;
    }
}

/*
 * Create a new thing.
 */
static void thing_try_to_flush_ids_ (levelp level)
{
    thingp t;

    FOR_ALL_THINGS(level, t)

        if (thing_is_cloud_effect(t)) {
            thing_destroy(level, t, "too many things");
        }

    FOR_ALL_THINGS_END
}

static void thing_try_to_flush_ids (levelp level)
{
    thing_try_to_flush_ids_(level);
}

static void thing_map_dump_ (thing_map_t *map, const char *name)
{
    uint32_t i;
    uint32_t x;
    uint32_t y;

    FILE *fp;

    fp = fopen(name, "w");

    uint32_t width = 0;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            thing_map_cell *cell = &map->cells[x][y];

            width = max(width, cell->count);
        }
    }

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            thing_map_cell *cell = &map->cells[x][y];

            for (i = 0; i < width; i++) {
                uint32_t m = cell->id[i];

                if (!m) {
                    fprintf(fp, "----- ");
                    continue;
                }

                fprintf(fp, "%5u ", m);
            }

            fprintf(fp, "|");
        }
        fprintf(fp, "\n");
    }
}

void thing_map_dump (void)
{
    thing_map_dump_(&thing_map, "thing.map");
}

static void thing_map_sanity_ (levelp level, thing_map_t *map)
{
    uint32_t i;
    uint32_t x;
    uint32_t y;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            thing_map_cell *cell = &map->cells[x][y];

            uint8_t found_end = 0;

            for (i = 0; i < MAP_THINGS_PER_CELL; i++) {
                uint32_t m = cell->id[i];

                if (!m) {
                    found_end = true;
                    continue;
                }

                if (found_end) {
                    thing_map_dump();

                    ERR("map elements are not contiguous at %d,%d", x, y);
                }

                thingp t = id_to_thing(m);
                if (!t) {
                    ERR("thing %p id %08X is invalid and on map", t, m);
                }

                verify(t);
            }
        }
    }
}

void thing_map_sanity (levelp level)
{
    thing_map_sanity_(level, &thing_map);
}

void thing_sanity (levelp level, thingp t)
{
    verify(t);

    /*
     * On the client owners may be missing due to order of deletion
     * of objects.
     */
    thingp tmp;
    tmp = thing_owner(level, t);
    if (tmp) {
        verify(tmp);
    }

    tmp = thing_weapon_carry_anim(level, t);
    if (tmp) {
        verify(tmp);
    }

    tmp = thing_weapon_swing_anim(level, t);
    if (tmp) {
        verify(tmp);
    }
}

void thing_map_remove (levelp level, thingp t)
{
    int i;

    verify(t);

    int32_t x = t->map_x;
    int32_t y = t->map_y;

    /*
     * Check not on the map.
     */
    if ((x == -1) || (y == -1)) {
        return;
    }

    thing_map_t *map = level_map(level);
    thing_map_cell *cell = &map->cells[x][y];

    if (!cell->count) {
        ERR("map count mismatch");
        return;
    }

    /*
     * Remove from the map.
     */
    for (i = 0; i < cell->count; i++) {
        uint32_t m = cell->id[i];
        if (m != thing_id(t)) {
            continue;
        }

        if (i == cell->count - 1) {
            /*
             * Popping last element.
             */
            cell->id[i] = 0;
        } else {
            /*
             * Pop and swap last element.
             */
            cell->id[i] = cell->id[cell->count - 1];
            cell->id[cell->count - 1] = 0;
        }

        cell->count--;

        t->map_x = -1;
        t->map_y = -1;

        return;
    }

    ERR("did not find id %08X/%s on map at %d,%d to remove",
        thing_id(t), thing_logname(t), x, y);
}

void thing_map_add (levelp level, thingp t, int32_t x, int32_t y)
{
    verify(t);

    if (!t->thing_id) {
        DIE("cannot add thing %s ID of 0", thing_logname(t));
    }

    if (x < 0) {
        DIE("map underflow for thing %s", thing_logname(t));
    }

    if (y < 0) {
        DIE("map y underflow for thing %s", thing_logname(t));
    }

    if (x >= MAP_WIDTH) {
        DIE("map x overflow for thing %s", thing_logname(t));
    }

    if (y >= MAP_HEIGHT) {
        DIE("map y overflow for thing %s", thing_logname(t));
    }

    thing_map_t *map = level_map(level);

    /*
     * Check not on the map.
     */
    if ((t->map_x != -1) || (t->map_y != -1)) {
        ERR("thing %s already on map at %d,%d", thing_logname(t),
            t->map_x, t->map_y);
        return;
    }

    uint32_t i;

    /*
     * Sanity check we're not on already.
     */
    thing_map_cell *cell = &map->cells[x][y];

#ifdef ENABLE_MAP_SANITY
    for (i = 0; i < cell->count; i++) {
        uint32_t m = cell->id[i];

        if (!m) {
            continue;
        }

        /*
         * Something is on the map.
         */
        if (m == t->thing_id) {
            /*
             * It's us?
             */
            ERR("already found on map");
        }

        thingp p = id_to_thing(m);
        if (p == t) {
            ERR("already found thing %s on map", thing_logname(t));
        }
    }
#endif

    if (cell->count == MAP_THINGS_PER_CELL) {
        /*
         * Try to find something we can boot out.
         */
        if (thing_is_cloud_effect(t)    ||
            thing_is_explosion(t)       ||
            thing_is_projectile(t)      ||
            thing_is_weapon_swing_effect(t)) {
            /*
             * Don't bother. This is a transient thing.
             */
            return;
        }

        /*
         * This is a more important thing. Try and boot out something less
         * important.
         */
        for (i = 0; i < cell->count; i++) {
            uint32_t m = cell->id[i];
            if (!m) {
                DIE("expected to find a map id on the map here");
            }

            thingp p = id_to_thing(m);
            if (!p) {
                DIE("expected to find a thing on the map here at slot %d", m);
            }

            if (thing_is_cloud_effect(p)    ||
                thing_is_explosion(p)       ||
                thing_is_projectile(t)      ||
                thing_is_weapon_swing_effect(p)) {
                /*
                 * Kick out this transient thing.
                 */
                thing_map_remove(level, p);
                break;
            }
        }
    }

    if (cell->count == MAP_THINGS_PER_CELL) {
        /*
         * We're hosed.
         */
        ERR("Out of map slots trying to add %s", thing_logname(t));

        for (i = 0; i < cell->count; i++) {
            uint32_t m = cell->id[i];
            thingp p = id_to_thing(m);

            LOG("  slot [%d] id %08X %s", i, m, thing_logname(p));
        }

        return;
    }

    cell->id[cell->count] = t->thing_id;
    cell->count++;

    t->map_x = x;
    t->map_y = y;
}

/*
 * Find a new thing slot on the lvel
 */
static uint32_t thing_new_id (levelp level)
{
    /*
     * Use a different base for monsters so that the IDs we create are going
     * to be contiguous and allows us to optimize when sending map updates.
     */
    uint32_t *next;
    uint32_t id;
    uint32_t min;
    uint32_t max;

    /*
     * Reserve thing id 0 for unused.
     */
    next = &level->next_thing_id;
    id = level->next_thing_id;
    min = 1;
    max = MAX_THINGS_PER_LEVEL - 1;

    if (!id) {
        id = min;
    }

    /*
     * Find a free thing slot
     */
    int looped = 0;

    while (level->things[id].thing_id) {
        id++;
        if (id >= max) {
            id = min;
            looped++;

            /*
             * Try hard to reclaim space.
             */
            if (looped == 2) {
                thing_try_to_flush_ids(level);
            }

            if (looped == 3) {
                ERR("out of thing ids, min %u max %u!", min, max);
            }
        }
    }

    *next = id + 1;
    if (*next >= max) {
        *next = min;
    }

    if (!id) {
        DIE("sanity check, ID 0 never used min %u max %u", min, max);
    }

    return (id);
}

/*
 * Create a new thing.
 */
thingp thing_new (levelp level,
                  const char *name,
                  double x, double y)
{
    thingp t;
    tpp tp;

    tp = tp_find(name);
    if (!tp) {
        ERR("thing [%s] has no template", name);
    }

    uint32_t id = thing_new_id(level);

    t = &level->things[id];
    newptr(t, "thing");
    memset(t, 0, sizeof(*t));

    t->thing_id = (level->level_no << 24) | id | (myrand() & THING_ID_RAND_MASK);

    t->tp_id = tp_to_id(tp);

    /*
     * New items are top quality.
     */
    t->is_sleeping = tp_is_sleeping(tp);
    t->tick_born = game.tick;
    t->timestamp_sound_random = time_get_time_ms();

    /*
     * Set a timestamp so we don't fire on birth. Else mudmen spawned from mud 
     * multiply crazily
     */
    t->timestamp_fired = time_get_time_ms();

    if (tp_get_mob_spawn_delay_tenths(tp)) {
        t->timestamp_mob_spawn = time_get_time_ms() +
            myrand() % (tp_get_mob_spawn_delay_tenths(tp) * 100);
    }

    /*
     * Start our with max stats.
     */
    t->hp = tp->max_hp;
    t->max_hp = tp->max_hp;

    if (thing_is_player(t)) {
        t->torches = 10;
        t->ropes = 5;
        t->bombs = 1;
        thing_wield(level, t, id_to_tp(THING_SWORD1));
    }

    thing_wield_next_weapon(level, t);

    thing_reinit(level, t, x, y);

    if (thing_is_monst(t)) {
        monst_things_total++;
    }

    things_total++;

    /*
     * Too boring to log about?
     */
    if (thing_is_monst(t) ||
        thing_is_player_or_owned_by_player(level, t)) {

        THING_LOG(t, "Created (total things %d, monsts %d)",
                  things_total, monst_things_total);
    }

    return (t);
}

/*
 * Reinit this player on a new level
 */
void thing_reinit (levelp level, thingp t, double x, double y)
{
    verify(t);

    /*
     * Start out not on the map.
     */
    t->last_x = -1.0;
    t->last_y = -1.0;
    t->map_x = -1.0;
    t->map_y = -1.0;

    t->x = x;
    t->y = y;
}

void thing_restarted (levelp level, thingp t)
{
    verify(t);

    t->current_tile = 0;

    if (!thing_is_dead(t)) {
        return;
    }

    thing_set_is_dead(t, false);
}

/*
 * Get rid of all the hooks to other things that this thing has. e.g. the
 * weapons it carries etc.
 */
static void thing_remove_hooks (levelp level, thingp t)
{
    verify(t);

    /*
     * We are owned by something. i.e. we are a sword.
     */
    thingp owner = 0;

#ifdef THING_DEBUG
        THING_LOG(t, "remove hooks");
#endif

    if (t->owner_thing_id) {
        owner = thing_owner(level, t);
    }

    if (t->owner_thing_id && owner) {
#ifdef THING_DEBUG
        THING_LOG(t, "detach from owner %s", thing_logname(owner));
#endif

        if (t->thing_id == owner->weapon_carry_anim_thing_id) {
            thing_unwield(level, owner, "remove hooks");

#ifdef THING_DEBUG
            THING_LOG(t, "detach from carry anim owner %s", thing_logname(owner));
#endif

            thing_set_weapon_carry_anim(level, owner, 0);
        }

        if (t->thing_id == owner->weapon_swing_anim_thing_id) {
#ifdef THING_DEBUG
            THING_LOG(t, "detach from swing anim owner %s", thing_logname(owner));
#endif

            thing_set_weapon_swing_anim(level, owner, 0);

            /*
             * End of the swing animation, make the sword visible again.
             */
            thingp carry = thing_weapon_carry_anim(level, owner);
            if (carry) {
                /*
                 * But only if the owner is visible. They may have reached the
                 * level.
                 */
                if (thing_is_visible(level, owner)) {
                    thing_visible(level, carry);
                }
            }
        }

        thing_set_owner(level, t, 0);
    }

    /*
     * We own things like a sword. i.e. we are a player.
     */
    if (t->weapon_carry_anim_thing_id) {
        thingp item = thing_weapon_carry_anim(level, t);
        thing_set_weapon_carry_anim(level, t, 0);
        verify(item);
        thing_set_owner(level, item, 0);
        thing_dead(level, item, 0, "weapon carry anim owner killed");
    }

    if (t->weapon_swing_anim_thing_id) {
        thingp item = thing_weapon_swing_anim(level, t);
        thing_set_weapon_swing_anim(level, t, 0);
        verify(item);
        thing_set_owner(level, item, 0);
        thing_dead(level, item, 0, "weapon swing anim owner killed");
    }

    /*
     * Some things have lots of things they own
     */
    if (t->owned_count) {
        THING_LOG(t, "Remove remaining %d owned things", t->owned_count);

        thingp i;

        FOR_ALL_THINGS(level, i)
            thingp owner = thing_owner(level, i);
            if (owner && (owner == t)) {
                thing_set_owner(level, i, 0);
            }
        FOR_ALL_THINGS_END
    }
}

void thing_destroy (levelp level, thingp t, const char *why)
{
    verify(t);

    if (why &&
        (thing_is_monst(t) ||
         thing_is_player_or_owned_by_player(level, t))) {
        THING_LOG(t, "Destroyed, why(%s)", why);
    }

    /*
     * Stop all timers.
     */
    thing_timers_destroy(level, t);

    /*
     * Detach from the owner
     */
    thing_remove_hooks(level, t);

    /*
     * Destroy the things weapon. Eventually drop a backpack.
     */
    thing_unwield(level, t, "thing destroy");

    if (t->wid) {
        thing_map_remove(level, t);
        thing_set_wid(level, t, 0);
    }

    /*
     * Record that the client player may have died so we do not disconnect.
     */
    if (t == player) {
        player_wid_destroy(level);

        player = 0;
    }

    if (thing_is_monst(t)) {
        monst_things_total--;
    }

    things_total--;

    t->thing_id = 0;

    oldptr(t);
}

void thing_destroy_in (levelp level, thingp t, int32_t ms)
{
    t->destroy_in_ms = ms;
}

static void thing_dead_ (levelp level, thingp t, char *reason)
{
    /*
     * Detach from the owner
     */
    thing_remove_hooks(level, t);

    if (!reason) {
        ERR("thing %s dead for no reason? why? why? why?!", thing_logname(t));
    }

    if (thing_is_dungeon_floor(t)   || 
        thing_is_ladder(t)          || 
        thing_is_door(t)            || 
        thing_is_cloud_effect(t)    || 
        thing_is_explosion(t)       || 
        thing_is_projectile(t)      || 
        thing_is_rock(t)            ||
        thing_is_wall(t)) {
        /*
         * Too boring to log.
         */
    } else {
        THING_LOG(t, "Dead, why(%s)", reason);
    }

    if (thing_is_player(t)) {
        /*
         * Tell the poor player they've croaked it.
         */
        hiscore_try_to_add("Happless human", reason, t->score);
    }

    const char *sound = tp_sound_on_death(thing_tp(t));
    if (sound) {
        MSG_SHOUT_AT(SOUND, t, t->x, t->y, "%s", sound);
    }

    /*
     * Move the weapon behind the poor thing.
     */
    thing_set_weapon_placement(level, t);

    myfree(reason);
}

void thing_dead (levelp level, 
                 thingp t, 
                 thingp killer, 
                 const char *reason, ...)
{
    /*
     * If in a shop, this might be the shopkeeper.
     */
    thingp owner = thing_owner(level, t);

    /*
     * If an arrow, this might be an elf.
     */
    thingp real_killer = 0;

    if (killer) {
        real_killer = thing_owner(level, killer);
        if (!real_killer) {
            real_killer = killer;
        }
    }

    va_list args;

    verify(t);

    tpp tp = thing_tp(t);

    /*
     * If the reason of death was collection, some things we do not want
     * to do.
     */
    if (!t->is_collected) {
        /*
         * When it dies, doth it polymorph and thus avoid the reaper?
         * e.g. a mob spawner dying and creating a smaller one.
         */
        const char *polymorph = tp_polymorph_on_death(tp);

        if (thing_is_sawblade(t) && killer) {
            /*
             * Skip polymorph if there is a killer. We want the blades to
             * just vanish and not get more bloody. That only happens if
             * there is no killer and we force a polymorph.
             */
        } else if (polymorph) {
            tpp what = tp_find(polymorph);
            if (!what) {
                ERR("could now find %s to polymorph into on %s death",
                    polymorph, thing_logname(t));
            }

            /*
             * It doth polymorph.
             */
            t->tp_id = tp_to_id(what);
            t->hp = what->max_hp;
            return;
        }

        /*
         * Or perhaps it does die, but spawns something else, like the
         * player dying and creating a mob spawner.
         */
        const char *spawn = tp_spawn_on_death(tp);
        if (spawn) {
            thingp newt = thing_mob_spawn_on_death(level, t);

            /*
             * If this is the player death then give the gravestone a lot of 
             * health or it can be immediately killed by a lingering explosion 
             * that killed the player too.
             */
            if (newt && thing_is_player(t)) {
                newt->hp = 200;
            }
        }
    }

    /*
     * You only die once.
     */
    if (thing_is_dead(t)) {
        return;
    }

    thing_set_is_dead(t, true);

    /*
     * Bounty for the killer?
     */
    uint32_t xp = tp_get_bonus_score_on_death(tp);
    if (xp && real_killer) {
        /*
         * Did someone throw this weapon and gets the xp?
         */
        int32_t val = tp_get_bonus_score_on_death(tp);

        if (val) {
            real_killer->score += val;

            if (thing_is_player(real_killer)) {
#if 0
                if (thing_is_cloud_effect(killer)) {
                    /*
                        * Too many packets if we kill a lot of things in one
                        * go.
                        *
                        * But it looks nice... 8)
                        */
                } else {
#endif
                    MSG_SHOUT_AT(OVER_THING,
                                 t,
                                 0, 0,
                                 "%%%%font=%s$%%%%fg=%s$+%d XP",
                                 "large", "gold",
                                 val);
#if 0
                }
#endif
            }
        }
    }

    /*
     * Flash briefly red on death.
     */
    if (thing_is_monst(t)        ||
        thing_is_mob_spawner(t)  ||
        thing_is_rock(t)         ||
        thing_is_wall(t)         ||
        thing_is_door(t)) {

        widp w = t->wid;
        if (w) {
            wid_set_mode(w, WID_MODE_ACTIVE);
            if (!wid_is_hidden(w)) {
                wid_set_color(w, WID_COLOR_BLIT, RED);
            }
        }
    }

    /*
     * Boom! If this bomb is not being collected then make it blow up.
     */
    {
#if 0
if (thing_is_treasure(t)) {
CON("%s destroyed",thing_logname(t));
if (owner) {
CON("  %s owner is keeper",thing_logname(owner));
}
if (killer) {
CON("  %s killer ",thing_logname(killer));
}
if (real_killer) {
CON("  %s real_killer ",thing_logname(real_killer));
}
}
#endif
        if (!t->is_collected) {
            if (thing_is_bomb(t)        || 
                thing_is_fireball(t)    ||
                thing_is_bullet(t)) {
                level_place_explosion(level,
                                      0, /* owner */
                                      thing_tp(t),
                                      t->x, t->y,
                                      t->x, t->y);
            }

            /*
             * Breaking stuff in a shop? bad idea.
             */
            if (thing_is_treasure(t)) {
                if (owner && thing_is_shopkeeper(owner)) {
                    if (thing_is_player(real_killer)) {
                        shop_break_message(level, real_killer, owner);
                    } else {
                        shop_whodunnit_break_message(level, real_killer, owner);
                    }
                }
            }
        } else {
            /*
             * Collecting a thing?
             */
            if (thing_is_treasure(t)) {
                if (owner && thing_is_shopkeeper(owner)) {
                    if (thing_is_player(real_killer)) {
                        shop_collect_message(level, real_killer, t);
                    }
                }
            }
        }
    }

    /*
     * Stop bouncing or swaying.
     */
    if (t->wid) {
        if (tp_is_effect_pulse(tp)) {
            wid_scaling_to_pct_in(t->wid, 1.0, 1.0, 0, 0);
        }

        if (tp_is_effect_sway(tp)) {
            wid_rotate_to_pct_in(t->wid, 0, 0, 0, 0);
        }
    }

    /*
     * Log the means of death!
     */
    if (reason) {
        va_start(args, reason);
        thing_dead_(level, t, dynvprintf(reason, args));
        va_end(args);
    } else {
        thing_dead_(level, t, 0);
    }
}

void thing_wake (levelp level, thingp t)
{
    verify(t);

    if (!t->is_sleeping) {
        return;
    }

    THING_LOG(t, "Wake");

    thing_set_is_sleeping(t, false);
}

static int thing_hit_ (levelp level,
                       thingp t, 
                       thingp orig_hitter, 
                       thingp real_hitter, 
                       thingp hitter, 
                       int32_t damage)
{
    int32_t orig_damage = damage;

    verify(t);

    /*
     * Cruel to let things keep on hitting you when you're dead
     */
    if (thing_is_dead(t)) {
        return (false);
    }

    if (!damage) {
        /*
         * Could be a spider silkball
         */
        return (false);
    }

    t->timestamp_last_attacked = time_get_time_ms();

    /*
     * Take note of the hit so we can send an event to the client.
     *
     * Send this for both player and monsters so we see monsters being 
     * pumelled!
     */
#if 0
CON("%s hit success on %s hitter %s",
    thing_logname(t), 
    hitter ? thing_logname(hitter) : "",
    real_hitter ? thing_logname(real_hitter) : "");
#endif

    thing_effect_hit_success(level, t);

    if (thing_is_lava(hitter)) {
        thing_effect_flames(level, t);
    }

    if (thing_is_acid(hitter)) {
        thing_effect_bubbles(level, t);
    }

    if (damage > t->hp / 10) {
        thing_effect_hit_crit(level, t);
    }

    /*
     * Keep hitting until all damage is used up or the thing is dead.
     */
    THING_LOG(t, "Is hit! hp %d, damage %d", t->hp, damage);

    /*
     * Clash of swords?
     */
    const char *sound = 
        tp_sound_on_hitting_something(thing_tp(orig_hitter));
    if (sound) {
        if (thing_is_player(real_hitter)) {
            MSG_SHOUT_AT(SOUND, 
                                real_hitter, 
                                t->x, t->y,
                                "%s", sound);
        } else if (thing_is_player(t)) {
            /*
             * Orc hitting player
             */
            MSG_SHOUT_AT(SOUND, t, 
                                t->x, t->y, 
                                "%s", sound);
        }
    }

    while (damage > 0) {
        if (t->hp <= damage) {
            damage -= t->hp;

            t->hp = 0;

            THING_LOG(t, "Is hit terminally, hp %d, damage %d", 
                      t->hp, damage);

            /*
             * Record who dun it.
             */
            thing_dead(level, t, orig_hitter, "%s",
                       tp_short_name(thing_tp(real_hitter)));

            /*
             * Explodes on death ala Sith Lord? Only a lesser one, mind.
             */
            if (tp_is_combustable(thing_tp(t))) {
                level_place_explosion(level,
                                      0, // owner
                                      thing_tp(t),
                                      t->x, t->y,
                                      t->x, t->y);
            }

            THING_LOG(t, "Is hit by (%s) for %u, now dead", 
                      thing_logname(orig_hitter), damage);

            if (t == orig_hitter) {
                THING_ERR(t, "Hitting thyself? wth.");
                DIE("hitting thyself");
                return (false);
            }

            /*
             * If polymorphed, hit again?
             */
            if (!t->hp) {
                /*
                 * No it really died.
                 */
                break;
            }

            if (!damage) {
                /*
                 * If polymorphed, but we're out of damage.
                 */
                break;
            }
        } else {
            /*
             * A hit, but not enough to kill the thing.
             */
            t->hp -= damage;

            if (t->hp < 0) {
                t->hp = 0;
            }

            THING_LOG(t, "Is hit by (%s) for %u, hp now %d",
                      thing_logname(orig_hitter), damage,
                      t->hp);

            if (t == orig_hitter) {
                THING_ERR(t, "Hitting thyself? wth.");
                DIE("hitting thyself");
                return (false);
            }

            damage = 0;
        }
    }

    /*
     * If a thing that modifies the level dies, update it.
     */
    if (thing_is_dead(t)) {
        /*
         * Destroying one door opens all doors.
         */
        if (thing_is_door(t)) {
            level_open_door(level, t->x-1, t->y);
            level_open_door(level, t->x+1, t->y);
            level_open_door(level, t->x, t->y-1);
            level_open_door(level, t->x, t->y+1);
        }
    } else {
        if (orig_damage > 0) {
            const char *color = "white";
            const char *font = "small";

            if (orig_damage > 20) {
                font = "vlarge";
            } else if (orig_damage > 10) {
                font = "large";
            } else if (orig_damage > 5) {
                font = "medium";
            } else if (orig_damage > 2) {
                font = "small";
            } else {
                font = "vsmall";
            }

            if (thing_is_player(t)) {
                color = "red";
            }

            MSG_SHOUT_AT(OVER_THING, t,
                                t->x, t->y,
                                "%%%%font=%s$%%%%fg=%s$-%d HP",
                                font, color, orig_damage);
        }
    }

    return (true);
}

int thing_hit (levelp level, thingp t, thingp hitter, uint32_t damage)
{
    thingp orig_hitter = hitter;

    if (t == hitter) {
        THING_ERR(t, "Hitting thyself? wth.");
        DIE("hitting thyself");
        return (false);
    }

    /*
     * If an arrow, this might be an elf.
     */
    thingp real_hitter = 0;

    if (hitter) {
        real_hitter = thing_owner(level, hitter);
        if (!real_hitter) {
            real_hitter = hitter;
        }
    }

    if (!hitter) {
        ERR("no hitter");
    }
    tpp weapon = 0;

#if 0
    if (hitter && thing_owner(hitter)) {
        THING_CON(t, "is being hit by %s, owner %s",
                  thing_logname(hitter), 
                  thing_logname(thing_owner(hitter)));
    } else {
        THING_CON(t, "is being hit by %s",
                  thing_logname(hitter));
    }
#endif

    verify(t);
    if (hitter) {
        verify(hitter);
    }

    /*
     * Cruel to let things keep on hitting you when you're dead
     */
    if (thing_is_dead(t)) {
        return (false);
    }

    if (thing_is_dead(hitter)) {
        /*
         * This case is hit if a ghost runs into a player. The ghost takes
         * damage. We don't want the player to keep absorbing hits when
         * already dead though.
         */
        return (false);
    }

    /*
     * If this is a thing on the edge of the level acting as a indestructable
     * wall, then don't allow it to be destroyed.
     */
    if (thing_is_joinable(t)) {
        if ((t->x <= 0) || (t->x >= MAP_WIDTH - 1) ||
            (t->y <= 0) || (t->y >= MAP_HEIGHT - 1)) {
            return (false);
        }
    }

    /*
     * Check to see if this is a thing that can be damaged by the hitter.
     */
    if (hitter) {
        /*
         * Walls and doors and other solid object are not damaged by poison
         * or similar effects. Limit it to explosions and the like.
         */
        if (thing_is_door(t)            ||
            thing_is_rock(t)            ||
            thing_is_wall(t)) {

            if (!thing_is_explosion(hitter)     &&
                !thing_is_projectile(hitter)    &&
                !thing_is_weapon_swing_effect(hitter)) {
                /*
                 * Not something that typically damages walls.
                 */
                return (false);
            }
        }

        if (thing_is_weapon_swing_effect(hitter)) {
            if (!hitter->owner_thing_id) {
                /*
                 * Happens with rapid swings as we only allow one active swing
                 * per owner.
                 *
                ERR("swung weapon %s has no owner ID", thing_logname(hitter));
                 */
                return (false);
            }

            /*
             * Get the player swinging the weapon as the hitter.
             */
            hitter = thing_owner(level, hitter);
            if (!hitter) {
                return (false);
            }

            verify(hitter);

            /*
             * Don't let a sword hit too far away else we end up hitting
             * through walls! Actually I now like the effect. Keep it.
             */
#if 0
            double dist = DISTANCE(hitter->x, hitter->y, t->x, t->y);
            if (dist > 1.0) {
                /*
                 * Too far.
                 */
                return (false);
            }
#endif

            /*
             * Get the damage from the weapon being used to swing.
             */
            weapon = thing_weapon(hitter);
            if (!weapon) {
                return (false);
            }

            if (!damage) {
                damage = tp_get_damage(weapon);
            }

        } else if (hitter->owner_thing_id) {
            /*
             * Get the player firing the weapon as the hitter.
             */
            hitter = thing_owner(level, hitter);
            if (!hitter) {
                return (false);
            }

            verify(hitter);
        }

        /*
         * Don't let our own potion hit ourselves!
         */
        if (hitter == t) {
            return (false);
        }

        /*
         * Don't allow one player's potion effect to kill another player.
         */
        if (thing_is_player(hitter) && thing_is_player(t)) {
            return (false);
        }

        if (orig_hitter &&
            (orig_hitter->tp_id == THING_SILKBALL)) {
            /*
             * No inherited spider damage from silk ball.
             */
        } else {
            /*
             * Take the damage from the weapon that hits first.
             */
            if (!damage) {
                if (orig_hitter) {
                    damage = thing_stats_get_total_damage(orig_hitter);
                }
            }

            /*
             * If still no damage, use the thing that did the hitting.
             */
            if (!damage) {
                if (hitter) {
                    damage = thing_stats_get_total_damage(hitter);
                }
            }
        }
    }

    /*
     * Attack a shopkeeper? Not a wise move...
     */
    if (thing_is_shopkeeper(t)) {
        if (hitter && thing_is_player(hitter)) {
            shop_attack_message(level, hitter);
        }
    }

    /*
     * Allow no more hits than x per second by the hitter.
     */
    if (orig_hitter) {
        /*
         * We want the orig hitter, i.e. the sword and not the playet.
         */
        uint32_t delay =
            tp_get_hit_delay_tenths(thing_tp(orig_hitter));

        if (delay) {
            if (!time_have_x_tenths_passed_since(
                    delay, orig_hitter->timestamp_i_attacked)) {

                return (false);
            }

            orig_hitter->timestamp_i_attacked = time_get_time_ms();
        }

        if (t == orig_hitter) {
            THING_ERR(t, "Hitting thyself? wth.");
            DIE("hitting thyself");
            return (false);
        }
    }

    /*
     * Flash briefly red on attempted hits.
     */
    if (thing_is_monst(t)               ||
        thing_is_mob_spawner(t)         ||
        thing_is_rock(t)                ||
        thing_is_wall(t)                ||
        thing_is_door(t)) {

        /*
         * Assume missed due to the logic below where we detect chance.
         */
        if (orig_hitter && thing_is_cloud_effect(orig_hitter)) {
            /*
             * No flashing
             */
        } else {
            thing_effect_hit_miss(level, t);
        }
    }

    /*
     * Does the thing get off being hit.
     */
    uint32_t can_be_hit_chance = tp_get_can_be_hit_chance(thing_tp(t));
    if (can_be_hit_chance) {
        uint32_t chance = myrand() % (can_be_hit_chance + 1);

        if (chance > damage) {
            return (false);
        }
    }

    int r;

    r = thing_hit_(level, t, orig_hitter, real_hitter, hitter, damage);

    /*
     * Do we need to kill the original hitter?
     */
    if (orig_hitter) {
        if (thing_is_fragile(orig_hitter)) {
            /*
             * Sawblades get more covered in blood each time they kill
             * something that is warm blooded. But we don't want that to
             * happen for damage from say a bomb. However if the damage is
             * really high then we just stop the blade.
             */
            if (thing_is_sawblade(orig_hitter)) {
                if (thing_is_warm_blooded(t)) {
                    /*
                     * Move to the next most bloody blade
                     */
                    thing_dead(level, orig_hitter, 0, "blood splatter");
                } else {
                    /*
                     * Keep on spinning those blades.
                     */
                }
            } else {
                thing_dead(level, orig_hitter, 0, "self destruct on hitting");
            }
        }
    }

    return (r);
}

thingp thing_owner (levelp level, thingp t)
{
    verify(t);

    if (t->owner_thing_id) {
        thingp n = id_to_thing(t->owner_thing_id);
        if (!n) {
            /*
             * for things like missiles, it is accepted that the owner may
             * be dead but the missile keeps on going.
             */
            if (thing_is_projectile(t)) {
                t->owner_thing_id = 0;

                THING_LOG(t, "no thing found for owner id %08X for %s",
                            t->owner_thing_id, thing_logname(t));
                return (0);
            } else {
                ERR("no thing found for owner id %08X for %s",
                    t->owner_thing_id, thing_logname(t));
            }
        }

        verify(n);
        return (n);
    }

    return (0);
}

void thing_hide (levelp level, thingp t)
{
    verify(t);

    widp w = t->wid;
    if (!w) {
        return;
    }

    if (wid_this_is_hidden(w)) {
        return;
    }

    wid_this_hide(w, 0);

    /*
     * Hide the weapon too or it just floats in the air.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(level, t);
    if (weapon_carry_anim) {
        thing_hide(level, weapon_carry_anim);
    }
}

void thing_visible (levelp level, thingp t)
{
    verify(t);

    widp w = t->wid;
    if (!w) {
        return;
    }

    if (!wid_this_is_hidden(w)) {
        return;
    }

    /*
     * If this thing has an owner, should the thing stay hidden?
     */
    thingp owner = thing_owner(level, t);
    if (owner) {
        if (t == thing_weapon_carry_anim(level, owner)) {
            if (thing_weapon_swing_anim(level, owner)) {
                /*
                 * Stay hidden until the weapon swing is done.
                 */
                return;
            }
        }
    }

    /*
     * Reveal the thing.
     */
    wid_this_visible(w, 0);

    /*
     * Reveal the weapon again too.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(level, t);
    if (weapon_carry_anim) {
        thing_visible(level, weapon_carry_anim);
    }
}

uint8_t thing_is_visible (levelp level, thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        return (!wid_is_hidden(w));
    }

    return (false);
}

void thing_leave_level (levelp level, thingp t)
{
    /*
     * Make the weapon leave too
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(level, t);
    if (weapon_carry_anim) {
        thing_leave_level(level, weapon_carry_anim);
    }

    /*
     * Still in a shop? Sneaky.
     */
    t->gold_owed = 0;
    t->in_shop_owned_by_thing_id = 0;

    thing_map_remove(level, t);
    thing_set_wid(level, t, 0);
}

void thing_join_level (levelp level, thingp t)
{
    /*
     * Make the weapon leave to
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(level, t);
    if (weapon_carry_anim) {
        thing_join_level(level, weapon_carry_anim);
    }
}

int thing_is_player_or_owned_by_player (levelp level, thingp t)
{
    if (thing_is_player(t)) {
        return (true);
    }

    /*
     * Keep owned weapons too.
     */
    thingp owner = thing_owner(level, t);
    if (owner) {
        if (thing_is_player(owner)) {
            return (true);
        }
    }

    return (false);
}

void things_level_destroyed (levelp level, uint8_t keep_player)
{
    thingp t;

    /*
     * Ensure no stale pointers.
     */
    {
        {
            if (keep_player) {
                LEVEL_LOG(level, "Destroy things, keep player");
            } else {
                LEVEL_LOG(level, "Destroy things and player");
            }

            FOR_ALL_THINGS(level, t)

                if (keep_player &&
                    thing_is_player_or_owned_by_player(level, t) &&
                    !thing_is_animation(t) &&
                    !thing_is_weapon_carry_anim(t) &&
                    !thing_is_weapon_swing_effect(t)) {

                    thing_map_remove(level, t);
                    thing_set_wid(level, t, 0);
                    continue;
                }

                thing_destroy(level, t, "level destroyed");

            FOR_ALL_THINGS_END
        }

        {
            FOR_ALL_THINGS(level, t)

                if (keep_player &&
                    thing_is_player_or_owned_by_player(level, t)) {
                    continue;
                }

                ERR("thing still exists %s", thing_logname(t));

            FOR_ALL_THINGS_END
        }
    }
}

const char *thing_logname (thingp t)
{
    /*
     * Return constant strings from a small pool.
     */
    static char tmp[10][MAXSTR];
    static int loop;

    verify(t);

    if (loop >= 10) {
        loop = 0;
    }

    snprintf(tmp[loop], sizeof(tmp[loop]) - 1,
             "%s[%p, id %08X]", thing_short_name(t), t, t->thing_id);

    return (tmp[loop++]);
}

const char *thing_short_name (thingp t)
{
    verify(t);

    return (tp_short_name(thing_tp(t)));
}

uint8_t thing_z_depth (thingp t)
{
    verify(t);

    return (tp_get_z_depth(thing_tp(t)));
}

uint8_t thing_z_order (thingp t)
{
    verify(t);

    return (tp_get_z_order(thing_tp(t)));
}

tree_rootp thing_tile_tiles (thingp t)
{
    verify(t);

    return (tp_get_tiles(thing_tp(t)));
}

widp thing_wid (thingp t)
{
    verify(t);

    return (t->wid);
}

void thing_set_wid (levelp level, thingp t, widp w)
{
    verify(t);

    if (w) {
        verify(w);
    } else {
        if (t->wid) {
            verify(t->wid);
            wid_set_thing(t->wid, 0, 0);
            wid_fade_out(t->wid, 100);
            wid_destroy_in(t->wid, 100);
        }
    }

    t->wid = w;
}

uint8_t thing_is_light_source (thingp t)
{
    verify(t);

    return (thing_tp(t)->is_light_source);
}

void thing_set_is_candle_light (thingp t, uint8_t val)
{
    verify(t);

    thing_tp(t)->is_candle_light = val;
}

uint8_t thing_is_candle_light (thingp t)
{
    verify(t);

    return (thing_tp(t)->is_candle_light);
}

void thing_set_is_sleeping (thingp t, uint8_t val)
{
    verify(t);

    t->is_sleeping = val;
}

void thing_set_is_collected (thingp t, uint8_t val)
{
    verify(t);

    t->is_collected = val;
}

uint8_t thing_is_collected (thingp t)
{
    verify(t);

    return (t->is_collected);
}

void thing_set_dir_none (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_NONE) {
        t->dir = THING_DIR_NONE;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_none (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_NONE);
}

void thing_set_dir_down (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_DOWN) {
        t->dir = THING_DIR_DOWN;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_down (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_DOWN);
}

void thing_set_dir_up (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_UP) {
        t->dir = THING_DIR_UP;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_up (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_UP);
}

void thing_set_dir_left (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_LEFT) {
        t->dir = THING_DIR_LEFT;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_left (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_LEFT);
}

void thing_set_dir_right (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_RIGHT) {
        t->dir = THING_DIR_RIGHT;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_right (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_RIGHT);
}

void thing_set_dir_tl (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_TL) {
        t->dir = THING_DIR_TL;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_tl (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_TL);
}

void thing_set_dir_bl (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_BL) {
        t->dir = THING_DIR_BL;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_bl (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_BL);
}

void thing_set_dir_tr (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_TR) {
        t->dir = THING_DIR_TR;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_tr (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_TR);
}

void thing_set_dir_br (levelp level, thingp t)
{
    verify(t);

    if (thing_is_animated_no_dir(t)) {
        return;
    }

    if (t->dir != THING_DIR_BR) {
        t->dir = THING_DIR_BR;
        thing_set_weapon_placement(level, t);
    }
}

uint8_t thing_is_dir_br (thingp t)
{
    verify(t);

    return (t->dir == THING_DIR_BR);
}

void thing_set_is_open (thingp t, uint8_t val)
{
    verify(t);

    t->timestamp_change_to_next_frame = time_get_time_ms();

    t->is_open = val;
}

void thing_set_is_angry (thingp t, uint8_t val)
{
    verify(t);

    t->timestamp_change_to_next_frame = time_get_time_ms();

    t->is_angry = val;
}

void thing_set_is_dead (thingp t, uint8_t val)
{
    verify(t);

    t->is_dead = val;
}

const char *thing_name (thingp t)
{
    verify(t);

    return (tp_short_name(thing_tp(t)));
}

const char * thing_tooltip (thingp t)
{
    verify(t);

    return (tp_get_tooltip(thing_tp(t)));
}

tree_rootp thing_tiles (thingp t)
{
    verify(t);

    return (tp_get_tiles(thing_tp(t)));
}

thing_tilep thing_current_tile (thingp t)
{
    verify(t);

    if (!t->current_tile) {
        return (0);
    }

    return (&thing_tile_arr[t->current_tile]);
}

void thing_teleport (levelp level, thingp t, int32_t x, int32_t y)
{
    if (time_get_time_ms() - t->timestamp_teleport < 500) {
        return;
    }

    t->timestamp_teleport = time_get_time_ms();

    widp wid_next_floor = wid_grid_find_thing_template(
                                    game.wid_grid,
                                    x,
                                    y,
                                    tp_is_dungeon_floor);
    if (!wid_next_floor) {
        ERR("no floor tile to hpp to");
    }

    double next_floor_x, next_floor_y;

    wid_get_mxy(wid_next_floor, &next_floor_x, &next_floor_y);

    widp w = thing_wid(t);

    wid_move_end(w);
    wid_move_to_abs_centered_in(w, next_floor_x, next_floor_y, 0);

    sound_play_level_end();
}

void thing_move_to (thingp t, double x, double y)
{
    verify(t);

    if ((t->last_x == -1.0) && (t->last_y == -1.0)) {
        t->last_x = x;
        t->last_y = y;
    } else {
        t->last_x = t->x;
        t->last_y = t->y;
    }

    t->x = x;
    t->y = y;
}

void thing_move_set_dir (levelp level,
                         thingp t,
                         double *x,
                         double *y,
                         uint8_t up,
                         uint8_t down,
                         uint8_t left,
                         uint8_t right)
{
    /*
     * Projectiles like the fireball anim have their direction set when 
     * created and don't need upating; and if we do we get the angles not 
     * quite right
     */
    if (thing_is_projectile(t)) {
        return;
    }

    double ox = t->x;
    double oy = t->y;

    if (*x < 0) {
        *x = 0;
    }

    if (*y < 0) {
        *y = 0;
    }

    if (*x > MAP_WIDTH - 1) {
        *x = MAP_WIDTH - 1;
    }

    if (*y > MAP_HEIGHT - 1) {
        *y = MAP_HEIGHT - 1;
    }

    if (*x > ox) {
        right = true;
    }

    if (*x < ox) {
        left = true;
    }

    if (*y > oy) {
        down = true;
    }

    if (*y < oy) {
        up = true;
    }

    if (up) {
        if (left) {
            thing_set_dir_tl(level, t);
        } else if (right) {
            thing_set_dir_tr(level, t);
        } else {
            thing_set_dir_up(level, t);
        }
    } else if (down) {
        if (left) {
            thing_set_dir_bl(level, t);
        } else if (right) {
            thing_set_dir_br(level, t);
        } else {
            thing_set_dir_down(level, t);
        }
    } else if (left) {
        thing_set_dir_left(level, t);
    } else if (right) {
        thing_set_dir_right(level, t);
    }
}

void thing_set_owner_id (levelp level,
                         thingp t, uint32_t owner_id)
{
    thingp owner;

    if (!owner_id) {
        thing_set_owner(level, t, 0);
        return;
    }

    owner = id_to_thing(owner_id);

    thing_set_owner(level, t, owner);
}

void thing_set_owner (levelp level,
                      thingp t, thingp owner)
{
    if (owner) {
        verify(owner);
    }

    thingp old_owner = thing_owner(level, t);
    if (old_owner) {
        if (old_owner == owner) {
            return;
        }

        if (owner) {
            THING_LOG(t, "Owner change %s->%s",
                      thing_logname(old_owner), thing_logname(owner));
        } else {
#ifdef THING_DEBUG
            THING_LOG(t, "remove owner %s", thing_logname(old_owner));
#endif
        }
    } else {
        if (owner) {
#ifdef THING_DEBUG
            THING_LOG(t, "owner %s", thing_logname(owner));
#endif
        }
    }

    if (owner) {
        t->owner_thing_id = owner->thing_id;

        owner->owned_count++;
    } else {
        t->owner_thing_id = 0;

        if (old_owner) {
            old_owner->owned_count--;
        }
    }
}
