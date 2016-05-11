/*
 * Copyright (C) 2011-2017 goblinhack@gmail.com
 *
 * See the LICENSE file.
 */

#pragma once

#include "tree.h"

#define IS_JOIN_ALT_MAX 20

enum {
    IS_JOIN_BLOCK,
    IS_JOIN_HORIZ,
    IS_JOIN_VERT,
    IS_JOIN_NODE,
    IS_JOIN_LEFT,
    IS_JOIN_RIGHT,
    IS_JOIN_TOP,
    IS_JOIN_BOT,
    IS_JOIN_TL,
    IS_JOIN_TR,
    IS_JOIN_BL,
    IS_JOIN_BR,
    IS_JOIN_T,
    IS_JOIN_T90,
    IS_JOIN_T180,
    IS_JOIN_T270,
    IS_JOIN_X,
    IS_JOIN_TL2,
    IS_JOIN_TR2,
    IS_JOIN_BL2,
    IS_JOIN_BR2,
    IS_JOIN_T_1,
    IS_JOIN_T_2,
    IS_JOIN_T_3,
    IS_JOIN_T90_1,
    IS_JOIN_T90_2,
    IS_JOIN_T90_3,
    IS_JOIN_T180_1,
    IS_JOIN_T180_2,
    IS_JOIN_T180_3,
    IS_JOIN_T270_1,
    IS_JOIN_T270_2,
    IS_JOIN_T270_3,
    IS_JOIN_X1,
    IS_JOIN_X1_270,
    IS_JOIN_X1_180,
    IS_JOIN_X1_90,
    IS_JOIN_X2,
    IS_JOIN_X2_270,
    IS_JOIN_X2_180,
    IS_JOIN_X2_90,
    IS_JOIN_X3,
    IS_JOIN_X3_180,
    IS_JOIN_X4,
    IS_JOIN_X4_270,
    IS_JOIN_X4_180,
    IS_JOIN_X4_90,
    IS_JOIN_VERT2,
    IS_JOIN_HORIZ2,
    IS_JOIN_MAX,
};

