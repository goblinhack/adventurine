/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file.
 */

#pragma once

uint8_t map_is_ring_at(levelp, int32_t x, int32_t y);
uint8_t map_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
uint8_t map_is_exit_at(levelp, int32_t x, int32_t y);
uint8_t map_is_dungeon_floor_at(levelp, int32_t x, int32_t y);
uint8_t map_is_food_at(levelp, int32_t x, int32_t y);
uint8_t map_is_monst_at(levelp, int32_t x, int32_t y);
uint8_t map_is_inactive_at(levelp, int32_t x, int32_t y);
uint8_t map_is_player_at(levelp, int32_t x, int32_t y);
uint8_t map_is_wall_at(levelp, int32_t x, int32_t y);
uint8_t map_is_shop_floor_at(levelp, int32_t x, int32_t y);
uint8_t map_is_rock_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animated_at(levelp, int32_t x, int32_t y);
uint8_t map_is_combustable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_key_at(levelp, int32_t x, int32_t y);
uint8_t map_is_shadow_caster_at(levelp, int32_t x, int32_t y);
uint8_t map_is_weapon_at(levelp, int32_t x, int32_t y);
uint8_t map_is_treasure_at(levelp, int32_t x, int32_t y);
uint8_t map_is_wearable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_fragile_at(levelp, int32_t x, int32_t y);
uint8_t map_is_star_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animated_no_dir_at(levelp, int32_t x, int32_t y);
uint8_t map_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
uint8_t map_is_light_source_at(levelp, int32_t x, int32_t y);
uint8_t map_is_candle_light_at(levelp, int32_t x, int32_t y);
uint8_t map_is_cats_eyes_at(levelp, int32_t x, int32_t y);
uint8_t map_is_fire_at(levelp, int32_t x, int32_t y);
uint8_t map_is_ring_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animation_at(levelp, int32_t x, int32_t y);
uint8_t map_is_shadow_caster_soft_at(levelp, int32_t x, int32_t y);
uint8_t map_is_non_explosive_gas_cloud_at(levelp, int32_t x, int32_t y);
uint8_t map_is_item_unusable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_item_unusable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y);
uint8_t map_is_door_at(levelp, int32_t x, int32_t y);
uint8_t map_is_mob_spawner_at(levelp, int32_t x, int32_t y);
uint8_t map_is_acid_at(levelp, int32_t x, int32_t y);
uint8_t map_is_water_at(levelp, int32_t x, int32_t y);
uint8_t map_is_lava_at(levelp, int32_t x, int32_t y);
uint8_t map_is_water_at(levelp, int32_t x, int32_t y);
uint8_t map_is_teleport_at(levelp, int32_t x, int32_t y);
uint8_t map_is_cobweb_at(levelp, int32_t x, int32_t y);
uint8_t map_is_ethereal_at(levelp, int32_t x, int32_t y);
uint8_t map_is_variable_size_at(levelp, int32_t x, int32_t y);
uint8_t map_is_variable_size_at(levelp, int32_t x, int32_t y);
uint8_t map_is_magical_weapon_at(levelp, int32_t x, int32_t y);
uint8_t map_is_ranged_weapon_at(levelp, int32_t x, int32_t y);
uint8_t map_is_melee_weapon_at(levelp, int32_t x, int32_t y);
uint8_t map_is_poison_at(levelp, int32_t x, int32_t y);
uint8_t map_is_cloud_effect_at(levelp, int32_t x, int32_t y);
uint8_t map_is_powerup_at(levelp, int32_t x, int32_t y);
uint8_t map_is_damaged_on_firing_at(levelp, int32_t x, int32_t y);
uint8_t map_is_hard_at(levelp, int32_t x, int32_t y);
uint8_t map_is_sleeping_at(levelp, int32_t x, int32_t y);
uint8_t map_is_bomb_at(levelp, int32_t x, int32_t y);
uint8_t map_is_sawblade_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_text_at(levelp, int32_t x, int32_t y);
uint8_t map_is_visible_on_debug_only_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_zap_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_sleep_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_trigger_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_down_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_up_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_left_at(levelp, int32_t x, int32_t y);
uint8_t map_is_action_right_at(levelp, int32_t x, int32_t y);
uint8_t map_can_walk_through_at(levelp, int32_t x, int32_t y);
uint8_t map_is_weapon_carry_anim_at(levelp, int32_t x, int32_t y);
uint8_t map_is_spell_at(levelp, int32_t x, int32_t y);
uint8_t map_is_hand_item_at(levelp, int32_t x, int32_t y);
uint8_t map_is_boots_at(levelp, int32_t x, int32_t y);
uint8_t map_is_helmet_at(levelp, int32_t x, int32_t y);
uint8_t map_is_armor_at(levelp, int32_t x, int32_t y);
uint8_t map_is_given_randomly_at_start_at(levelp, int32_t x, int32_t y);
uint8_t map_is_magical_at(levelp, int32_t x, int32_t y);
uint8_t map_is_degradable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_cursed_at(levelp, int32_t x, int32_t y);
uint8_t map_is_animated_only_when_moving_at(levelp, int32_t x, int32_t y);
uint8_t map_is_warm_blooded_at(levelp, int32_t x, int32_t y);
uint8_t map_can_be_enchanted_at(levelp, int32_t x, int32_t y);
uint8_t map_is_stackable_at(levelp, int32_t x, int32_t y);
uint8_t map_is_torch_at(levelp, int32_t x, int32_t y);
uint8_t map_is_explosion_at(levelp, int32_t x, int32_t y);
uint8_t map_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);
uint8_t map_is_projectile_at(levelp, int32_t x, int32_t y);