typedef struct tp_ {
    tree_key_string tree;
    tree_key_int tree2;

    uint16_t tp_id;

    /*
     * Internal description of the thing.
     */
    char *short_name;

    /*
     * Base filename
     */
    char *raw_name;

    /*
     * End user description of the thing.
     */
    char *tooltip;

    /*
     * What the thing throws.
     */
    tpp fires;

    /*
     * What to create when you die. Usually a smaller monster.
     */
    char *polymorph_on_death;

    /*
     * When you collect a bundle of items it as carried as x of this.
     */
    char *carried_as;

    /*
     * What shade of light does this thing emit
     */
    char *light_tint;
    color light_color;

    /*
     * For potions and monster death.
     */
    char *explodes_as;

    /*
     * Sound on thing creation
     */
    char *sound_on_creation;

    /*
     * Sound on hitting a thing
     */
    char *sound_on_hitting_something;

    /*
     * Sound on thing death
     */
    char *sound_on_death;

    /*
     * Sound on thing hit
     */
    char *sound_on_hit;

    /*
     * Sound on thing collect
     */
    char *sound_on_collect;

    /*
     * Occasional grunts
     */
    char *sound_random;

    /*
     * How this weapon appears on screen when it is used.
     */
    char *weapon_carry_anim;
    char *weapon_swing_anim;

    /*
     * What to shout when an item is used.
     */
    char *message_on_use;

    /*
     * Periodically spawn something.
     */
#define MAX_MOB_SPAWN 16
    char *mob_spawn_what[MAX_MOB_SPAWN];
    int mob_spawn_chance_d1000[MAX_MOB_SPAWN];
    int mob_spawn_count;

    /*
     * What to spawn when you die. Usually a generator or grave.
     */
    char *spawn_on_death_what[MAX_MOB_SPAWN];
    int spawn_on_death_chance_d1000[MAX_MOB_SPAWN];
    int spawn_on_death_count;

    uint32_t d10000_chance_of_appearing;

    /*
     * In relation to other widgets, where are we.
     */
    uint8_t z_depth;
    uint8_t z_order;

    /*
     * Animation tiles.
     */
    tree_rootp tiles;

    /*
     * Speed in milliseconds it takes to move one tile.
     */
    uint16_t speed;

    /*
     * Damage on hits.
     */
    uint16_t damage;

    /*
     * Cost in shops
     */
    uint16_t cost;

    /*
     * Lifespan in milliseconds.
     */
    uint16_t lifespan_ticks;

    /*
     * How far in tiles the thing can detect the monster.
     */
    uint8_t vision_distance;

    /*
     * How close a thing tries to get to you
     */
    uint8_t approach_distance;

    /*
     * Various bounties.
     */
    int32_t bonus_score_on_death;
    int32_t bonus_gold_on_collect;
    int16_t bonus_hp_on_collect;

    int32_t blit_top_off;
    int32_t blit_bot_off;
    int32_t blit_left_off;
    int32_t blit_right_off;

    uint32_t ppp6;
    uint32_t ppp7;
    uint32_t min_appear_depth;
    uint32_t max_appear_depth;
    uint32_t jump_speed;
    uint32_t hp_per_level;
    uint32_t max_hp;

    /*
     * How much light it gives off in tiles.
     */
    float light_radius;

    /*
     * For multi fire weapons
     */
    float weapon_density;
    float weapon_spread;

    /*
     * How large or small the thing is blitted as.
     */
    float scale;

    /*
     * How large an explosion is.
     */
    float explosion_radius;

    /*
     * How close for collision detection.
     */
    float collision_radius;

    uint32_t hit_priority;
    uint32_t weapon_fire_delay_hundredths;
    uint32_t sound_random_delay_secs;
    uint32_t swing_distance_from_player;
    uint32_t can_be_hit_chance;
    uint32_t hit_delay_tenths;
    uint32_t mob_spawn_delay_tenths;

    uint8_t can_be_enchanted:1;
    uint8_t can_fall:1;
    uint8_t can_walk_through:1;
    uint8_t has_dir_anim:1;
    uint8_t is_acid:1;
    uint8_t is_acid_proof:1;
    uint8_t is_action:1;
    uint8_t is_action_down:1;
    uint8_t is_action_left:1;
    uint8_t is_action_right:1;
    uint8_t is_action_sleep:1;
    uint8_t is_action_trigger:1;
    uint8_t is_action_trigger_on_hero:1;
    uint8_t is_action_trigger_on_monst:1;
    uint8_t is_action_trigger_on_wall:1;
    uint8_t is_action_up:1;
    uint8_t is_action_zap:1;
    uint8_t is_animated:1;
    uint8_t is_animated_no_dir:1;
    uint8_t is_animated_only_when_moving:1;
    uint8_t is_animation:1;
    uint8_t is_bomb:1;
    uint8_t is_bullet:1;
    uint8_t is_candle_light:1;
    uint8_t is_carryable:1;
    uint8_t is_cats_eyes:1;
    uint8_t is_cloud_effect:1;
    uint8_t is_cobweb:1;
    uint8_t is_collision_map_large:1;
    uint8_t is_collision_map_medium:1;
    uint8_t is_collision_map_small:1;
    uint8_t is_collision_map_tiny:1;
    uint8_t is_combustable:1;
    uint8_t is_conical_breath_attack:1;
    uint8_t is_corpse:1;
    uint8_t is_corridor:1;
    uint8_t is_corridor_wall:1;
    uint8_t is_door:1;
    uint8_t is_dungeon_floor:1;
    uint8_t is_effect_fade_in_out:1;
    uint8_t is_effect_pulse:1;
    uint8_t is_effect_rotate_2way:1;
    uint8_t is_effect_sway:1;
    uint8_t is_entrance:1;
    uint8_t is_ethereal:1;
    uint8_t is_exit:1;
    uint8_t is_explosion:1;
    uint8_t is_fire:1;
    uint8_t is_fireball:1;
    uint8_t is_fires_when_angry:1;
    uint8_t is_food:1;
    uint8_t is_fragile:1;
    uint8_t is_given_randomly_at_start:1;
    uint8_t is_hard:1;
    uint8_t is_hidden:1;
    uint8_t is_hidden_from_editor:1;
    uint8_t is_inactive:1;
    uint8_t is_internal:1;
    uint8_t is_item_unusable:1;
    uint8_t is_joinable:1;
    uint8_t is_key:1;
    uint8_t is_ladder:1;
    uint8_t is_ladder_deco:1;
    uint8_t is_lava:1;
    uint8_t is_lava_proof:1;
    uint8_t is_levitating:1;
    uint8_t is_life_saving:1;
    uint8_t is_light_source:1;
    uint8_t is_magical_weapon:1;
    uint8_t is_melee_weapon:1;
    uint8_t is_mob_spawner:1;
    uint8_t is_monst:1;
    uint8_t is_non_explosive_gas_cloud:1;
    uint8_t is_not_light_blocking:1;
    uint8_t is_obstacle:1;
    uint8_t is_player:1;
    uint8_t is_projectile:1;
    uint8_t is_ranged_weapon:1;
    uint8_t is_rock:1;
    uint8_t is_rrr1:1;
    uint8_t is_rrr2:1;
    uint8_t is_rrr3:1;
    uint8_t is_rrr4:1;
    uint8_t is_rrr5:1;
    uint8_t is_rrr6:1;
    uint8_t is_rrr7:1;
    uint8_t is_rrr8:1;
    uint8_t is_rrr9:1;
    uint8_t is_rrr10:1;
    uint8_t is_rrr11:1;
    uint8_t is_rrr12:1;
    uint8_t is_rrr13:1;
    uint8_t is_rrr14:1;
    uint8_t is_rrr15:1;
    uint8_t is_rrr16:1;
    uint8_t is_rrr17:1;
    uint8_t is_rrr18:1;
    uint8_t is_rrr19:1;
    uint8_t is_rrr20:1;
    uint8_t is_rrr21:1;
    uint8_t is_rrr22:1;
    uint8_t is_rrr23:1;
    uint8_t is_rrr24:1;
    uint8_t is_rrr25:1;
    uint8_t is_rrr26:1;
    uint8_t is_rrr27:1;
    uint8_t is_bubbles:1;
    uint8_t can_drown:1;
    uint8_t is_ropepile:1;
    uint8_t is_ropetop:1;
    uint8_t is_climbable:1;
    uint8_t is_smallrock:1;
    uint8_t is_rope:1;
    uint8_t is_throwable:1;
    uint8_t is_sawblade:1;
    uint8_t is_shadow_caster:1;
    uint8_t is_shadow_caster_soft:1;
    uint8_t is_shop_floor:1;
    uint8_t is_shopkeeper:1;
    uint8_t is_single_mob_spawner:1;
    uint8_t is_sleeping:1;
    uint8_t is_spawns_under:1;
    uint8_t is_spider_proof:1;
    uint8_t is_spikes:1;
    uint8_t is_stackable:1;
    uint8_t is_teleport:1;
    uint8_t is_torch:1;
    uint8_t is_trap:1;
    uint8_t is_treasure:1;
    uint8_t is_undead:1;
    uint8_t is_variable_size:1;
    uint8_t is_visible_on_debug_only:1;
    uint8_t is_wall:1;
    uint8_t is_wall_deco:1;
    uint8_t is_wanderer:1;
    uint8_t is_warm_blooded:1;
    uint8_t is_water:1;
    uint8_t is_water_proof:1;
    uint8_t is_weapon:1;
    uint8_t is_weapon_carry_anim:1;
    uint8_t is_weapon_swing_effect:1;

    thing_tilep tilep_join[IS_JOIN_MAX][IS_JOIN_ALT_MAX];
    tilep tilep_join_tile[IS_JOIN_MAX][IS_JOIN_ALT_MAX];
    uint8_t tilep_join_count[IS_JOIN_MAX];
} thing_template;

typedef struct tpp_data_ {
    /*
     * Color of the thing, mainly used in triggers.
     */
    color col;
} thing_template_data;

uint8_t tp_init(void);
void tp_fini(void);
tpp tp_load(uint16_t id, const char *file);
tpp tp_find(const char *name);
tpp tp_find_short_name(const char *name);
void demarshal_thing_template(demarshal_p ctx, tpp);
void marshal_thing_template(marshal_p ctx, tpp);
void thing_templates_marshal(marshal_p out);
uint8_t thing_test(int32_t argc, char *argv[]);
tpp string2thing_template(const char **s);

const char *tp_name(tpp);
const char *tp_short_name(tpp);
const char *tp_raw_name(tpp);
tpp tp_fires(tpp);
const char *tp_polymorph_on_death(tpp);
const char *tp_carried_as(tpp);
const char *tp_light_tint(tpp);
color tp_light_color(tpp);
const char *tp_spawn_on_death(tpp);
const char *tp_explodes_as(tpp);
const char *tp_sound_on_creation(tpp);
const char *tp_sound_on_hitting_something(tpp);
const char *tp_sound_on_death(tpp);
const char *tp_sound_on_hit(tpp);
const char *tp_sound_on_collect(tpp);
const char *tp_weapon_carry_anim(tpp);
const char *tp_shield_carry_anim(tpp);
const char *tp_weapon_swing_anim(tpp);
const char *tp_message_on_use(tpp);
const char *tp_mob_spawn(tpp);
const char *tp_get_tooltip(tpp);
uint8_t tp_get_z_depth(tpp);
uint8_t tp_get_z_order(tpp);
uint32_t tp_get_speed(tpp);
uint32_t tp_get_lifespan_ticks(tpp);
int32_t tp_get_bonus_score_on_death(tpp t);
uint16_t tp_get_damage(tpp);
uint16_t tp_get_cost(tpp);
uint32_t tp_get_vision_distance(tpp);
uint32_t tp_get_approach_distance(tpp);
int32_t tp_get_bonus_gold_on_collect(tpp);
int32_t tp_get_blit_top_off(tpp);
int32_t tp_get_blit_bot_off(tpp);
int32_t tp_get_blit_left_off(tpp);
int32_t tp_get_blit_right_off(tpp);
uint32_t tp_get_ppp6(tpp);
uint32_t tp_get_ppp7(tpp);
uint32_t tp_get_min_appear_depth(tpp);
uint32_t tp_get_max_appear_depth(tpp);
uint32_t tp_get_jump_speed(tpp);
double tp_get_light_radius(tpp);
double tp_get_weapon_density(tpp);
double tp_get_weapon_spread(tpp);
double tp_get_scale(tpp);
double tp_get_explosion_radius(tpp);
double tp_get_collision_radius(tpp);
uint32_t tp_get_quantity(tpp);
uint32_t tp_get_hit_priority(tpp);
uint32_t tp_get_weapon_fire_delay_hundredths(tpp);
uint32_t tp_get_sound_random_delay_secs(tpp);
uint32_t tp_get_swing_distance_from_player(tpp);
int16_t tp_get_stats_max_hp(tpp);
uint32_t tp_get_hp_per_level(tpp);
int16_t tp_get_bonus_hp_on_collect(tpp);