tpp map_find_key1_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_ring_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_exit_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_floor_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_food_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_projectile_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_monst_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_item_perma_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_plant_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_player_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_star_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_wall_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_wall_deco_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_ladder_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_ladder_deco_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_rock_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_key_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_xxx2_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_xxx3_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_blocks_light_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_xxx5_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_xxx6_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_xxx7_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_fragile_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_star_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_key1_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_light_source_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_candle_light_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_star_green_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_star_cyan_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_star_black_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_star_purple_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_explosion_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_spikes_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_item_unusable_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_item_unusable_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_bomb_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_door_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_generator_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_explosion_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_hidden_from_editor_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_acid_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_lava_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_water_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_teleport_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_potion_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_shield_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_cobweb_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_ethereal_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_variable_size_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_one_shot_move_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_magical_weapon_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_ranged_weapon_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_melee_weapon_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_is_poison_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_cloud_effect_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_damaged_on_firing_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_hard_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_sleeping_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_bomb_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_saw_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_text_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_visible_on_debug_only_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_sleep_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_zap_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_trigger_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_down_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_up_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_left_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_action_right_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_can_walk_through_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_weapon_carry_anim_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_spell_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_given_randomly_at_start_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_magical_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_degradable_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_cursed_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_animate_only_when_moving_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_damaged_on_firing_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_hard_at(levelp, int32_t x, int32_t y, widp *);
tpp map_find_torch_at(levelp, int32_t x, int32_t y, widp *);

void level_open_door(levelp level, int32_t ix, int32_t iy);

thingp map_thing_is_ring_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_thing_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_exit_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_dungeon_floor_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_food_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_monst_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_inactive_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_plant_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_player_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_wall_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_rock_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_combustable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_projectile_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_key_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_shadow_caster_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_weapon_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_treasure_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_wearable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_fragile_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_star_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_no_dir_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_light_source_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_candle_light_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_cats_eyes_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_fire_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_ring_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animation_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animation_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_non_explosive_gas_cloud_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_unusable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_item_unusable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bomb_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_door_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_mob_spawner_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_acid_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_lava_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_water_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_teleport_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_cobweb_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_ethereal_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_variable_size_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_variable_size_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_magical_weapon_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_ranged_weapon_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_melee_weapon_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_poison_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_cloud_effect_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_powerup_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_damaged_on_firing_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_hard_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_sleeping_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_bomb_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_sawblade_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_text_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_visible_on_debug_only_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_sleep_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_zap_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_trigger_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_down_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_up_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_left_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_action_right_at(levelp, int32_t x, int32_t y);
thingp map_thing_can_walk_through_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_weapon_carry_anim_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_spell_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_hand_item_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_boots_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_helmet_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_armor_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_given_randomly_at_start_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_magical_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_degradable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_cursed_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_animated_only_when_moving_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_warm_blooded_at(levelp, int32_t x, int32_t y);
thingp map_thing_can_be_enchanted_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_stackable_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_torch_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_explosion_at(levelp, int32_t x, int32_t y);
thingp map_thing_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);