uint32_t tp_get_tx_map_update_delay_thousandths(tpp);
uint32_t tp_get_can_be_hit_chance(tpp);
uint32_t tp_get_d10000_chance_of_appearing(tpp);
uint32_t tp_get_hit_delay_tenths(tpp);
uint32_t tp_get_mob_spawn_delay_tenths(tpp);

tree_rootp tp_get_tiles(tpp);

extern tree_rootp thing_templates;
extern tree_rootp thing_templates_create_order;

static inline int16_t tp_to_id (tpp t) 
{
    return (t->tp_id);
}

static inline tpp id_to_tp (uint16_t id) 
{
    if (!id) {
        return (0);
    }

    extern thing_template thing_templates_chunk[THING_MAX];

    if (id >= THING_MAX) {
        DIE("overflow, bad template id %08X", id);
    }

    return (&thing_templates_chunk[id]);
}

static inline uint8_t tp_is_exit (tpp t)
{
    return (t->is_exit);
}

static inline uint8_t tp_is_dungeon_floor (tpp t)
{
    return (t->is_dungeon_floor);
}

static inline uint8_t tp_is_food (tpp t)
{
    return (t->is_food);
}

static inline uint8_t tp_is_monst (tpp t)
{
    return (t->is_monst);
}

static inline uint8_t tp_is_player (tpp t)
{
    return (t->is_player);
}

static inline uint8_t tp_is_key (tpp t)
{
    return (t->is_key);
}

static inline uint8_t tp_is_rock (tpp t)
{
    return (t->is_rock);
}

static inline uint8_t tp_is_shadow_caster (tpp t)
{
    return (t->is_shadow_caster);
}

static inline uint8_t tp_is_weapon (tpp t)
{
    return (t->is_weapon);
}

static inline uint8_t tp_is_treasure (tpp t)
{
    return (t->is_treasure);
}

static inline uint8_t tp_is_fragile (tpp t)
{
    return (t->is_fragile);
}

static inline uint8_t tp_is_animated_no_dir (tpp t)
{
    return (t->is_animated_no_dir);
}

static inline uint8_t tp_is_weapon_swing_effect (tpp t)
{
    return (t->is_weapon_swing_effect);
}

static inline uint8_t tp_is_light_source (tpp t)
{
    return (t->is_light_source);
}

static inline uint8_t tp_is_candle_light (tpp t)
{
    return (t->is_candle_light);
}

static inline uint8_t tp_is_cats_eyes (tpp t)
{
    return (t->is_cats_eyes);
}

static inline uint8_t tp_is_fire (tpp t)
{
    return (t->is_fire);
}

static inline uint8_t tp_is_animation (tpp t)
{
    return (t->is_animation);
}

static inline uint8_t tp_is_shadow_caster_soft (tpp t)
{
    return (t->is_shadow_caster_soft);
}

static inline uint8_t tp_is_collision_map_large (tpp t)
{
    return (t->is_collision_map_large);
}

static inline uint8_t tp_is_non_explosive_gas_cloud (tpp t)
{
    return (t->is_non_explosive_gas_cloud);
}

static inline uint8_t tp_is_item_unusable (tpp t)
{
    return (t->is_item_unusable);
}

static inline uint8_t tp_is_carryable (tpp t)
{
    return (t->is_carryable);
}

static inline uint8_t tp_is_door (tpp t)
{
    return (t->is_door);
}

static inline uint8_t tp_is_mob_spawner (tpp t)
{
    return (t->is_mob_spawner);
}

static inline uint8_t tp_is_single_mob_spawner (tpp t)
{
    return (t->is_single_mob_spawner);
}

static inline uint8_t tp_is_rrr1 (tpp t)
{
    return (t->is_rrr1);
}

static inline uint8_t tp_is_rrr2 (tpp t)
{
    return (t->is_rrr2);
}

static inline uint8_t tp_is_rrr3 (tpp t)
{
    return (t->is_rrr3);
}

static inline uint8_t tp_is_rrr4 (tpp t)
{
    return (t->is_rrr4);
}

static inline uint8_t tp_is_rrr5 (tpp t)
{
    return (t->is_rrr5);
}

static inline uint8_t tp_is_rrr6 (tpp t)
{
    return (t->is_rrr6);
}

static inline uint8_t tp_is_rrr7 (tpp t)
{
    return (t->is_rrr7);
}