tree_rootp map_all_things_is_ring(levelp);
tree_rootp map_all_things_is_ring_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_exit(levelp);
tree_rootp map_all_things_is_exit_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_dungeon_floor(levelp);
tree_rootp map_all_things_is_dungeon_floor_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_food(levelp);
tree_rootp map_all_things_is_food_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_monst(levelp);
tree_rootp map_all_things_is_monst_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_inactive(levelp);
tree_rootp map_all_things_is_inactive_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_plant(levelp);
tree_rootp map_all_things_is_plant_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_player(levelp);
tree_rootp map_all_things_is_player_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_wall(levelp);
tree_rootp map_all_things_is_wall_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key(levelp);
tree_rootp map_all_things_is_animated(levelp);
tree_rootp map_all_things_is_animated_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_combustable(levelp);
tree_rootp map_all_things_is_combustable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_key_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_rock(levelp);
tree_rootp map_all_things_is_rock_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_shadow_caster(levelp);
tree_rootp map_all_things_is_shadow_caster_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_weapon(levelp);
tree_rootp map_all_things_is_weapon_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_treasure(levelp);
tree_rootp map_all_things_is_treasure_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_wearable(levelp);
tree_rootp map_all_things_is_wearable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_fragile(levelp);
tree_rootp map_all_things_is_fragile_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_star(levelp);
tree_rootp map_all_things_is_star_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animated_no_dir(levelp);
tree_rootp map_all_things_is_animated_no_dir_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_weapon_swing_effect(levelp);
tree_rootp map_all_things_is_weapon_swing_effect_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_light_source(levelp);
tree_rootp map_all_things_is_light_source_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_candle_light(levelp);
tree_rootp map_all_things_is_candle_light_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_cats_eyes(levelp);
tree_rootp map_all_things_is_cats_eyes_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_fire(levelp);
tree_rootp map_all_things_is_fire_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_ring(levelp);
tree_rootp map_all_things_is_ring_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animation(levelp);
tree_rootp map_all_things_is_animation_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animation(levelp);
tree_rootp map_all_things_is_animation_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_non_explosive_gas_cloud(levelp);
tree_rootp map_all_things_is_non_explosive_gas_cloud_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_item_unusable(levelp);
tree_rootp map_all_things_is_item_unusable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_item_unusable(levelp);
tree_rootp map_all_things_is_item_unusable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_valid_for_action_bar(levelp);
tree_rootp map_all_things_is_valid_for_action_bar_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_seedpod_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_bomb(levelp);
tree_rootp map_all_things_is_bomb_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_door(levelp);
tree_rootp map_all_things_is_door_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_mob_spawner(levelp);
tree_rootp map_all_things_is_mob_spawner_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_acid(levelp);
tree_rootp map_all_things_is_lava(levelp);
tree_rootp map_all_things_is_water(levelp);
tree_rootp map_all_things_is_teleport(levelp);
tree_rootp map_all_things_is_cobweb(levelp);
tree_rootp map_all_things_is_ethereal(levelp);
tree_rootp map_all_things_is_variable_size(levelp);
tree_rootp map_all_things_is_variable_size(levelp);
tree_rootp map_all_things_is_magical_weapon(levelp);
tree_rootp map_all_things_is_ranged_weapon(levelp);
tree_rootp map_all_things_is_melee_weapon(levelp);
tree_rootp map_all_things_is_poison(levelp);
tree_rootp map_all_things_is_cloud_effect(levelp);
tree_rootp map_all_things_is_powerup(levelp);
tree_rootp map_all_things_is_damaged_on_firing(levelp);
tree_rootp map_all_things_is_hard(levelp);
tree_rootp map_all_things_is_sleeping(levelp);
tree_rootp map_all_things_is_bomb(levelp);
tree_rootp map_all_things_is_sawblade(levelp);
tree_rootp map_all_things_is_action_text(levelp);
tree_rootp map_all_things_is_visible_on_debug_only(levelp);
tree_rootp map_all_things_is_action_sleep(levelp);
tree_rootp map_all_things_is_action_zap(levelp);
tree_rootp map_all_things_is_action_trigger(levelp);
tree_rootp map_all_things_is_action_down(levelp);
tree_rootp map_all_things_is_action_up(levelp);
tree_rootp map_all_things_is_action_left(levelp);
tree_rootp map_all_things_is_action_right(levelp);
tree_rootp map_all_things_can_walk_through(levelp);
tree_rootp map_all_things_is_weapon_carry_anim(levelp);
tree_rootp map_all_things_is_spell(levelp);
tree_rootp map_all_things_is_hand_item(levelp);
tree_rootp map_all_things_is_boots(levelp);
tree_rootp map_all_things_is_helmet(levelp);
tree_rootp map_all_things_is_armor(levelp);
tree_rootp map_all_things_is_given_randomly_at_start(levelp);
tree_rootp map_all_things_is_magical(levelp);
tree_rootp map_all_things_is_degradable(levelp);
tree_rootp map_all_things_is_cursed(levelp);
tree_rootp map_all_things_is_animated_only_when_moving(levelp);
tree_rootp map_all_things_is_warm_blooded(levelp);
tree_rootp map_all_things_can_be_enchanted(levelp);
tree_rootp map_all_things_is_stackable(levelp);
tree_rootp map_all_things_is_torch(levelp);
tree_rootp map_all_things_is_acid_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_lava_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_water_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_teleport_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_cobweb_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_ethereal_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_variable_size_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_variable_size_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_magical_weapon_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_ranged_weapon_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_melee_weapon_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_poison_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_cloud_effect_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_powerup_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_damaged_on_firing_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_hard_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_sleeping_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_bomb_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_sawblade_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_text_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_visible_on_debug_only_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_sleep_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_zap_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_trigger_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_down_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_up_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_left_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_action_right_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_can_walk_through_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_weapon_carry_anim_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_spell_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_hand_item_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_boots_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_helmet_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_armor_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_given_randomly_at_start_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_magical_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_degradable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_cursed_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_animated_only_when_moving_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_warm_blooded_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_can_be_enchanted_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_stackable_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_torch_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_explosion(levelp);
tree_rootp map_all_things_is_explosion_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_hidden_from_editor(levelp);
tree_rootp map_all_things_is_hidden_from_editor_at(levelp, int32_t x, int32_t y);
tree_rootp map_all_things_is_projectile(levelp);
tree_rootp map_all_things_is_projectile_at(levelp, int32_t x, int32_t y);