static inline uint8_t tp_is_rrr8 (tpp t)
{
    return (t->is_rrr8);
}

static inline uint8_t tp_is_rrr9 (tpp t)
{
    return (t->is_rrr9);
}

static inline uint8_t tp_is_rrr10 (tpp t)
{
    return (t->is_rrr10);
}

static inline uint8_t tp_is_rrr11 (tpp t)
{
    return (t->is_rrr11);
}

static inline uint8_t tp_is_rrr12 (tpp t)
{
    return (t->is_rrr12);
}

static inline uint8_t tp_is_rrr13 (tpp t)
{
    return (t->is_rrr13);
}

static inline uint8_t tp_is_rrr14 (tpp t)
{
    return (t->is_rrr14);
}

static inline uint8_t tp_is_rrr15 (tpp t)
{
    return (t->is_rrr15);
}

static inline uint8_t tp_is_rrr16 (tpp t)
{
    return (t->is_rrr16);
}

static inline uint8_t tp_is_rrr17 (tpp t)
{
    return (t->is_rrr17);
}

static inline uint8_t tp_is_rrr18 (tpp t)
{
    return (t->is_rrr18);
}

static inline uint8_t tp_is_rrr19 (tpp t)
{
    return (t->is_rrr19);
}

static inline uint8_t tp_is_rrr20 (tpp t)
{
    return (t->is_rrr20);
}

static inline uint8_t tp_is_rrr21 (tpp t)
{
    return (t->is_rrr21);
}

static inline uint8_t tp_is_rrr22 (tpp t)
{
    return (t->is_rrr22);
}

static inline uint8_t tp_is_rrr23 (tpp t)
{
    return (t->is_rrr23);
}

static inline uint8_t tp_is_rrr24 (tpp t)
{
    return (t->is_rrr24);
}

static inline uint8_t tp_is_rrr25 (tpp t)
{
    return (t->is_rrr25);
}

static inline uint8_t tp_is_rrr26 (tpp t)
{
    return (t->is_rrr26);
}

static inline uint8_t tp_is_rrr27 (tpp t)
{
    return (t->is_rrr27);
}

static inline uint8_t tp_is_bubbles (tpp t)
{
    return (t->is_bubbles);
}

static inline uint8_t tp_can_drown (tpp t)
{
    return (t->can_drown);
}

static inline uint8_t tp_is_ropepile (tpp t)
{
    return (t->is_ropepile);
}

static inline uint8_t tp_is_ropetop (tpp t)
{
    return (t->is_ropetop);
}

static inline uint8_t tp_is_climbable (tpp t)
{
    return (t->is_climbable);
}

static inline uint8_t tp_is_smallrock (tpp t)
{
    return (t->is_smallrock);
}

static inline uint8_t tp_is_rope (tpp t)
{
    return (t->is_rope);
}

static inline uint8_t tp_is_throwable (tpp t)
{
    return (t->is_throwable);
}

static inline uint8_t tp_is_not_light_blocking (tpp t)
{
    return (t->is_not_light_blocking);
}

static inline uint8_t tp_is_spikes (tpp t)
{
    return (t->is_spikes);
}

static inline uint8_t tp_is_obstacle (tpp t)
{
    return (t->is_obstacle);
}

static inline uint8_t tp_is_ladder (tpp t)
{
    return (t->is_ladder);
}

static inline uint8_t tp_is_ladder_deco (tpp t)
{
    return (t->is_ladder_deco);
}

static inline uint8_t tp_is_entrance (tpp t)
{
    return (t->is_entrance);
}

static inline uint8_t tp_is_wall_deco (tpp t)
{
    return (t->is_wall_deco);
}

static inline uint8_t tp_is_conical_breath_attack (tpp t)
{
    return (t->is_conical_breath_attack);
}

static inline uint8_t tp_is_corpse (tpp t)
{
    return (t->is_corpse);
}

static inline uint8_t tp_is_spawns_under (tpp t)
{
    return (t->is_spawns_under);
}

static inline uint8_t tp_is_wanderer (tpp t)
{
    return (t->is_wanderer);
}

static inline uint8_t tp_can_fall (tpp t)
{
    return (t->can_fall);
}

static inline uint8_t tp_is_life_saving (tpp t)
{
    return (t->is_life_saving);
}

static inline uint8_t tp_is_water (tpp t)
{
    return (t->is_water);
}

static inline uint8_t tp_is_undead (tpp t)
{
    return (t->is_undead);
}

static inline uint8_t tp_is_hidden (tpp t)
{
    return (t->is_hidden);
}

static inline uint8_t tp_is_trap (tpp t)
{
    return (t->is_trap);
}

static inline uint8_t tp_is_corridor_wall (tpp t)
{
    return (t->is_corridor_wall);
}

static inline uint8_t tp_is_corridor (tpp t)
{
    return (t->is_corridor);
}

static inline uint8_t tp_is_water_proof (tpp t)
{
    return (t->is_water_proof);
}

static inline uint8_t tp_is_lava_proof (tpp t)
{
    return (t->is_lava_proof);
}

static inline uint8_t tp_is_spider_proof (tpp t)
{
    return (t->is_spider_proof);
}

static inline uint8_t tp_is_acid_proof (tpp t)
{
    return (t->is_acid_proof);
}

static inline uint8_t tp_is_fires_when_angry (tpp t)
{
    return (t->is_fires_when_angry);
}

static inline uint8_t tp_is_shopkeeper (tpp t)
{
    return (t->is_shopkeeper);
}

static inline uint8_t tp_is_shop_floor (tpp t)
{
    return (t->is_shop_floor);
}

static inline uint8_t tp_is_fireball (tpp t)
{
    return (t->is_fireball);
}

static inline uint8_t tp_is_bullet (tpp t)
{
    return (t->is_bullet);
}

static inline uint8_t tp_is_effect_fade_in_out (tpp t)
{
    return (t->is_effect_fade_in_out);
}

static inline uint8_t tp_is_internal (tpp t)
{
    return (t->is_internal);
}

static inline uint8_t tp_is_levitating (tpp t)
{
    return (t->is_levitating);
}

static inline uint8_t tp_is_acid (tpp t)
{
    return (t->is_acid);
}

static inline uint8_t tp_is_lava (tpp t)
{
    return (t->is_lava);
}

static inline uint8_t tp_is_teleport (tpp t)
{
    return (t->is_teleport);
}

static inline uint8_t tp_is_cobweb (tpp t)
{
    return (t->is_cobweb);
}

static inline uint8_t tp_is_ethereal (tpp t)
{
    return (t->is_ethereal);
}

static inline uint8_t tp_is_variable_size (tpp t)
{
    return (t->is_variable_size);
}

static inline uint8_t tp_is_magical_weapon (tpp t)
{
    return (t->is_magical_weapon);
}

static inline uint8_t tp_is_ranged_weapon (tpp t)
{
    return (t->is_ranged_weapon);
}

static inline uint8_t tp_is_melee_weapon (tpp t)
{
    return (t->is_melee_weapon);
}

static inline uint8_t tp_is_cloud_effect (tpp t)
{
    return (t->is_cloud_effect);
}

static inline uint8_t tp_is_hard (tpp t)
{
    return (t->is_hard);
}

static inline uint8_t tp_is_sleeping (tpp t)
{
    return (t->is_sleeping);
}

static inline uint8_t tp_is_bomb (tpp t)
{
    return (t->is_bomb);
}

static inline uint8_t tp_is_sawblade (tpp t)
{
    return (t->is_sawblade);
}

static inline uint8_t tp_is_visible_on_debug_only (tpp t)
{
    return (t->is_visible_on_debug_only);
}

static inline uint8_t tp_is_action (tpp t)
{
    return (t->is_action);
}