uint32_t level_count_is_item_is_hidden(levelp);
uint32_t level_count_is_exit(levelp);

#include "tree.h"

typedef struct tree_thing_node_ {
    tree_key_pointer tree;
} tree_thing_node;

void map_fixup(levelp);

typedef uint8_t (*map_is_at_callback)(tpp);

uint8_t map_count_x_at(levelp level,
                       int32_t x, int32_t y,
                       map_is_at_callback callback);

thingp map_is_tp_at(levelp level, int32_t x, int32_t y, tpp tp);

#include "enum.h"

#define MAP_DEPTH_ENUMS(list_macro)                             \
    list_macro(MAP_DEPTH_FLOOR = 0,         "floor"),           \
    list_macro(MAP_DEPTH_FLOOR2,            "floor2"),          \
    list_macro(MAP_DEPTH_OBJ,               "obj"),             \
    list_macro(MAP_DEPTH_OBJ2,              "obj2"),            \
    list_macro(MAP_DEPTH_LADDER,            "ladder"),          \
    list_macro(MAP_DEPTH_PLAYER,            "player"),          \
    /* water */                                                 \
    list_macro(MAP_DEPTH_WALL,              "wall"),            \
    list_macro(MAP_DEPTH_WALL_DECO,         "wall_deco"),       \
    list_macro(MAP_DEPTH_WALL_DECO2,        "wall_deco2"),      \
    list_macro(MAP_DEPTH_EXPLOSION,         "explosion"),       \
    list_macro(MAP_DEPTH_EXPLOSION_TOP,     "explosion_top"),   \
    list_macro(MAP_DEPTH_ACTIONS,           "actions"),         \

#define MAP_DEPTH 12

ENUM_DEF_H(MAP_DEPTH_ENUMS, map_depth)

static inline int map_ok (int x, int y)
{
    if ((x >= MAP_WIDTH) || (x < 0) || (y >= MAP_HEIGHT) || (y < 0)) {
        return (false);
    }

    return (true);
}