static inline uint8_t tp_is_action_sleep (tpp t)
{
    return (t->is_action_sleep);
}

static inline uint8_t tp_is_action_zap (tpp t)
{
    return (t->is_action_zap);
}

static inline uint8_t tp_is_action_trigger (tpp t)
{
    return (t->is_action_trigger);
}

static inline uint8_t tp_is_action_trigger_on_wall (tpp t)
{
    return (t->is_action_trigger_on_wall);
}

static inline uint8_t tp_is_action_trigger_on_hero (tpp t)
{
    return (t->is_action_trigger_on_hero);
}

static inline uint8_t tp_is_action_trigger_on_monst (tpp t)
{
    return (t->is_action_trigger_on_monst);
}

static inline uint8_t tp_is_action_down (tpp t)
{
    return (t->is_action_down);
}

static inline uint8_t tp_is_action_up (tpp t)
{
    return (t->is_action_up);
}

static inline uint8_t tp_is_action_left (tpp t)
{
    return (t->is_action_left);
}

static inline uint8_t tp_is_action_right (tpp t)
{
    return (t->is_action_right);
}

static inline uint8_t tp_can_walk_through (tpp t)
{
    return (t->can_walk_through);
}

static inline uint8_t tp_is_weapon_carry_anim (tpp t)
{
    return (t->is_weapon_carry_anim);
}

static inline uint8_t tp_is_animated_only_when_moving (tpp t)
{
    return (t->is_animated_only_when_moving);
}

static inline uint8_t tp_is_warm_blooded (tpp t)
{
    return (t->is_warm_blooded);
}

static inline uint8_t tp_can_be_enchanted (tpp t)
{
    return (t->can_be_enchanted);
}

static inline uint8_t tp_is_stackable (tpp t)
{
    return (t->is_stackable);
}

static inline uint8_t tp_is_torch (tpp t)
{
    return (t->is_torch);
}

static inline uint8_t tp_is_explosion (tpp t)
{
    return (t->is_explosion);
}

static inline uint8_t tp_is_hidden_from_editor (tpp t)
{
    return (t->is_hidden_from_editor);
}

static inline uint8_t tp_is_animated (tpp t)
{
    return (t->is_animated);
}

static inline uint8_t tp_is_collision_map_medium (tpp t)
{
    return (t->is_collision_map_medium);
}

static inline uint8_t tp_is_collision_map_small (tpp t)
{
    return (t->is_collision_map_small);
}

static inline uint8_t tp_is_collision_map_tiny (tpp t)
{
    return (t->is_collision_map_tiny);
}

static inline uint8_t tp_is_combustable (tpp t)
{
    return (t->is_combustable);
}

static inline uint8_t tp_is_projectile (tpp t)
{
    return (t->is_projectile);
}

static inline uint8_t tp_is_inactive (tpp t)
{
    return (t->is_inactive);
}

static inline uint8_t tp_is_joinable (tpp t)
{
    return (t->is_joinable);
}

static inline uint8_t tp_is_wall (tpp t)
{
    return (t->is_wall);
}

static inline uint8_t tp_is_effect_sway (tpp t)
{
    return (t->is_effect_sway);
}

static inline uint8_t tp_is_effect_pulse (tpp t)
{
    return (t->is_effect_pulse);
}

static inline uint8_t tp_is_effect_rotate_2way (tpp t)
{
    return (t->is_effect_rotate_2way);
}

tpp random_wall(void);
tpp random_corridor_wall(void);
tpp random_door(void);
tpp random_obstacle(void);
tpp random_spikes(void);
tpp random_floor(void);
tpp random_corridor(void);
tpp random_player(void);
tpp random_exit(void);
tpp random_entrance(void);
tpp random_food(void);
tpp random_treasure(int shop_floor);
tpp random_weapon(int shop_floor);
tpp random_rock(void);
tpp random_smallrock(void);
tpp random_lava(void);
tpp random_monst(int depth);
tpp random_trap(int depth);
tpp random_mob(int depth);
