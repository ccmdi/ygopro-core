/*
 * Duel state serialization implementations.
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "serialize.h"
#include "card.h"
#include "duel.h"
#include "effect.h"
#include "field.h"
#include "group.h"

// ============================================================
// card_state
// ============================================================
void serialize_card_state(SerializeBuffer& buf, const card_state& cs, const IdMaps& maps) {
	buf.write_u32(cs.code);
	buf.write_u32(cs.code2);
	// setcodes
	buf.write_u32(static_cast<uint32_t>(cs.setcodes.size()));
	for(uint16_t sc : cs.setcodes)
		buf.write_u16(sc);
	buf.write_u32(cs.type);
	buf.write_u32(cs.level);
	buf.write_u32(cs.rank);
	buf.write_u32(cs.link);
	buf.write_u32(cs.link_marker);
	buf.write_u32(cs.lscale);
	buf.write_u32(cs.rscale);
	buf.write_u32(cs.attribute);
	buf.write_u64(cs.race);
	buf.write_i32(cs.attack);
	buf.write_i32(cs.defense);
	buf.write_i32(cs.base_attack);
	buf.write_i32(cs.base_defense);
	buf.write_u8(cs.controler);
	buf.write_u8(cs.location);
	buf.write_u32(cs.sequence);
	buf.write_u32(cs.position);
	buf.write_u32(cs.reason);
	buf.write_bool(cs.pzone);
	buf.write_card_id(cs.reason_card, maps);
	buf.write_u8(cs.reason_player);
	buf.write_effect_id(cs.reason_effect, maps);
}

void deserialize_card_state(SerializeBuffer& buf, card_state& cs, const IdMaps& maps) {
	cs.code = buf.read_u32();
	cs.code2 = buf.read_u32();
	uint32_t n_sc = buf.read_u32();
	cs.setcodes.clear();
	for(uint32_t i = 0; i < n_sc; i++)
		cs.setcodes.insert(buf.read_u16());
	cs.type = buf.read_u32();
	cs.level = buf.read_u32();
	cs.rank = buf.read_u32();
	cs.link = buf.read_u32();
	cs.link_marker = buf.read_u32();
	cs.lscale = buf.read_u32();
	cs.rscale = buf.read_u32();
	cs.attribute = buf.read_u32();
	cs.race = buf.read_u64();
	cs.attack = buf.read_i32();
	cs.defense = buf.read_i32();
	cs.base_attack = buf.read_i32();
	cs.base_defense = buf.read_i32();
	cs.controler = buf.read_u8();
	cs.location = buf.read_u8();
	cs.sequence = buf.read_u32();
	cs.position = buf.read_u32();
	cs.reason = buf.read_u32();
	cs.pzone = buf.read_bool();
	cs.reason_card = buf.read_card_id(maps);
	cs.reason_player = buf.read_u8();
	cs.reason_effect = buf.read_effect_id(maps);
}

// ============================================================
// card_data (static data - only serialize the code, rebuild from DB)
// ============================================================
void serialize_card_data(SerializeBuffer& buf, const card_data& cd) {
	buf.write_u32(cd.code);
}

// ============================================================
// card
// ============================================================
void serialize_card(SerializeBuffer& buf, card* c, const IdMaps& maps) {
	// Identity
	buf.write_u32(c->cardid);
	serialize_card_data(buf, c->data);

	// States
	serialize_card_state(buf, c->previous, maps);
	serialize_card_state(buf, c->temp, maps);
	serialize_card_state(buf, c->current, maps);

	buf.write_u8(c->owner);

	// summon_info
	buf.write_u32(c->summon.type);
	buf.write_u8(c->summon.player);
	buf.write_u8(c->summon.location);
	buf.write_u8(c->summon.sequence);
	buf.write_bool(c->summon.pzone);

	buf.write_u32(c->status);
	buf.write_u32(c->cover);

	// sendto_param
	buf.write_u8(c->sendto_param.playerid);
	buf.write_u8(c->sendto_param.position);
	buf.write_u8(c->sendto_param.location);
	buf.write_u8(c->sendto_param.sequence);

	buf.write_u32(c->release_param);
	buf.write_u32(c->sum_param);
	buf.write_u32(c->position_param);
	buf.write_u32(c->spsummon_param);
	buf.write_u32(c->to_field_param);
	buf.write_u8(c->attack_announce_count);
	buf.write_u8(c->direct_attackable);
	buf.write_u8(c->announce_count);
	buf.write_u8(c->attacked_count);
	buf.write_u8(c->attack_all_target);
	buf.write_u8(c->attack_controler);
	buf.write_u32(c->cardid);
	buf.write_u32(c->fieldid);
	buf.write_u32(c->fieldid_r);
	buf.write_u16(c->turnid);
	buf.write_u16(c->turn_counter);
	buf.write_u8(c->unique_pos[0]);
	buf.write_u8(c->unique_pos[1]);
	buf.write_u32(c->unique_fieldid);
	buf.write_u32(c->unique_code);
	buf.write_u32(c->unique_location);
	buf.write_i32(c->unique_function);
	buf.write_effect_id(c->unique_effect, maps);
	buf.write_u32(c->spsummon_code);
	buf.write_u16(c->spsummon_counter[0]);
	buf.write_u16(c->spsummon_counter[1]);
	buf.write_u16(c->spsummon_counter_rst[0]);
	buf.write_u16(c->spsummon_counter_rst[1]);

	// assume map<uint32_t, uint64_t>
	buf.write_u32(static_cast<uint32_t>(c->assume.size()));
	for(auto& [k, v] : c->assume) {
		buf.write_u32(k);
		buf.write_u64(v);
	}

	// Pointer fields
	buf.write_card_id(c->equiping_target, maps);
	buf.write_card_id(c->pre_equip_target, maps);
	buf.write_card_id(c->overlay_target, maps);
	buf.write_card_id(c->pre_overlay_target, maps);

	// relation_map: unordered_map<card*, uint32_t>
	buf.write_u32(static_cast<uint32_t>(c->relations.size()));
	for(auto& [card_ptr, val] : c->relations) {
		buf.write_card_id(card_ptr, maps);
		buf.write_u32(val);
	}

	// counter_map: map<uint16_t, array<uint16_t, 2>>
	buf.write_u32(static_cast<uint32_t>(c->counters.size()));
	for(auto& [ctype, arr] : c->counters) {
		buf.write_u16(ctype);
		buf.write_u16(arr[0]);
		buf.write_u16(arr[1]);
	}

	// indestructable_effects: map<uint32_t, int32_t>
	buf.write_u32(static_cast<uint32_t>(c->indestructable_effects.size()));
	for(auto& [k, v] : c->indestructable_effects) {
		buf.write_u32(k);
		buf.write_i32(v);
	}

	// attacker_map: unordered_map<uint32_t, pair<card*, uint32_t>>
	auto write_attacker_map = [&](const card::attacker_map& m) {
		buf.write_u32(static_cast<uint32_t>(m.size()));
		for(auto& [k, p] : m) {
			buf.write_u32(k);
			buf.write_card_id(p.first, maps);
			buf.write_u32(p.second);
		}
	};
	write_attacker_map(c->announced_cards);
	write_attacker_map(c->attacked_cards);
	write_attacker_map(c->battled_cards);

	// Card sets
	buf.write_card_set(c->equiping_cards, maps);
	buf.write_card_set(c->material_cards, maps);
	buf.write_card_set(c->effect_target_owner, maps);
	buf.write_card_set(c->effect_target_cards, maps);
	buf.write_card_vector(c->xyz_materials, maps);

	// Effect containers: multimap<uint32_t, effect*>
	buf.write_effect_container(c->single_effect, maps);
	buf.write_effect_container(c->field_effect, maps);
	buf.write_effect_container(c->equip_effect, maps);
	buf.write_effect_container(c->target_effect, maps);
	buf.write_effect_container(c->xmaterial_effect, maps);

	// effect_indexer: unordered_map<effect*, effect_container::iterator>
	// We store as (effect_id, key) pairs -- the iterator will be rebuilt
	buf.write_u32(static_cast<uint32_t>(c->indexer.size()));
	for(auto& [eff, it] : c->indexer) {
		buf.write_effect_id(eff, maps);
		buf.write_u32(it->first);
	}

	// effect_relation: unordered_set<pair<effect*, uint16_t>>
	buf.write_u32(static_cast<uint32_t>(c->relate_effect.size()));
	for(auto& [eff, val] : c->relate_effect) {
		buf.write_effect_id(eff, maps);
		buf.write_u16(val);
	}

	// immune_effect: effect_set_v (vector<effect*>)
	buf.write_effect_set(c->immune_effect, maps);
}

void deserialize_card(SerializeBuffer& buf, card* c, const IdMaps& maps, duel* pduel) {
	// Identity
	c->cardid = buf.read_u32();
	uint32_t data_code = buf.read_u32();
	if(data_code)
		c->data = pduel->read_card(data_code);
	c->data.code = data_code;

	// States
	deserialize_card_state(buf, c->previous, maps);
	deserialize_card_state(buf, c->temp, maps);
	deserialize_card_state(buf, c->current, maps);

	c->owner = buf.read_u8();

	c->summon.type = buf.read_u32();
	c->summon.player = buf.read_u8();
	c->summon.location = buf.read_u8();
	c->summon.sequence = buf.read_u8();
	c->summon.pzone = buf.read_bool();

	c->status = buf.read_u32();
	c->cover = buf.read_u32();

	c->sendto_param.playerid = buf.read_u8();
	c->sendto_param.position = buf.read_u8();
	c->sendto_param.location = buf.read_u8();
	c->sendto_param.sequence = buf.read_u8();

	c->release_param = buf.read_u32();
	c->sum_param = buf.read_u32();
	c->position_param = buf.read_u32();
	c->spsummon_param = buf.read_u32();
	c->to_field_param = buf.read_u32();
	c->attack_announce_count = buf.read_u8();
	c->direct_attackable = buf.read_u8();
	c->announce_count = buf.read_u8();
	c->attacked_count = buf.read_u8();
	c->attack_all_target = buf.read_u8();
	c->attack_controler = buf.read_u8();
	c->cardid = buf.read_u32();
	c->fieldid = buf.read_u32();
	c->fieldid_r = buf.read_u32();
	c->turnid = buf.read_u16();
	c->turn_counter = buf.read_u16();
	c->unique_pos[0] = buf.read_u8();
	c->unique_pos[1] = buf.read_u8();
	c->unique_fieldid = buf.read_u32();
	c->unique_code = buf.read_u32();
	c->unique_location = buf.read_u32();
	c->unique_function = buf.read_i32();
	c->unique_effect = buf.read_effect_id(maps);
	c->spsummon_code = buf.read_u32();
	c->spsummon_counter[0] = buf.read_u16();
	c->spsummon_counter[1] = buf.read_u16();
	c->spsummon_counter_rst[0] = buf.read_u16();
	c->spsummon_counter_rst[1] = buf.read_u16();

	// assume
	uint32_t n = buf.read_u32();
	c->assume.clear();
	for(uint32_t i = 0; i < n; i++) {
		uint32_t k = buf.read_u32();
		uint64_t v = buf.read_u64();
		c->assume[k] = v;
	}

	c->equiping_target = buf.read_card_id(maps);
	c->pre_equip_target = buf.read_card_id(maps);
	c->overlay_target = buf.read_card_id(maps);
	c->pre_overlay_target = buf.read_card_id(maps);

	// relations
	n = buf.read_u32();
	c->relations.clear();
	for(uint32_t i = 0; i < n; i++) {
		card* cp = buf.read_card_id(maps);
		uint32_t val = buf.read_u32();
		c->relations[cp] = val;
	}

	// counters
	n = buf.read_u32();
	c->counters.clear();
	for(uint32_t i = 0; i < n; i++) {
		uint16_t ctype = buf.read_u16();
		uint16_t a0 = buf.read_u16();
		uint16_t a1 = buf.read_u16();
		c->counters[ctype] = {a0, a1};
	}

	// indestructable_effects
	n = buf.read_u32();
	c->indestructable_effects.clear();
	for(uint32_t i = 0; i < n; i++) {
		uint32_t k = buf.read_u32();
		int32_t v = buf.read_i32();
		c->indestructable_effects[k] = v;
	}

	// attacker maps
	auto read_attacker_map = [&](card::attacker_map& m) {
		uint32_t cnt = buf.read_u32();
		m.clear();
		for(uint32_t i = 0; i < cnt; i++) {
			uint32_t k = buf.read_u32();
			card* cp = buf.read_card_id(maps);
			uint32_t v = buf.read_u32();
			m[k] = {cp, v};
		}
	};
	read_attacker_map(c->announced_cards);
	read_attacker_map(c->attacked_cards);
	read_attacker_map(c->battled_cards);

	buf.read_card_set(c->equiping_cards, maps);
	buf.read_card_set(c->material_cards, maps);
	buf.read_card_set(c->effect_target_owner, maps);
	buf.read_card_set(c->effect_target_cards, maps);
	buf.read_card_vector(c->xyz_materials, maps);

	buf.read_effect_container(c->single_effect, maps);
	buf.read_effect_container(c->field_effect, maps);
	buf.read_effect_container(c->equip_effect, maps);
	buf.read_effect_container(c->target_effect, maps);
	buf.read_effect_container(c->xmaterial_effect, maps);

	// effect_indexer -- rebuild iterators from (effect_id, key) pairs
	n = buf.read_u32();
	c->indexer.clear();
	for(uint32_t i = 0; i < n; i++) {
		effect* eff = buf.read_effect_id(maps);
		uint32_t key = buf.read_u32();
		// Find the matching iterator in the appropriate effect_container
		// We need to search all containers this card has
		auto find_in = [&](effect_container& cont) -> bool {
			auto range = cont.equal_range(key);
			for(auto it = range.first; it != range.second; ++it) {
				if(it->second == eff) {
					c->indexer[eff] = it;
					return true;
				}
			}
			return false;
		};
		if(!find_in(c->single_effect))
			if(!find_in(c->field_effect))
				if(!find_in(c->equip_effect))
					if(!find_in(c->target_effect))
						find_in(c->xmaterial_effect);
	}

	// relate_effect
	n = buf.read_u32();
	c->relate_effect.clear();
	for(uint32_t i = 0; i < n; i++) {
		effect* eff = buf.read_effect_id(maps);
		uint16_t val = buf.read_u16();
		c->relate_effect.insert({eff, val});
	}

	buf.read_effect_set(c->immune_effect, maps);
}

// ============================================================
// effect
// ============================================================
void serialize_effect(SerializeBuffer& buf, effect* e, const IdMaps& maps) {
	buf.write_u32(e->id);
	buf.write_u8(e->count_limit);
	buf.write_u8(e->count_limit_max);
	buf.write_u8(e->count_flag);
	buf.write_u8(e->count_hopt_index);
	buf.write_u8(e->effect_owner);
	buf.write_u16(e->type);
	buf.write_u16(e->copy_id);
	buf.write_u16(e->range);
	buf.write_u16(e->s_range);
	buf.write_u16(e->o_range);
	buf.write_u16(e->reset_count);
	buf.write_u16(e->active_location);
	buf.write_u16(e->active_sequence);
	buf.write_u16(e->status);
	buf.write_u32(e->code);
	buf.write_u32(e->flag[0]);
	buf.write_u32(e->flag[1]);
	buf.write_u32(e->initial_id);
	buf.write_u32(e->reset_flag);
	buf.write_u32(e->count_code);
	buf.write_u32(e->hint_timing[0]);
	buf.write_u32(e->hint_timing[1]);
	buf.write_u32(e->card_type);
	buf.write_u32(e->active_type);
	buf.write_i32(e->label_object);
	// Lua function refs -- stored as-is for now; Phase 4 will remap them
	buf.write_i32(e->condition);
	buf.write_i32(e->cost);
	buf.write_i32(e->target);
	buf.write_i32(e->value);
	buf.write_i32(e->operation);
	buf.write_u64(e->category);
	buf.write_card_id(e->owner, maps);
	buf.write_card_id(e->handler, maps);
	buf.write_card_id(e->active_handler, maps);
	buf.write_u64(e->description);
	// label: vector<lua_Integer>
	buf.write_u32(static_cast<uint32_t>(e->label.size()));
	for(auto v : e->label)
		buf.write_u64(static_cast<uint64_t>(v));
}

void deserialize_effect(SerializeBuffer& buf, effect* e, const IdMaps& maps) {
	e->id = buf.read_u32();
	e->count_limit = buf.read_u8();
	e->count_limit_max = buf.read_u8();
	e->count_flag = buf.read_u8();
	e->count_hopt_index = buf.read_u8();
	e->effect_owner = buf.read_u8();
	e->type = buf.read_u16();
	e->copy_id = buf.read_u16();
	e->range = buf.read_u16();
	e->s_range = buf.read_u16();
	e->o_range = buf.read_u16();
	e->reset_count = buf.read_u16();
	e->active_location = buf.read_u16();
	e->active_sequence = buf.read_u16();
	e->status = buf.read_u16();
	e->code = buf.read_u32();
	e->flag[0] = buf.read_u32();
	e->flag[1] = buf.read_u32();
	e->initial_id = buf.read_u32();
	e->reset_flag = buf.read_u32();
	e->count_code = buf.read_u32();
	e->hint_timing[0] = buf.read_u32();
	e->hint_timing[1] = buf.read_u32();
	e->card_type = buf.read_u32();
	e->active_type = buf.read_u32();
	e->label_object = buf.read_i32();
	e->condition = buf.read_i32();
	e->cost = buf.read_i32();
	e->target = buf.read_i32();
	e->value = buf.read_i32();
	e->operation = buf.read_i32();
	e->category = buf.read_u64();
	e->owner = buf.read_card_id(maps);
	e->handler = buf.read_card_id(maps);
	e->active_handler = buf.read_card_id(maps);
	e->description = buf.read_u64();
	uint32_t n = buf.read_u32();
	e->label.resize(n);
	for(uint32_t i = 0; i < n; i++)
		e->label[i] = static_cast<lua_Integer>(buf.read_u64());
}

// ============================================================
// tevent
// ============================================================
void serialize_tevent(SerializeBuffer& buf, const tevent& ev, const IdMaps& maps) {
	buf.write_card_id(ev.trigger_card, maps);
	// owned_lua<group> event_cards -- inline the card set
	if(ev.event_cards) {
		buf.write_bool(true);
		buf.write_card_set(static_cast<group*>(ev.event_cards)->container, maps);
	} else {
		buf.write_bool(false);
	}
	buf.write_effect_id(ev.reason_effect, maps);
	buf.write_u32(ev.event_code);
	buf.write_u32(ev.event_value);
	buf.write_u32(ev.reason);
	buf.write_u8(ev.event_player);
	buf.write_u8(ev.reason_player);
	buf.write_u32(ev.global_id);
}

void deserialize_tevent(SerializeBuffer& buf, tevent& ev, const IdMaps& maps, duel* pduel) {
	ev.trigger_card = buf.read_card_id(maps);
	bool has_group = buf.read_bool();
	if(has_group) {
		card_set cs;
		buf.read_card_set(cs, maps);
		ev.event_cards = pduel->new_group(std::move(cs));
	} else {
		ev.event_cards = nullptr;
	}
	ev.reason_effect = buf.read_effect_id(maps);
	ev.event_code = buf.read_u32();
	ev.event_value = buf.read_u32();
	ev.reason = buf.read_u32();
	ev.event_player = buf.read_u8();
	ev.reason_player = buf.read_u8();
	ev.global_id = buf.read_u32();
}

// ============================================================
// optarget
// ============================================================
void serialize_optarget(SerializeBuffer& buf, const optarget& ot, const IdMaps& maps) {
	if(ot.op_cards) {
		buf.write_bool(true);
		buf.write_card_set(static_cast<group*>(ot.op_cards)->container, maps);
	} else {
		buf.write_bool(false);
	}
	buf.write_u8(ot.op_count);
	buf.write_u8(ot.op_player);
	buf.write_i32(ot.op_param);
}

void deserialize_optarget(SerializeBuffer& buf, optarget& ot, const IdMaps& maps, duel* pduel) {
	bool has_group = buf.read_bool();
	if(has_group) {
		card_set cs;
		buf.read_card_set(cs, maps);
		ot.op_cards = pduel->new_group(std::move(cs));
	} else {
		ot.op_cards = nullptr;
	}
	ot.op_count = buf.read_u8();
	ot.op_player = buf.read_u8();
	ot.op_param = buf.read_i32();
}

// ============================================================
// chain
// ============================================================
void serialize_chain(SerializeBuffer& buf, const chain& ch, const IdMaps& maps) {
	serialize_card_state(buf, ch.triggering_state, maps);
	serialize_tevent(buf, ch.evt, maps);
	buf.write_u8(ch.chain_count);
	buf.write_u8(ch.triggering_player);
	buf.write_u8(ch.triggering_controler);
	buf.write_u8(ch.triggering_position);
	buf.write_u8(ch.target_player);
	buf.write_u8(ch.disable_player);
	buf.write_u8(ch.triggering_summon_location);
	buf.write_bool(ch.triggering_summon_proc_complete);
	buf.write_bool(ch.was_just_sent);
	buf.write_u16(ch.chain_id);
	buf.write_u16(ch.triggering_location);
	buf.write_u32(ch.triggering_sequence);
	buf.write_u32(ch.triggering_status);
	buf.write_u32(ch.triggering_summon_type);
	buf.write_i32(ch.replace_op);
	buf.write_i32(ch.target_param);
	buf.write_u32(ch.flag);
	buf.write_u32(ch.event_id);
	buf.write_effect_id(ch.triggering_effect, maps);
	// target_cards: owned_lua<group>
	if(ch.target_cards) {
		buf.write_bool(true);
		buf.write_card_set(static_cast<group*>(ch.target_cards)->container, maps);
	} else {
		buf.write_bool(false);
	}
	buf.write_effect_id(ch.disable_reason, maps);
	// applied_chain_counters: pointer to vector<uint32_t>
	if(ch.applied_chain_counters) {
		buf.write_bool(true);
		buf.write_u32(static_cast<uint32_t>(ch.applied_chain_counters->size()));
		for(auto v : *ch.applied_chain_counters)
			buf.write_u32(v);
	} else {
		buf.write_bool(false);
	}
	// opmap: unordered_map<uint64_t, optarget>
	auto write_opmap = [&](const chain::opmap& m) {
		buf.write_u32(static_cast<uint32_t>(m.size()));
		for(auto& [k, v] : m) {
			buf.write_u64(k);
			serialize_optarget(buf, v, maps);
		}
	};
	write_opmap(ch.opinfos);
	write_opmap(ch.possibleopinfos);
}

void deserialize_chain(SerializeBuffer& buf, chain& ch, const IdMaps& maps, duel* pduel) {
	deserialize_card_state(buf, ch.triggering_state, maps);
	deserialize_tevent(buf, ch.evt, maps, pduel);
	ch.chain_count = buf.read_u8();
	ch.triggering_player = buf.read_u8();
	ch.triggering_controler = buf.read_u8();
	ch.triggering_position = buf.read_u8();
	ch.target_player = buf.read_u8();
	ch.disable_player = buf.read_u8();
	ch.triggering_summon_location = buf.read_u8();
	ch.triggering_summon_proc_complete = buf.read_bool();
	ch.was_just_sent = buf.read_bool();
	ch.chain_id = buf.read_u16();
	ch.triggering_location = buf.read_u16();
	ch.triggering_sequence = buf.read_u32();
	ch.triggering_status = buf.read_u32();
	ch.triggering_summon_type = buf.read_u32();
	ch.replace_op = buf.read_i32();
	ch.target_param = buf.read_i32();
	ch.flag = buf.read_u32();
	ch.event_id = buf.read_u32();
	ch.triggering_effect = buf.read_effect_id(maps);
	bool has_target = buf.read_bool();
	if(has_target) {
		card_set cs;
		buf.read_card_set(cs, maps);
		ch.target_cards = pduel->new_group(std::move(cs));
	} else {
		ch.target_cards = nullptr;
	}
	ch.disable_reason = buf.read_effect_id(maps);
	bool has_counters = buf.read_bool();
	if(has_counters) {
		uint32_t n = buf.read_u32();
		// applied_chain_counters is a raw pointer -- it points into chain_counter storage
		// For now, we skip restoring it (it's only used during chain resolution)
		// TODO: proper restore if needed
		ch.applied_chain_counters = nullptr;
		for(uint32_t i = 0; i < n; i++)
			buf.read_u32(); // consume
	} else {
		ch.applied_chain_counters = nullptr;
	}
	auto read_opmap = [&](chain::opmap& m) {
		uint32_t cnt = buf.read_u32();
		m.clear();
		for(uint32_t i = 0; i < cnt; i++) {
			uint64_t k = buf.read_u64();
			optarget ot{};
			deserialize_optarget(buf, ot, maps, pduel);
			m[k] = std::move(ot);
		}
	};
	read_opmap(ch.opinfos);
	read_opmap(ch.possibleopinfos);
}

// ============================================================
// Helper: owned_lua<group> serialize/deserialize
// ============================================================
static void serialize_owned_group(SerializeBuffer& buf, const owned_lua<group>& g, const IdMaps& maps) {
	if(g) {
		buf.write_bool(true);
		buf.write_card_set(static_cast<group*>(g)->container, maps);
	} else {
		buf.write_bool(false);
	}
}
static owned_lua<group> deserialize_owned_group(SerializeBuffer& buf, const IdMaps& maps, duel* pduel) {
	if(buf.read_bool()) {
		card_set cs;
		buf.read_card_set(cs, maps);
		return pduel->new_group(std::move(cs));
	}
	return nullptr;
}

// ============================================================
// field_info
// ============================================================
void serialize_field_info(SerializeBuffer& buf, const field_info& fi) {
	buf.write_u32(fi.event_id);
	buf.write_u32(fi.field_id);
	buf.write_u16(fi.copy_id);
	buf.write_i16(fi.turn_id);
	buf.write_i16(fi.turn_id_by_player[0]);
	buf.write_i16(fi.turn_id_by_player[1]);
	buf.write_u32(fi.card_id);
	buf.write_u16(fi.phase);
	buf.write_u8(fi.turn_player);
	buf.write_u8(fi.priorities[0]);
	buf.write_u8(fi.priorities[1]);
	buf.write_bool(fi.can_shuffle);
}
void deserialize_field_info(SerializeBuffer& buf, field_info& fi) {
	fi.event_id = buf.read_u32();
	fi.field_id = buf.read_u32();
	fi.copy_id = buf.read_u16();
	fi.turn_id = buf.read_i16();
	fi.turn_id_by_player[0] = buf.read_i16();
	fi.turn_id_by_player[1] = buf.read_i16();
	fi.card_id = buf.read_u32();
	fi.phase = buf.read_u16();
	fi.turn_player = buf.read_u8();
	fi.priorities[0] = buf.read_u8();
	fi.priorities[1] = buf.read_u8();
	fi.can_shuffle = buf.read_bool();
}

// ============================================================
// player_info
// ============================================================
void serialize_player_info(SerializeBuffer& buf, const player_info& pi, const IdMaps& maps) {
	buf.write_i32(pi.lp);
	buf.write_i32(pi.start_lp);
	buf.write_i32(pi.start_count);
	buf.write_i32(pi.draw_count);
	buf.write_u32(pi.used_location);
	buf.write_u32(pi.disabled_location);
	buf.write_u32(pi.extra_p_count);
	buf.write_u32(pi.exchanges);
	buf.write_u32(pi.tag_index);
	buf.write_bool(pi.recharge);
	buf.write_card_vector(pi.list_mzone, maps);
	buf.write_card_vector(pi.list_szone, maps);
	buf.write_card_vector(pi.list_main, maps);
	buf.write_card_vector(pi.list_grave, maps);
	buf.write_card_vector(pi.list_hand, maps);
	buf.write_card_vector(pi.list_remove, maps);
	buf.write_card_vector(pi.list_extra, maps);
	// extra_lists
	buf.write_u32(static_cast<uint32_t>(pi.extra_lists_main.size()));
	for(auto& v : pi.extra_lists_main)
		buf.write_card_vector(v, maps);
	buf.write_u32(static_cast<uint32_t>(pi.extra_lists_hand.size()));
	for(auto& v : pi.extra_lists_hand)
		buf.write_card_vector(v, maps);
	buf.write_u32(static_cast<uint32_t>(pi.extra_lists_extra.size()));
	for(auto& v : pi.extra_lists_extra)
		buf.write_card_vector(v, maps);
	buf.write_u32(static_cast<uint32_t>(pi.extra_extra_p_count.size()));
	for(auto v : pi.extra_extra_p_count)
		buf.write_u32(v);
}
void deserialize_player_info(SerializeBuffer& buf, player_info& pi, const IdMaps& maps) {
	pi.lp = buf.read_i32();
	pi.start_lp = buf.read_i32();
	pi.start_count = buf.read_i32();
	pi.draw_count = buf.read_i32();
	pi.used_location = buf.read_u32();
	pi.disabled_location = buf.read_u32();
	pi.extra_p_count = buf.read_u32();
	pi.exchanges = buf.read_u32();
	pi.tag_index = buf.read_u32();
	pi.recharge = buf.read_bool();
	buf.read_card_vector(pi.list_mzone, maps);
	buf.read_card_vector(pi.list_szone, maps);
	buf.read_card_vector(pi.list_main, maps);
	buf.read_card_vector(pi.list_grave, maps);
	buf.read_card_vector(pi.list_hand, maps);
	buf.read_card_vector(pi.list_remove, maps);
	buf.read_card_vector(pi.list_extra, maps);
	uint32_t n;
	n = buf.read_u32();
	pi.extra_lists_main.resize(n);
	for(uint32_t i = 0; i < n; i++)
		buf.read_card_vector(pi.extra_lists_main[i], maps);
	n = buf.read_u32();
	pi.extra_lists_hand.resize(n);
	for(uint32_t i = 0; i < n; i++)
		buf.read_card_vector(pi.extra_lists_hand[i], maps);
	n = buf.read_u32();
	pi.extra_lists_extra.resize(n);
	for(uint32_t i = 0; i < n; i++)
		buf.read_card_vector(pi.extra_lists_extra[i], maps);
	n = buf.read_u32();
	pi.extra_extra_p_count.resize(n);
	for(uint32_t i = 0; i < n; i++)
		pi.extra_extra_p_count[i] = buf.read_u32();
}

// ============================================================
// field_effect
// ============================================================
void serialize_field_effect(SerializeBuffer& buf, const field_effect& fe, const IdMaps& maps) {
	buf.write_effect_container(fe.aura_effect, maps);
	buf.write_effect_container(fe.ignition_effect, maps);
	buf.write_effect_container(fe.activate_effect, maps);
	buf.write_effect_container(fe.trigger_o_effect, maps);
	buf.write_effect_container(fe.trigger_f_effect, maps);
	buf.write_effect_container(fe.quick_o_effect, maps);
	buf.write_effect_container(fe.quick_f_effect, maps);
	buf.write_effect_container(fe.continuous_effect, maps);

	// indexer: unordered_map<effect*, effect_container::iterator>
	// Same approach as card indexer -- store (effect_id, key)
	buf.write_u32(static_cast<uint32_t>(fe.indexer.size()));
	for(auto& [eff, it] : fe.indexer) {
		buf.write_effect_id(eff, maps);
		buf.write_u32(it->first);
	}

	// oath: unordered_map<effect*, effect*>
	buf.write_u32(static_cast<uint32_t>(fe.oath.size()));
	for(auto& [k, v] : fe.oath) {
		buf.write_effect_id(k, maps);
		buf.write_effect_id(v, maps);
	}

	// effect_collections (unordered_set<effect*>)
	auto write_eff_coll = [&](const field_effect::effect_collection& c) {
		buf.write_u32(static_cast<uint32_t>(c.size()));
		for(auto* e : c)
			buf.write_effect_id(e, maps);
	};
	write_eff_coll(fe.pheff);
	write_eff_coll(fe.cheff);
	write_eff_coll(fe.rechargeable);
	write_eff_coll(fe.spsummon_count_eff);

	// disable_check_set: card_set
	buf.write_card_set(fe.disable_check_set, maps);

	// grant_effect: complex container -- serialize unsorted map
	buf.write_u32(static_cast<uint32_t>(fe.grant_effect.unsorted.size()));
	for(auto& [eff, gains] : fe.grant_effect.unsorted) {
		buf.write_effect_id(eff, maps);
		buf.write_u32(static_cast<uint32_t>(gains.size()));
		for(auto& [c, e] : gains) {
			buf.write_card_id(c, maps);
			buf.write_effect_id(e, maps);
		}
	}
}

void deserialize_field_effect(SerializeBuffer& buf, field_effect& fe, const IdMaps& maps) {
	buf.read_effect_container(fe.aura_effect, maps);
	buf.read_effect_container(fe.ignition_effect, maps);
	buf.read_effect_container(fe.activate_effect, maps);
	buf.read_effect_container(fe.trigger_o_effect, maps);
	buf.read_effect_container(fe.trigger_f_effect, maps);
	buf.read_effect_container(fe.quick_o_effect, maps);
	buf.read_effect_container(fe.quick_f_effect, maps);
	buf.read_effect_container(fe.continuous_effect, maps);

	// indexer -- rebuild from (effect_id, key) pairs
	uint32_t n = buf.read_u32();
	fe.indexer.clear();
	for(uint32_t i = 0; i < n; i++) {
		effect* eff = buf.read_effect_id(maps);
		uint32_t key = buf.read_u32();
		// Search all 8 containers
		effect_container* conts[] = {
			&fe.aura_effect, &fe.ignition_effect, &fe.activate_effect,
			&fe.trigger_o_effect, &fe.trigger_f_effect, &fe.quick_o_effect,
			&fe.quick_f_effect, &fe.continuous_effect
		};
		for(auto* cont : conts) {
			auto range = cont->equal_range(key);
			for(auto it = range.first; it != range.second; ++it) {
				if(it->second == eff) {
					fe.indexer[eff] = it;
					goto found;
				}
			}
		}
		found:;
	}

	// oath
	n = buf.read_u32();
	fe.oath.clear();
	for(uint32_t i = 0; i < n; i++) {
		effect* k = buf.read_effect_id(maps);
		effect* v = buf.read_effect_id(maps);
		fe.oath[k] = v;
	}

	auto read_eff_coll = [&](field_effect::effect_collection& c) {
		uint32_t cnt = buf.read_u32();
		c.clear();
		for(uint32_t i = 0; i < cnt; i++)
			c.insert(buf.read_effect_id(maps));
	};
	read_eff_coll(fe.pheff);
	read_eff_coll(fe.cheff);
	read_eff_coll(fe.rechargeable);
	read_eff_coll(fe.spsummon_count_eff);

	buf.read_card_set(fe.disable_check_set, maps);

	// grant_effect
	n = buf.read_u32();
	// Clear the grant_effect by clearing both containers
	fe.grant_effect.unsorted.clear();
	fe.grant_effect.sorted.clear();
	for(uint32_t i = 0; i < n; i++) {
		effect* eff = buf.read_effect_id(maps);
		uint32_t gc = buf.read_u32();
		field_effect::gain_effects gains;
		for(uint32_t j = 0; j < gc; j++) {
			card* c = buf.read_card_id(maps);
			effect* e = buf.read_effect_id(maps);
			gains[c] = e;
		}
		fe.grant_effect.emplace(eff, std::move(gains));
	}
}

// ============================================================
// processor_unit serialization
// ============================================================
// Variant type indices are used as discriminants.
// We use std::visit for serialize and a big switch for deserialize.

using namespace Processors;

void serialize_processor_unit(SerializeBuffer& buf, const processor_unit& unit, const IdMaps& maps) {
	// Write the variant index
	buf.write_u16(static_cast<uint16_t>(unit.index()));

	std::visit([&](auto& arg) {
		using T = std::decay_t<decltype(arg)>;
		// All types have step
		buf.write_u16(arg.step);

		if constexpr(std::is_same_v<T, Adjust>) {
			// step only
		} else if constexpr(std::is_same_v<T, Turn>) {
			buf.write_u8(arg.turn_player);
			buf.write_bool(arg.has_performed_second_battle_phase);
		} else if constexpr(std::is_same_v<T, RefreshLoc>) {
			buf.write_u8(arg.dis_count);
			buf.write_u32(arg.previously_disabled_locations);
			buf.write_effect_id(arg.current_disable_field_effect, maps);
		} else if constexpr(std::is_same_v<T, Startup>) {
			// step only
		} else if constexpr(std::is_same_v<T, SelectBattleCmd>) {
			buf.write_u8(arg.playerid);
		} else if constexpr(std::is_same_v<T, SelectIdleCmd>) {
			buf.write_u8(arg.playerid);
		} else if constexpr(std::is_same_v<T, SelectEffectYesNo>) {
			buf.write_u8(arg.playerid);
			buf.write_card_id(arg.pcard, maps);
			buf.write_u64(arg.description);
		} else if constexpr(std::is_same_v<T, SelectYesNo>) {
			buf.write_u8(arg.playerid);
			buf.write_u64(arg.description);
		} else if constexpr(std::is_same_v<T, SelectOption>) {
			buf.write_u8(arg.playerid);
		} else if constexpr(std::is_same_v<T, SelectCard>) {
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.cancelable);
			buf.write_u8(arg.min);
			buf.write_u8(arg.max);
		} else if constexpr(std::is_same_v<T, SelectCardCodes>) {
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.cancelable);
			buf.write_u8(arg.min);
			buf.write_u8(arg.max);
		} else if constexpr(std::is_same_v<T, SelectUnselectCard>) {
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.cancelable);
			buf.write_u8(arg.min);
			buf.write_u8(arg.max);
			buf.write_bool(arg.finishable);
		} else if constexpr(std::is_same_v<T, SelectChain>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.spe_count);
			buf.write_bool(arg.forced);
		} else if constexpr(std::is_same_v<T, SelectPlace>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.count);
			buf.write_u32(arg.flag);
			buf.write_bool(arg.disable_field);
		} else if constexpr(std::is_same_v<T, SelectPosition>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.positions);
			buf.write_u32(arg.code);
		} else if constexpr(std::is_same_v<T, SelectTributeP>) {
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.cancelable);
			buf.write_u8(arg.min);
			buf.write_u8(arg.max);
		} else if constexpr(std::is_same_v<T, SortChain>) {
			buf.write_u8(arg.playerid);
		} else if constexpr(std::is_same_v<T, SelectCounter>) {
			buf.write_u16(arg.countertype);
			buf.write_u16(arg.count);
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.self);
			buf.write_u8(arg.oppo);
		} else if constexpr(std::is_same_v<T, SelectSum>) {
			buf.write_u8(arg.playerid);
			buf.write_i32(arg.acc);
			buf.write_i32(arg.min);
			buf.write_i32(arg.max);
		} else if constexpr(std::is_same_v<T, SortCard>) {
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.is_chain);
		} else if constexpr(std::is_same_v<T, SelectRelease>) {
			buf.write_u16(arg.min);
			buf.write_u16(arg.max);
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.cancelable);
			buf.write_bool(arg.check_field);
			buf.write_u8(arg.toplayer);
			buf.write_u8(arg.zone);
			buf.write_card_id(arg.to_check, maps);
			buf.write_effect_id(arg.extra_release_nonsum_effect, maps);
			if(arg.must_choose_one) {
				buf.write_bool(true);
				buf.write_card_set(*arg.must_choose_one, maps);
			} else {
				buf.write_bool(false);
			}
		} else if constexpr(std::is_same_v<T, SelectTribute>) {
			buf.write_u16(arg.min);
			buf.write_u16(arg.max);
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.cancelable);
			buf.write_u8(arg.toplayer);
			buf.write_u8(arg.zone);
			buf.write_card_id(arg.target, maps);
			buf.write_effect_id(arg.extra_release_effect, maps);
			buf.write_card_set(arg.must_choose_one, maps);
		} else if constexpr(std::is_same_v<T, PointEvent>) {
			buf.write_bool(arg.skip_trigger);
			buf.write_bool(arg.skip_freechain);
			buf.write_bool(arg.skip_new);
		} else if constexpr(std::is_same_v<T, QuickEffect>) {
			buf.write_bool(arg.skip_freechain);
			buf.write_bool(arg.is_opponent);
			buf.write_u8(arg.priority_player);
		} else if constexpr(std::is_same_v<T, IdleCommand>) {
			buf.write_u8(arg.phase_to_change_to);
			buf.write_card_id(arg.card_to_reposition, maps);
		} else if constexpr(std::is_same_v<T, PhaseEvent>) {
			buf.write_u16(arg.phase);
			buf.write_bool(arg.is_opponent);
			buf.write_bool(arg.priority_passed);
		} else if constexpr(std::is_same_v<T, BattleCommand>) {
			buf.write_u16(arg.phase_to_change_to);
			buf.write_bool(arg.forced_attack);
			buf.write_bool(arg.forced_attack_done);
			buf.write_bool(arg.is_replaying_attack);
			buf.write_bool(arg.attack_announce_failed);
			buf.write_bool(arg.repeat_battle_phase);
			buf.write_bool(arg.second_battle_phase_is_optional);
			buf.write_bool(arg.previous_point_event_had_any_trigger_to_resolve);
			buf.write_u8(arg.reason_player);
			buf.write_effect_id(arg.damage_change_effect, maps);
			serialize_owned_group(buf, arg.cards_destroyed_by_battle, maps);
			buf.write_card_id(arg.reason_card, maps);
			// must_attack_map: multimap<effect*, card*>
			buf.write_u32(static_cast<uint32_t>(arg.must_attack_map.size()));
			for(auto& [e, c] : arg.must_attack_map) {
				buf.write_effect_id(e, maps);
				buf.write_card_id(c, maps);
			}
		} else if constexpr(std::is_same_v<T, DamageStep>) {
			buf.write_u16(arg.backup_phase);
			buf.write_bool(arg.new_attack);
			buf.write_card_id(arg.attacker, maps);
			buf.write_card_id(arg.attack_target, maps);
			serialize_owned_group(buf, arg.cards_destroyed_by_battle, maps);
		} else if constexpr(std::is_same_v<T, ForcedBattle>) {
			buf.write_u16(arg.backup_phase);
		} else if constexpr(std::is_same_v<T, AddChain>) {
			buf.write_bool(arg.is_activated_effect);
		} else if constexpr(std::is_same_v<T, SolveChain>) {
			buf.write_bool(arg.skip_trigger);
			buf.write_bool(arg.skip_freechain);
			buf.write_bool(arg.skip_new);
			buf.write_i32(arg.backed_up_operation);
		} else if constexpr(std::is_same_v<T, SolveContinuous>) {
			buf.write_u8(arg.reason_player);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, ExecuteCost>) {
			buf.write_u8(arg.triggering_player);
			buf.write_bool(arg.shuffle_check_was_disabled);
			buf.write_effect_id(arg.triggering_effect, maps);
		} else if constexpr(std::is_same_v<T, ExecuteOperation>) {
			buf.write_u8(arg.triggering_player);
			buf.write_bool(arg.shuffle_check_was_disabled);
			buf.write_effect_id(arg.triggering_effect, maps);
		} else if constexpr(std::is_same_v<T, ExecuteTarget>) {
			buf.write_u8(arg.triggering_player);
			buf.write_bool(arg.shuffle_check_was_disabled);
			buf.write_effect_id(arg.triggering_effect, maps);
		} else if constexpr(std::is_same_v<T, Destroy>) {
			buf.write_u8(arg.reason_player);
			buf.write_u32(arg.reason);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, Release>) {
			buf.write_u8(arg.reason_player);
			buf.write_u32(arg.reason);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, SendTo>) {
			buf.write_u8(arg.reason_player);
			buf.write_u32(arg.reason);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_effect_id(arg.reason_effect, maps);
			if(arg.extra_args) {
				buf.write_bool(true);
				buf.write_card_set(arg.extra_args->leave_field, maps);
				buf.write_card_set(arg.extra_args->leave_grave, maps);
				buf.write_card_set(arg.extra_args->detach, maps);
				buf.write_bool(arg.extra_args->check_decktop_visibility[0]);
				buf.write_bool(arg.extra_args->check_decktop_visibility[1]);
				buf.write_card_vector(arg.extra_args->cv, maps);
				// cvit is an iterator into cv -- store index
				uint32_t cvit_idx = 0;
				if(!arg.extra_args->cv.empty()) {
					cvit_idx = static_cast<uint32_t>(
						std::distance(arg.extra_args->cv.begin(), arg.extra_args->cvit));
				}
				buf.write_u32(cvit_idx);
				buf.write_effect_id(arg.extra_args->predirect, maps);
			} else {
				buf.write_bool(false);
			}
		} else if constexpr(std::is_same_v<T, DestroyReplace>) {
			buf.write_bool(arg.battle);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_card_id(arg.target, maps);
		} else if constexpr(std::is_same_v<T, ReleaseReplace>) {
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_card_id(arg.target, maps);
		} else if constexpr(std::is_same_v<T, SendToReplace>) {
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_card_id(arg.target, maps);
		} else if constexpr(std::is_same_v<T, MoveToField>) {
			buf.write_bool(arg.enable);
			buf.write_u8(arg.ret);
			buf.write_bool(arg.pzone);
			buf.write_u8(arg.zone);
			buf.write_bool(arg.rule);
			buf.write_u8(arg.location_reason);
			buf.write_bool(arg.confirm);
			buf.write_card_id(arg.target, maps);
		} else if constexpr(std::is_same_v<T, ChangePos>) {
			buf.write_u8(arg.reason_player);
			buf.write_bool(arg.enable);
			buf.write_bool(arg.oppo_selection);
			buf.write_effect_id(arg.reason_effect, maps);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_card_set(arg.to_grave_set, maps);
		} else if constexpr(std::is_same_v<T, OperationReplace>) {
			buf.write_bool(arg.is_destroy);
			buf.write_effect_id(arg.replace_effect, maps);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_card_id(arg.target, maps);
		} else if constexpr(std::is_same_v<T, ActivateEffect>) {
			buf.write_effect_id(arg.peffect, maps);
		} else if constexpr(std::is_same_v<T, SummonRule>) {
			buf.write_u8(arg.sumplayer);
			buf.write_u8(arg.min_tribute);
			buf.write_u8(arg.max_allowed_tributes);
			buf.write_bool(arg.ignore_count);
			buf.write_u32(arg.zone);
			buf.write_card_id(arg.target, maps);
			buf.write_effect_id(arg.summon_procedure_effect, maps);
			buf.write_effect_id(arg.extra_summon_effect, maps);
			buf.write_card_set(arg.tributes, maps);
			buf.write_effect_set(arg.summon_cost_effects, maps);
		} else if constexpr(std::is_same_v<T, SpSummonRule>) {
			buf.write_u8(arg.sumplayer);
			buf.write_bool(arg.is_mid_chain);
			buf.write_u32(arg.summon_type);
			buf.write_card_id(arg.target, maps);
			buf.write_effect_id(arg.summon_proc_effect, maps);
			serialize_owned_group(buf, arg.cards_to_summon_g, maps);
			buf.write_effect_set(arg.spsummon_cost_effects, maps);
		} else if constexpr(std::is_same_v<T, SpSummon>) {
			buf.write_u8(arg.reason_player);
			buf.write_u32(arg.zone);
			buf.write_effect_id(arg.reason_effect, maps);
			serialize_owned_group(buf, arg.targets, maps);
		} else if constexpr(std::is_same_v<T, FlipSummon>) {
			buf.write_u8(arg.sumplayer);
			buf.write_card_id(arg.target, maps);
			buf.write_effect_set(arg.flip_summon_cost_effects, maps);
		} else if constexpr(std::is_same_v<T, MonsterSet>) {
			buf.write_u8(arg.setplayer);
			buf.write_u8(arg.min_tribute);
			buf.write_u8(arg.max_allowed_tributes);
			buf.write_bool(arg.ignore_count);
			buf.write_u32(arg.zone);
			buf.write_card_id(arg.target, maps);
			buf.write_effect_id(arg.summon_procedure_effect, maps);
			buf.write_effect_id(arg.extra_summon_effect, maps);
			buf.write_card_set(arg.tributes, maps);
		} else if constexpr(std::is_same_v<T, SpellSet>) {
			buf.write_u8(arg.setplayer);
			buf.write_u8(arg.toplayer);
			buf.write_card_id(arg.target, maps);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, SpSummonStep>) {
			buf.write_u32(arg.zone);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_card_id(arg.target, maps);
			buf.write_effect_set(arg.spsummon_cost_effects, maps);
		} else if constexpr(std::is_same_v<T, SpellSetGroup>) {
			buf.write_u8(arg.setplayer);
			buf.write_u8(arg.toplayer);
			buf.write_bool(arg.confirm);
			serialize_owned_group(buf, arg.ptarget, maps);
			buf.write_effect_id(arg.reason_effect, maps);
			buf.write_card_set(arg.set_cards, maps);
		} else if constexpr(std::is_same_v<T, SpSummonRuleGroup>) {
			buf.write_u8(arg.sumplayer);
			buf.write_u32(arg.summon_type);
		} else if constexpr(std::is_same_v<T, Draw>) {
			buf.write_u16(arg.count);
			buf.write_u8(arg.reason_player);
			buf.write_u8(arg.playerid);
			buf.write_u32(arg.reason);
			buf.write_effect_id(arg.reason_effect, maps);
			buf.write_card_set(arg.drawn_set, maps);
		} else if constexpr(std::is_same_v<T, Damage>) {
			buf.write_u8(arg.reason_player);
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.is_step);
			buf.write_bool(arg.is_reflected);
			buf.write_u32(arg.amount);
			buf.write_u32(arg.reason);
			buf.write_card_id(arg.reason_card, maps);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, Recover>) {
			buf.write_u8(arg.reason_player);
			buf.write_u8(arg.playerid);
			buf.write_bool(arg.is_step);
			buf.write_u32(arg.amount);
			buf.write_u32(arg.reason);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, Equip>) {
			buf.write_u8(arg.equip_player);
			buf.write_bool(arg.is_step);
			buf.write_bool(arg.faceup);
			buf.write_card_id(arg.equip_card, maps);
			buf.write_card_id(arg.target, maps);
		} else if constexpr(std::is_same_v<T, GetControl>) {
			buf.write_u8(arg.chose_player);
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.reset_count);
			buf.write_u16(arg.reset_phase);
			buf.write_u32(arg.zone);
			buf.write_effect_id(arg.reason_effect, maps);
			serialize_owned_group(buf, arg.targets, maps);
			buf.write_card_set(arg.destroy_set, maps);
		} else if constexpr(std::is_same_v<T, SwapControl>) {
			buf.write_u8(arg.reset_count);
			buf.write_u8(arg.reason_player);
			buf.write_u8(arg.self_selected_sequence);
			buf.write_u16(arg.reset_phase);
			buf.write_effect_id(arg.reason_effect, maps);
			serialize_owned_group(buf, arg.targets1, maps);
			serialize_owned_group(buf, arg.targets2, maps);
		} else if constexpr(std::is_same_v<T, ControlAdjust>) {
			buf.write_u8(arg.adjusting_player);
			buf.write_card_set(arg.destroy_set, maps);
			buf.write_card_set(arg.adjust_set, maps);
		} else if constexpr(std::is_same_v<T, SelfDestroyUnique>) {
			buf.write_u8(arg.playerid);
			buf.write_card_id(arg.unique_card, maps);
		} else if constexpr(std::is_same_v<T, SelfDestroy>) {
			// step only
		} else if constexpr(std::is_same_v<T, SelfToGrave>) {
			// step only
		} else if constexpr(std::is_same_v<T, TrapMonsterAdjust>) {
			buf.write_bool(arg.oppo_selection);
			buf.write_card_set(arg.to_grave_set, maps);
		} else if constexpr(std::is_same_v<T, PayLPCost>) {
			buf.write_u8(arg.playerid);
			buf.write_u32(arg.cost);
		} else if constexpr(std::is_same_v<T, RemoveCounter>) {
			buf.write_u8(arg.rplayer);
			buf.write_u8(arg.self);
			buf.write_u8(arg.oppo);
			buf.write_u16(arg.countertype);
			buf.write_u16(arg.count);
			buf.write_u32(arg.reason);
			buf.write_card_id(arg.pcard, maps);
		} else if constexpr(std::is_same_v<T, AttackDisable>) {
			// step only
		} else if constexpr(std::is_same_v<T, AnnounceRace>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.count);
			buf.write_u64(arg.available);
		} else if constexpr(std::is_same_v<T, AnnounceAttribute>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.count);
			buf.write_u32(arg.available);
		} else if constexpr(std::is_same_v<T, AnnounceCard>) {
			buf.write_u8(arg.playerid);
		} else if constexpr(std::is_same_v<T, AnnounceNumber>) {
			buf.write_u8(arg.playerid);
		} else if constexpr(std::is_same_v<T, TossCoin>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.reason_player);
			buf.write_u8(arg.count);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, TossDice>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.reason_player);
			buf.write_u8(arg.count1);
			buf.write_u8(arg.count2);
			buf.write_effect_id(arg.reason_effect, maps);
		} else if constexpr(std::is_same_v<T, RockPaperScissors>) {
			buf.write_bool(arg.repeat);
			buf.write_u8(arg.hand0);
		} else if constexpr(std::is_same_v<T, SelectFusion>) {
			buf.write_u8(arg.playerid);
			buf.write_u32(arg.chkf);
			serialize_owned_group(buf, arg.fusion_materials, maps);
			serialize_owned_group(buf, arg.forced_materials, maps);
			buf.write_card_id(arg.pcard, maps);
		} else if constexpr(std::is_same_v<T, DiscardHand>) {
			buf.write_u8(arg.playerid);
			buf.write_u8(arg.min);
			buf.write_u8(arg.max);
			buf.write_u32(arg.reason);
		} else if constexpr(std::is_same_v<T, DiscardDeck>) {
			buf.write_u16(arg.count);
			buf.write_u8(arg.playerid);
			buf.write_u32(arg.reason);
		} else if constexpr(std::is_same_v<T, SortDeck>) {
			buf.write_u16(arg.count);
			buf.write_u8(arg.sort_player);
			buf.write_u8(arg.target_player);
			buf.write_bool(arg.bottom);
		} else if constexpr(std::is_same_v<T, RemoveOverlay>) {
			buf.write_u16(arg.min);
			buf.write_u16(arg.max);
			buf.write_u16(arg.replaced_amount);
			buf.write_bool(arg.has_used_overlay_remove_replace_effect);
			buf.write_u8(arg.rplayer);
			buf.write_u8(arg.self);
			buf.write_u8(arg.oppo);
			buf.write_u32(arg.reason);
			serialize_owned_group(buf, arg.pgroup, maps);
		} else if constexpr(std::is_same_v<T, XyzOverlay>) {
			buf.write_bool(arg.send_materials_to_grave);
			buf.write_card_id(arg.target, maps);
			serialize_owned_group(buf, arg.materials, maps);
		} else if constexpr(std::is_same_v<T, RefreshRelay>) {
			// step only
		} else {
			static_assert(sizeof(T) == 0, "Unhandled processor_unit type in serialize");
		}
	}, unit);
}

processor_unit deserialize_processor_unit(SerializeBuffer& buf, const IdMaps& maps, duel* pduel) {
	uint16_t idx = buf.read_u16();
	uint16_t step = buf.read_u16();

	// Helper lambdas for common patterns
	auto read_owned_group = [&]() -> owned_lua<group> {
		return deserialize_owned_group(buf, maps, pduel);
	};

	switch(idx) {
	case 0: // Adjust
		return Adjust(step);
	case 1: { // Turn
		uint8_t tp = buf.read_u8();
		bool bp = buf.read_bool();
		auto r = Turn(step, tp);
		r.has_performed_second_battle_phase = bp;
		return r;
	}
	case 2: { // RefreshLoc
		auto r = RefreshLoc(step);
		r.dis_count = buf.read_u8();
		r.previously_disabled_locations = buf.read_u32();
		r.current_disable_field_effect = buf.read_effect_id(maps);
		return r;
	}
	case 3: // Startup
		return Startup(step);
	case 4: // SelectBattleCmd
		return SelectBattleCmd(step, buf.read_u8());
	case 5: // SelectIdleCmd
		return SelectIdleCmd(step, buf.read_u8());
	case 6: { // SelectEffectYesNo
		uint8_t pid = buf.read_u8();
		card* pc = buf.read_card_id(maps);
		uint64_t desc = buf.read_u64();
		return SelectEffectYesNo(step, pid, desc, pc);
	}
	case 7: { // SelectYesNo
		uint8_t pid = buf.read_u8();
		uint64_t desc = buf.read_u64();
		return SelectYesNo(step, pid, desc);
	}
	case 8: // SelectOption
		return SelectOption(step, buf.read_u8());
	case 9: { // SelectCard
		uint8_t pid = buf.read_u8();
		bool c = buf.read_bool();
		uint8_t mn = buf.read_u8();
		uint8_t mx = buf.read_u8();
		return SelectCard(step, pid, c, mn, mx);
	}
	case 10: { // SelectCardCodes
		uint8_t pid = buf.read_u8();
		bool c = buf.read_bool();
		uint8_t mn = buf.read_u8();
		uint8_t mx = buf.read_u8();
		return SelectCardCodes(step, pid, c, mn, mx);
	}
	case 11: { // SelectUnselectCard
		uint8_t pid = buf.read_u8();
		bool c = buf.read_bool();
		uint8_t mn = buf.read_u8();
		uint8_t mx = buf.read_u8();
		bool f = buf.read_bool();
		return SelectUnselectCard(step, pid, c, mn, mx, f);
	}
	case 12: { // SelectChain
		uint8_t pid = buf.read_u8();
		uint8_t sc = buf.read_u8();
		bool f = buf.read_bool();
		return SelectChain(step, pid, sc, f);
	}
	case 13: { // SelectPlace
		uint8_t pid = buf.read_u8();
		uint8_t cnt = buf.read_u8();
		uint32_t fl = buf.read_u32();
		bool df = buf.read_bool();
		if(df) {
			return SelectDisField(step, pid, fl, cnt);
		}
		return SelectPlace(step, pid, fl, cnt);
	}
	case 14: { // SelectPosition
		uint8_t pid = buf.read_u8();
		uint8_t pos = buf.read_u8();
		uint32_t code = buf.read_u32();
		return SelectPosition(step, pid, code, pos);
	}
	case 15: { // SelectTributeP
		uint8_t pid = buf.read_u8();
		bool c = buf.read_bool();
		uint8_t mn = buf.read_u8();
		uint8_t mx = buf.read_u8();
		return SelectTributeP(step, pid, c, mn, mx);
	}
	case 16: // SortChain
		return SortChain(step, buf.read_u8());
	case 17: { // SelectCounter
		uint16_t ct = buf.read_u16();
		uint16_t cnt = buf.read_u16();
		uint8_t pid = buf.read_u8();
		uint8_t s = buf.read_u8();
		uint8_t o = buf.read_u8();
		return SelectCounter(step, pid, ct, cnt, s, o);
	}
	case 18: { // SelectSum
		uint8_t pid = buf.read_u8();
		int32_t acc = buf.read_i32();
		int32_t mn = buf.read_i32();
		int32_t mx = buf.read_i32();
		return SelectSum(step, pid, acc, mn, mx);
	}
	case 19: { // SortCard
		uint8_t pid = buf.read_u8();
		bool ic = buf.read_bool();
		return SortCard(step, pid, ic);
	}
	case 20: { // SelectRelease
		uint16_t mn = buf.read_u16();
		uint16_t mx = buf.read_u16();
		uint8_t pid = buf.read_u8();
		bool c = buf.read_bool();
		bool cf = buf.read_bool();
		uint8_t tp = buf.read_u8();
		uint8_t z = buf.read_u8();
		card* tc = buf.read_card_id(maps);
		auto r = SelectRelease(step, pid, c, mn, mx, cf, tc, tp, z);
		r.extra_release_nonsum_effect = buf.read_effect_id(maps);
		if(buf.read_bool()) {
			r.must_choose_one = std::make_unique<card_set>();
			buf.read_card_set(*r.must_choose_one, maps);
		}
		return r;
	}
	case 21: { // SelectTribute
		uint16_t mn = buf.read_u16();
		uint16_t mx = buf.read_u16();
		uint8_t pid = buf.read_u8();
		bool c = buf.read_bool();
		uint8_t tp = buf.read_u8();
		uint8_t z = buf.read_u8();
		card* tgt = buf.read_card_id(maps);
		auto r = SelectTribute(step, tgt, pid, c, mn, mx, tp, z);
		r.extra_release_effect = buf.read_effect_id(maps);
		buf.read_card_set(r.must_choose_one, maps);
		return r;
	}
	case 22: { // QuickEffect
		bool sf = buf.read_bool();
		bool io = buf.read_bool();
		uint8_t pp = buf.read_u8();
		auto r = QuickEffect(step, sf, pp);
		r.is_opponent = io;
		return r;
	}
	case 23: { // IdleCommand
		auto r = IdleCommand(step);
		r.phase_to_change_to = buf.read_u8();
		r.card_to_reposition = buf.read_card_id(maps);
		return r;
	}
	case 24: { // PhaseEvent
		uint16_t ph = buf.read_u16();
		bool io = buf.read_bool();
		bool pp = buf.read_bool();
		auto r = PhaseEvent(step, ph);
		r.is_opponent = io;
		r.priority_passed = pp;
		return r;
	}
	case 25: { // PointEvent
		bool st = buf.read_bool();
		bool sf = buf.read_bool();
		bool sn = buf.read_bool();
		return PointEvent(step, st, sf, sn);
	}
	case 26: { // BattleCommand
		auto r = BattleCommand(step);
		r.phase_to_change_to = buf.read_u16();
		r.forced_attack = buf.read_bool();
		r.forced_attack_done = buf.read_bool();
		r.is_replaying_attack = buf.read_bool();
		r.attack_announce_failed = buf.read_bool();
		r.repeat_battle_phase = buf.read_bool();
		r.second_battle_phase_is_optional = buf.read_bool();
		r.previous_point_event_had_any_trigger_to_resolve = buf.read_bool();
		r.reason_player = buf.read_u8();
		r.damage_change_effect = buf.read_effect_id(maps);
		r.cards_destroyed_by_battle = read_owned_group();
		r.reason_card = buf.read_card_id(maps);
		uint32_t mam_n = buf.read_u32();
		for(uint32_t i = 0; i < mam_n; i++) {
			effect* e = buf.read_effect_id(maps);
			card* c = buf.read_card_id(maps);
			r.must_attack_map.emplace(e, c);
		}
		return r;
	}
	case 27: { // DamageStep
		uint16_t bp = buf.read_u16();
		bool na = buf.read_bool();
		card* att = buf.read_card_id(maps);
		card* at = buf.read_card_id(maps);
		auto r = DamageStep(step, att, at, na);
		r.backup_phase = bp;
		r.cards_destroyed_by_battle = read_owned_group();
		return r;
	}
	case 28: { // ForcedBattle
		auto r = ForcedBattle(step);
		r.backup_phase = buf.read_u16();
		return r;
	}
	case 29: { // AddChain
		auto r = AddChain(step);
		r.is_activated_effect = buf.read_bool();
		return r;
	}
	case 30: { // SolveChain
		bool st = buf.read_bool();
		bool sf = buf.read_bool();
		bool sn = buf.read_bool();
		auto r = SolveChain(step, st, sf, sn);
		r.backed_up_operation = buf.read_i32();
		return r;
	}
	case 31: { // SolveContinuous
		auto r = SolveContinuous(step);
		r.reason_player = buf.read_u8();
		r.reason_effect = buf.read_effect_id(maps);
		return r;
	}
	case 32: { // ExecuteCost
		uint8_t tp = buf.read_u8();
		bool sd = buf.read_bool();
		effect* te = buf.read_effect_id(maps);
		auto r = ExecuteCost(step, te, tp);
		r.shuffle_check_was_disabled = sd;
		return r;
	}
	case 33: { // ExecuteOperation
		uint8_t tp = buf.read_u8();
		bool sd = buf.read_bool();
		effect* te = buf.read_effect_id(maps);
		auto r = ExecuteOperation(step, te, tp);
		r.shuffle_check_was_disabled = sd;
		return r;
	}
	case 34: { // ExecuteTarget
		uint8_t tp = buf.read_u8();
		bool sd = buf.read_bool();
		effect* te = buf.read_effect_id(maps);
		auto r = ExecuteTarget(step, te, tp);
		r.shuffle_check_was_disabled = sd;
		return r;
	}
	case 35: { // Destroy
		uint8_t rp = buf.read_u8();
		uint32_t rs = buf.read_u32();
		auto tg = read_owned_group();
		effect* re = buf.read_effect_id(maps);
		return Destroy(step, std::move(tg), re, rs, rp);
	}
	case 36: { // Release
		uint8_t rp = buf.read_u8();
		uint32_t rs = buf.read_u32();
		auto tg = read_owned_group();
		effect* re = buf.read_effect_id(maps);
		return Release(step, std::move(tg), re, rs, rp);
	}
	case 37: { // SendTo
		uint8_t rp = buf.read_u8();
		uint32_t rs = buf.read_u32();
		auto tg = read_owned_group();
		effect* re = buf.read_effect_id(maps);
		auto r = SendTo(step, std::move(tg), re, rs, rp);
		if(buf.read_bool()) {
			r.extra_args = std::make_unique<SendTo::exargs>();
			buf.read_card_set(r.extra_args->leave_field, maps);
			buf.read_card_set(r.extra_args->leave_grave, maps);
			buf.read_card_set(r.extra_args->detach, maps);
			r.extra_args->check_decktop_visibility[0] = buf.read_bool();
			r.extra_args->check_decktop_visibility[1] = buf.read_bool();
			buf.read_card_vector(r.extra_args->cv, maps);
			uint32_t cvit_idx = buf.read_u32();
			r.extra_args->cvit = r.extra_args->cv.begin() + cvit_idx;
			r.extra_args->predirect = buf.read_effect_id(maps);
		}
		return r;
	}
	case 38: { // DestroyReplace
		bool b = buf.read_bool();
		auto tg = read_owned_group();
		card* t = buf.read_card_id(maps);
		return DestroyReplace(step, std::move(tg), t, b);
	}
	case 39: { // ReleaseReplace
		auto tg = read_owned_group();
		card* t = buf.read_card_id(maps);
		return ReleaseReplace(step, std::move(tg), t);
	}
	case 40: { // SendToReplace
		auto tg = read_owned_group();
		card* t = buf.read_card_id(maps);
		return SendToReplace(step, std::move(tg), t);
	}
	case 41: { // MoveToField
		bool en = buf.read_bool();
		uint8_t rt = buf.read_u8();
		bool pz = buf.read_bool();
		uint8_t zn = buf.read_u8();
		bool rl = buf.read_bool();
		uint8_t lr = buf.read_u8();
		bool cf = buf.read_bool();
		card* tgt = buf.read_card_id(maps);
		return MoveToField(step, tgt, en, rt, pz, zn, rl, lr, cf);
	}
	case 42: { // ChangePos
		uint8_t rp = buf.read_u8();
		bool en = buf.read_bool();
		bool os = buf.read_bool();
		effect* re = buf.read_effect_id(maps);
		auto tg = read_owned_group();
		auto r = ChangePos(step, std::move(tg), re, rp, en);
		r.oppo_selection = os;
		buf.read_card_set(r.to_grave_set, maps);
		return r;
	}
	case 43: { // OperationReplace
		bool id = buf.read_bool();
		effect* re = buf.read_effect_id(maps);
		auto tg = read_owned_group();
		card* t = buf.read_card_id(maps);
		return OperationReplace(step, re, std::move(tg), t, id);
	}
	case 44: { // ActivateEffect
		effect* pe = buf.read_effect_id(maps);
		return ActivateEffect(step, pe);
	}
	case 45: { // SummonRule
		uint8_t sp = buf.read_u8();
		uint8_t mt = buf.read_u8();
		uint8_t mat = buf.read_u8();
		bool ic = buf.read_bool();
		uint32_t zn = buf.read_u32();
		card* tgt = buf.read_card_id(maps);
		effect* spe = buf.read_effect_id(maps);
		effect* ese = buf.read_effect_id(maps);
		auto r = SummonRule(step, sp, tgt, spe, ic, mt, zn);
		r.max_allowed_tributes = mat;
		r.extra_summon_effect = ese;
		buf.read_card_set(r.tributes, maps);
		buf.read_effect_set(r.summon_cost_effects, maps);
		return r;
	}
	case 46: { // SpSummonRule
		uint8_t sp = buf.read_u8();
		bool imc = buf.read_bool();
		uint32_t st2 = buf.read_u32();
		card* tgt = buf.read_card_id(maps);
		effect* spe = buf.read_effect_id(maps);
		auto r = SpSummonRule(step, sp, tgt, st2, imc, spe);
		r.cards_to_summon_g = read_owned_group();
		buf.read_effect_set(r.spsummon_cost_effects, maps);
		return r;
	}
	case 47: { // SpSummon
		uint8_t rp = buf.read_u8();
		uint32_t zn = buf.read_u32();
		effect* re = buf.read_effect_id(maps);
		auto tg = read_owned_group();
		return SpSummon(step, re, rp, std::move(tg), zn);
	}
	case 48: { // FlipSummon
		uint8_t sp = buf.read_u8();
		card* tgt = buf.read_card_id(maps);
		auto r = FlipSummon(step, sp, tgt);
		buf.read_effect_set(r.flip_summon_cost_effects, maps);
		return r;
	}
	case 49: { // MonsterSet
		uint8_t sp = buf.read_u8();
		uint8_t mt = buf.read_u8();
		uint8_t mat = buf.read_u8();
		bool ic = buf.read_bool();
		uint32_t zn = buf.read_u32();
		card* tgt = buf.read_card_id(maps);
		effect* spe = buf.read_effect_id(maps);
		effect* ese = buf.read_effect_id(maps);
		auto r = MonsterSet(step, sp, tgt, spe, ic, mt, zn);
		r.max_allowed_tributes = mat;
		r.extra_summon_effect = ese;
		buf.read_card_set(r.tributes, maps);
		return r;
	}
	case 50: { // SpellSet
		uint8_t sp = buf.read_u8();
		uint8_t tp = buf.read_u8();
		card* tgt = buf.read_card_id(maps);
		effect* re = buf.read_effect_id(maps);
		return SpellSet(step, sp, tp, tgt, re);
	}
	case 51: { // SpSummonStep
		uint32_t zn = buf.read_u32();
		auto tg = read_owned_group();
		card* tgt = buf.read_card_id(maps);
		auto r = SpSummonStep(step, std::move(tg), tgt, zn);
		buf.read_effect_set(r.spsummon_cost_effects, maps);
		return r;
	}
	case 52: { // SpellSetGroup
		uint8_t sp = buf.read_u8();
		uint8_t tp = buf.read_u8();
		bool cf = buf.read_bool();
		auto pt = read_owned_group();
		effect* re = buf.read_effect_id(maps);
		auto r = SpellSetGroup(step, sp, tp, std::move(pt), cf, re);
		buf.read_card_set(r.set_cards, maps);
		return r;
	}
	case 53: { // SpSummonRuleGroup
		uint8_t sp = buf.read_u8();
		uint32_t st2 = buf.read_u32();
		return SpSummonRuleGroup(step, sp, st2);
	}
	case 54: { // Draw
		uint16_t cnt = buf.read_u16();
		uint8_t rp = buf.read_u8();
		uint8_t pid = buf.read_u8();
		uint32_t rs = buf.read_u32();
		effect* re = buf.read_effect_id(maps);
		auto r = Draw(step, re, rs, rp, pid, cnt);
		buf.read_card_set(r.drawn_set, maps);
		return r;
	}
	case 55: { // Damage
		uint8_t rp = buf.read_u8();
		uint8_t pid = buf.read_u8();
		bool is = buf.read_bool();
		bool ir = buf.read_bool();
		uint32_t amt = buf.read_u32();
		uint32_t rs = buf.read_u32();
		card* rc = buf.read_card_id(maps);
		effect* re = buf.read_effect_id(maps);
		auto r = Damage(step, re, rs, rp, rc, pid, amt, is);
		r.is_reflected = ir;
		return r;
	}
	case 56: { // Recover
		uint8_t rp = buf.read_u8();
		uint8_t pid = buf.read_u8();
		bool is = buf.read_bool();
		uint32_t amt = buf.read_u32();
		uint32_t rs = buf.read_u32();
		effect* re = buf.read_effect_id(maps);
		return Recover(step, re, rs, rp, pid, amt, is);
	}
	case 57: { // Equip
		uint8_t ep = buf.read_u8();
		bool is = buf.read_bool();
		bool fu = buf.read_bool();
		card* ec = buf.read_card_id(maps);
		card* tgt = buf.read_card_id(maps);
		return Equip(step, ep, ec, tgt, fu, is);
	}
	case 58: { // GetControl
		uint8_t cp = buf.read_u8();
		uint8_t pid = buf.read_u8();
		uint8_t rc = buf.read_u8();
		uint16_t rp = buf.read_u16();
		uint32_t zn = buf.read_u32();
		effect* re = buf.read_effect_id(maps);
		auto tg = read_owned_group();
		auto r = GetControl(step, re, cp, std::move(tg), pid, rp, rc, zn);
		buf.read_card_set(r.destroy_set, maps);
		return r;
	}
	case 59: { // SwapControl
		uint8_t rc = buf.read_u8();
		uint8_t rp = buf.read_u8();
		uint8_t ss = buf.read_u8();
		uint16_t rph = buf.read_u16();
		effect* re = buf.read_effect_id(maps);
		auto t1 = read_owned_group();
		auto t2 = read_owned_group();
		auto r = SwapControl(step, re, rp, std::move(t1), std::move(t2), rph, rc);
		r.self_selected_sequence = ss;
		return r;
	}
	case 60: { // ControlAdjust
		auto r = ControlAdjust(step);
		r.adjusting_player = buf.read_u8();
		buf.read_card_set(r.destroy_set, maps);
		buf.read_card_set(r.adjust_set, maps);
		return r;
	}
	case 61: { // SelfDestroyUnique
		uint8_t pid = buf.read_u8();
		card* uc = buf.read_card_id(maps);
		return SelfDestroyUnique(step, uc, pid);
	}
	case 62: // SelfDestroy
		return SelfDestroy(step);
	case 63: // SelfToGrave
		return SelfToGrave(step);
	case 64: { // TrapMonsterAdjust
		auto r = TrapMonsterAdjust(step);
		r.oppo_selection = buf.read_bool();
		buf.read_card_set(r.to_grave_set, maps);
		return r;
	}
	case 65: { // PayLPCost
		uint8_t pid = buf.read_u8();
		uint32_t cost = buf.read_u32();
		return PayLPCost(step, pid, cost);
	}
	case 66: { // RemoveCounter
		uint8_t rp = buf.read_u8();
		uint8_t s = buf.read_u8();
		uint8_t o = buf.read_u8();
		uint16_t ct = buf.read_u16();
		uint16_t cnt = buf.read_u16();
		uint32_t rs = buf.read_u32();
		card* pc = buf.read_card_id(maps);
		return RemoveCounter(step, rs, pc, rp, s, o, ct, cnt);
	}
	case 67: // AttackDisable
		return AttackDisable(step);
	case 68: { // AnnounceRace
		uint8_t pid = buf.read_u8();
		uint8_t cnt = buf.read_u8();
		uint64_t av = buf.read_u64();
		return AnnounceRace(step, pid, cnt, av);
	}
	case 69: { // AnnounceAttribute
		uint8_t pid = buf.read_u8();
		uint8_t cnt = buf.read_u8();
		uint32_t av = buf.read_u32();
		return AnnounceAttribute(step, pid, cnt, av);
	}
	case 70: // AnnounceCard
		return AnnounceCard(step, buf.read_u8());
	case 71: // AnnounceNumber
		return AnnounceNumber(step, buf.read_u8());
	case 72: { // TossCoin
		uint8_t pid = buf.read_u8();
		uint8_t rp = buf.read_u8();
		uint8_t cnt = buf.read_u8();
		effect* re = buf.read_effect_id(maps);
		return TossCoin(step, re, rp, pid, cnt);
	}
	case 73: { // TossDice
		uint8_t pid = buf.read_u8();
		uint8_t rp = buf.read_u8();
		uint8_t c1 = buf.read_u8();
		uint8_t c2 = buf.read_u8();
		effect* re = buf.read_effect_id(maps);
		return TossDice(step, re, rp, pid, c1, c2);
	}
	case 74: { // RockPaperScissors
		bool rp = buf.read_bool();
		uint8_t h0 = buf.read_u8();
		auto r = RockPaperScissors(step, rp);
		r.hand0 = h0;
		return r;
	}
	case 75: { // SelectFusion
		uint8_t pid = buf.read_u8();
		uint32_t chkf = buf.read_u32();
		auto fm = read_owned_group();
		auto ffm = read_owned_group();
		card* pc = buf.read_card_id(maps);
		return SelectFusion(step, pid, std::move(fm), chkf, std::move(ffm), pc);
	}
	case 76: { // DiscardHand
		uint8_t pid = buf.read_u8();
		uint8_t mn = buf.read_u8();
		uint8_t mx = buf.read_u8();
		uint32_t rs = buf.read_u32();
		return DiscardHand(step, pid, mn, mx, rs);
	}
	case 77: { // DiscardDeck
		uint16_t cnt = buf.read_u16();
		uint8_t pid = buf.read_u8();
		uint32_t rs = buf.read_u32();
		return DiscardDeck(step, pid, cnt, rs);
	}
	case 78: { // SortDeck
		uint16_t cnt = buf.read_u16();
		uint8_t sp = buf.read_u8();
		uint8_t tp = buf.read_u8();
		bool bt = buf.read_bool();
		return SortDeck(step, sp, tp, cnt, bt);
	}
	case 79: { // RemoveOverlay
		uint16_t mn = buf.read_u16();
		uint16_t mx = buf.read_u16();
		uint16_t ra = buf.read_u16();
		bool hu = buf.read_bool();
		uint8_t rp = buf.read_u8();
		uint8_t s = buf.read_u8();
		uint8_t o = buf.read_u8();
		uint32_t rs = buf.read_u32();
		auto pg = read_owned_group();
		auto r = RemoveOverlay(step, rs, std::move(pg), rp, s, o, mn, mx);
		r.replaced_amount = ra;
		r.has_used_overlay_remove_replace_effect = hu;
		return r;
	}
	case 80: { // XyzOverlay
		bool sg = buf.read_bool();
		card* tgt = buf.read_card_id(maps);
		auto mats = read_owned_group();
		return XyzOverlay(step, tgt, std::move(mats), sg);
	}
	case 81: // RefreshRelay
		return RefreshRelay(step);
	default:
		assert(false && "Unknown processor_unit variant index");
		return Adjust(0); // unreachable
	}
}

// ============================================================
// processor
// ============================================================
static void serialize_event_list(SerializeBuffer& buf, const event_list& el, const IdMaps& maps) {
	buf.write_u32(static_cast<uint32_t>(el.size()));
	for(auto& ev : el)
		serialize_tevent(buf, ev, maps);
}
static void deserialize_event_list(SerializeBuffer& buf, event_list& el, const IdMaps& maps, duel* pduel) {
	uint32_t n = buf.read_u32();
	el.clear();
	for(uint32_t i = 0; i < n; i++) {
		tevent ev{};
		deserialize_tevent(buf, ev, maps, pduel);
		el.push_back(std::move(ev));
	}
}
static void serialize_chain_list(SerializeBuffer& buf, const chain_list& cl, const IdMaps& maps) {
	buf.write_u32(static_cast<uint32_t>(cl.size()));
	for(auto& ch : cl)
		serialize_chain(buf, ch, maps);
}
static void deserialize_chain_list(SerializeBuffer& buf, chain_list& cl, const IdMaps& maps, duel* pduel) {
	uint32_t n = buf.read_u32();
	cl.clear();
	for(uint32_t i = 0; i < n; i++) {
		chain ch{};
		deserialize_chain(buf, ch, maps, pduel);
		cl.push_back(std::move(ch));
	}
}
static void serialize_processor_list(SerializeBuffer& buf, const std::list<processor_unit>& pl, const IdMaps& maps) {
	buf.write_u32(static_cast<uint32_t>(pl.size()));
	for(auto& u : pl)
		serialize_processor_unit(buf, u, maps);
}
static void deserialize_processor_list(SerializeBuffer& buf, std::list<processor_unit>& pl, const IdMaps& maps, duel* pduel) {
	uint32_t n = buf.read_u32();
	pl.clear();
	for(uint32_t i = 0; i < n; i++)
		pl.push_back(deserialize_processor_unit(buf, maps, pduel));
}

static void serialize_delayed_effect_collection(SerializeBuffer& buf, const processor::delayed_effect_collection& dec, const IdMaps& maps) {
	buf.write_u32(static_cast<uint32_t>(dec.size()));
	for(auto& [eff, ev] : dec) {
		buf.write_effect_id(eff, maps);
		serialize_tevent(buf, ev, maps);
	}
}
static void deserialize_delayed_effect_collection(SerializeBuffer& buf, processor::delayed_effect_collection& dec, const IdMaps& maps, duel* pduel) {
	uint32_t n = buf.read_u32();
	dec.clear();
	for(uint32_t i = 0; i < n; i++) {
		effect* eff = buf.read_effect_id(maps);
		tevent ev{};
		deserialize_tevent(buf, ev, maps, pduel);
		dec.emplace(eff, std::move(ev));
	}
}

static void serialize_instant_f_list(SerializeBuffer& buf, const instant_f_list& ifl, const IdMaps& maps) {
	buf.write_u32(static_cast<uint32_t>(ifl.size()));
	for(auto& [eff, ch] : ifl) {
		buf.write_effect_id(eff, maps);
		serialize_chain(buf, ch, maps);
	}
}
static void deserialize_instant_f_list(SerializeBuffer& buf, instant_f_list& ifl, const IdMaps& maps, duel* pduel) {
	uint32_t n = buf.read_u32();
	ifl.clear();
	for(uint32_t i = 0; i < n; i++) {
		effect* eff = buf.read_effect_id(maps);
		chain ch{};
		deserialize_chain(buf, ch, maps, pduel);
		ifl.emplace(eff, std::move(ch));
	}
}

static void serialize_chain_limit_list(SerializeBuffer& buf, const processor::chain_limit_list& cll) {
	buf.write_u32(static_cast<uint32_t>(cll.size()));
	for(auto& cl : cll) {
		buf.write_i32(cl.function);
		buf.write_i32(cl.player);
	}
}
static void deserialize_chain_limit_list(SerializeBuffer& buf, processor::chain_limit_list& cll) {
	uint32_t n = buf.read_u32();
	cll.clear();
	for(uint32_t i = 0; i < n; i++) {
		int32_t f = buf.read_i32();
		int32_t p = buf.read_i32();
		cll.emplace_back(f, p);
	}
}

static void serialize_action_counter(SerializeBuffer& buf, const processor::action_counter_t& ac, const IdMaps& maps) {
	buf.write_u32(static_cast<uint32_t>(ac.size()));
	for(auto& [k, v] : ac) {
		buf.write_u32(k);
		buf.write_i32(v.check_function);
		buf.write_u16(v.player_amount[0]);
		buf.write_u16(v.player_amount[1]);
	}
}
static void deserialize_action_counter(SerializeBuffer& buf, processor::action_counter_t& ac) {
	uint32_t n = buf.read_u32();
	ac.clear();
	for(uint32_t i = 0; i < n; i++) {
		uint32_t k = buf.read_u32();
		processor::action_value_t v;
		v.check_function = buf.read_i32();
		v.player_amount[0] = buf.read_u16();
		v.player_amount[1] = buf.read_u16();
		ac[k] = v;
	}
}

static void serialize_effect_count_map(SerializeBuffer& buf, const processor::effect_count_map& ecm) {
	buf.write_u32(static_cast<uint32_t>(ecm.size()));
	for(auto& [k, v] : ecm) {
		buf.write_u64(k);
		buf.write_u32(v);
	}
}
static void deserialize_effect_count_map(SerializeBuffer& buf, processor::effect_count_map& ecm) {
	uint32_t n = buf.read_u32();
	ecm.clear();
	for(uint32_t i = 0; i < n; i++) {
		uint64_t k = buf.read_u64();
		uint32_t v = buf.read_u32();
		ecm[k] = v;
	}
}

void serialize_processor(SerializeBuffer& buf, const processor& proc, const IdMaps& maps) {
	serialize_processor_list(buf, proc.units, maps);
	serialize_processor_list(buf, proc.subunits, maps);
	// reserved: optional<processor_unit>
	buf.write_bool(proc.reserved.has_value());
	if(proc.reserved)
		serialize_processor_unit(buf, *proc.reserved, maps);

	buf.write_card_set(proc.just_sent_cards, maps);
	buf.write_card_vector(proc.select_cards, maps);
	// select_cards_codes: vector<pair<uint32_t, uint32_t>>
	buf.write_u32(static_cast<uint32_t>(proc.select_cards_codes.size()));
	for(auto& [a, b] : proc.select_cards_codes) {
		buf.write_u32(a);
		buf.write_u32(b);
	}
	buf.write_card_vector(proc.unselect_cards, maps);
	buf.write_card_vector(proc.summonable_cards, maps);
	buf.write_card_vector(proc.spsummonable_cards, maps);
	buf.write_card_vector(proc.repositionable_cards, maps);
	buf.write_card_vector(proc.msetable_cards, maps);
	buf.write_card_vector(proc.ssetable_cards, maps);
	buf.write_card_vector(proc.attackable_cards, maps);
	buf.write_effect_set(proc.select_effects, maps);
	// select_options: vector<uint64_t>
	buf.write_u32(static_cast<uint32_t>(proc.select_options.size()));
	for(auto v : proc.select_options)
		buf.write_u64(v);
	buf.write_card_vector(proc.must_select_cards, maps);

	// Event lists
	serialize_event_list(buf, proc.point_event, maps);
	serialize_event_list(buf, proc.instant_event, maps);
	serialize_event_list(buf, proc.queue_event, maps);
	serialize_event_list(buf, proc.delayed_activate_event, maps);
	serialize_event_list(buf, proc.full_event, maps);
	serialize_event_list(buf, proc.used_event, maps);
	serialize_event_list(buf, proc.single_event, maps);
	serialize_event_list(buf, proc.solving_event, maps);
	serialize_event_list(buf, proc.sub_solving_event, maps);

	// Chain lists
	serialize_chain_list(buf, proc.select_chains, maps);
	// current_chain: chain_array (vector<chain>)
	buf.write_u32(static_cast<uint32_t>(proc.current_chain.size()));
	for(auto& ch : proc.current_chain)
		serialize_chain(buf, ch, maps);
	buf.write_i32(proc.real_chain_count);
	serialize_chain_list(buf, proc.tpchain, maps);
	serialize_chain_list(buf, proc.ntpchain, maps);
	serialize_chain_list(buf, proc.ignition_priority_chains, maps);
	serialize_chain_list(buf, proc.continuous_chain, maps);
	serialize_chain_list(buf, proc.solving_continuous, maps);
	serialize_chain_list(buf, proc.sub_solving_continuous, maps);
	serialize_chain_list(buf, proc.delayed_continuous_tp, maps);
	serialize_chain_list(buf, proc.delayed_continuous_ntp, maps);
	serialize_chain_list(buf, proc.desrep_chain, maps);
	serialize_chain_list(buf, proc.new_fchain, maps);
	serialize_chain_list(buf, proc.new_fchain_s, maps);
	serialize_chain_list(buf, proc.new_ochain, maps);
	serialize_chain_list(buf, proc.new_ochain_s, maps);
	serialize_chain_list(buf, proc.new_fchain_b, maps);
	serialize_chain_list(buf, proc.new_ochain_b, maps);
	serialize_chain_list(buf, proc.new_ochain_h, maps);
	serialize_chain_list(buf, proc.new_chains, maps);

	serialize_delayed_effect_collection(buf, proc.delayed_quick_tmp, maps);
	serialize_delayed_effect_collection(buf, proc.delayed_quick, maps);
	serialize_instant_f_list(buf, proc.quick_f_chain, maps);

	// Card sets
	buf.write_card_set(proc.leave_confirmed, maps);
	buf.write_card_set(proc.special_summoning, maps);
	buf.write_card_set(proc.ss_tograve_set, maps);
	buf.write_card_set(proc.equiping_cards, maps);
	buf.write_card_set(proc.control_adjust_set[0], maps);
	buf.write_card_set(proc.control_adjust_set[1], maps);
	buf.write_card_set(proc.unique_destroy_set, maps);
	buf.write_card_set(proc.self_destroy_set, maps);
	buf.write_card_set(proc.self_tograve_set, maps);
	buf.write_card_set(proc.trap_monster_adjust_set[0], maps);
	buf.write_card_set(proc.trap_monster_adjust_set[1], maps);
	buf.write_card_set(proc.release_cards, maps);
	buf.write_card_set(proc.release_cards_ex, maps);
	buf.write_card_set(proc.release_cards_ex_oneof, maps);
	buf.write_card_set(proc.battle_destroy_rep, maps);
	buf.write_card_set(proc.fusion_materials, maps);
	buf.write_card_set(proc.operated_set, maps);
	buf.write_card_set(proc.discarded_set, maps);
	buf.write_card_set(proc.destroy_canceled, maps);
	buf.write_card_set(proc.delayed_enable_set, maps);
	buf.write_card_set(proc.set_group_pre_set, maps);
	buf.write_card_set(proc.set_group_set, maps);

	// Effect sets
	buf.write_effect_set(proc.disfield_effects, maps);
	buf.write_effect_set(proc.extra_mzone_effects, maps);
	buf.write_effect_set(proc.extra_szone_effects, maps);

	// reseted_effects: set<effect*>
	buf.write_u32(static_cast<uint32_t>(proc.reseted_effects.size()));
	for(auto* e : proc.reseted_effects)
		buf.write_effect_id(e, maps);

	// readjust_map: unordered_map<card*, uint32_t>
	buf.write_u32(static_cast<uint32_t>(proc.readjust_map.size()));
	for(auto& [c, v] : proc.readjust_map) {
		buf.write_card_id(c, maps);
		buf.write_u32(v);
	}

	// unique_cards[2]: unordered_set<card*>
	for(int p = 0; p < 2; p++) {
		buf.write_u32(static_cast<uint32_t>(proc.unique_cards[p].size()));
		for(auto* c : proc.unique_cards[p])
			buf.write_card_id(c, maps);
	}

	// effect_count maps
	serialize_effect_count_map(buf, proc.effect_count_code);
	serialize_effect_count_map(buf, proc.effect_count_code_duel);
	serialize_effect_count_map(buf, proc.effect_count_code_chain);

	// spsummon_once_map[2]: unordered_map<uint32_t, uint32_t>
	for(int p = 0; p < 2; p++) {
		buf.write_u32(static_cast<uint32_t>(proc.spsummon_once_map[p].size()));
		for(auto& [k, v] : proc.spsummon_once_map[p]) {
			buf.write_u32(k);
			buf.write_u32(v);
		}
	}
	for(int p = 0; p < 2; p++) {
		buf.write_u32(static_cast<uint32_t>(proc.spsummon_once_map_rst[p].size()));
		for(auto& [k, v] : proc.spsummon_once_map_rst[p]) {
			buf.write_u32(k);
			buf.write_u32(v);
		}
	}

	// xmaterial_lst: multimap<int32_t, card*, greater<int32_t>>
	buf.write_u32(static_cast<uint32_t>(proc.xmaterial_lst.size()));
	for(auto& [k, c] : proc.xmaterial_lst) {
		buf.write_i32(k);
		buf.write_card_id(c, maps);
	}

	// Scalar fields
	buf.write_u32(proc.global_flag);
	buf.write_u32(proc.pre_field[0]);
	buf.write_u32(proc.pre_field[1]);
	// opp_mzone: set<uint32_t>
	buf.write_u32(static_cast<uint32_t>(proc.opp_mzone.size()));
	for(auto v : proc.opp_mzone)
		buf.write_u32(v);

	serialize_chain_limit_list(buf, proc.chain_limit);
	serialize_chain_limit_list(buf, proc.chain_limit_p);

	buf.write_bool(proc.chain_solving);
	buf.write_bool(proc.conti_solving);
	buf.write_u8(proc.win_player);
	buf.write_u8(proc.win_reason);
	buf.write_bool(proc.re_adjust);
	buf.write_effect_id(proc.reason_effect, maps);
	buf.write_u8(proc.reason_player);
	buf.write_card_id(proc.summoning_card, maps);
	buf.write_u32(proc.summoning_proc_group_type);
	buf.write_u8(proc.summon_depth);
	buf.write_u8(proc.summon_cancelable);
	buf.write_card_id(proc.attacker, maps);
	buf.write_card_id(proc.attack_target, maps);
	buf.write_bool(proc.set_forced_attack);
	buf.write_card_id(proc.forced_attacker, maps);
	buf.write_card_id(proc.forced_attack_target, maps);
	serialize_owned_group(buf, proc.must_use_mats, maps);
	serialize_owned_group(buf, proc.only_use_mats, maps);
	buf.write_i32(proc.forced_summon_minc);
	buf.write_i32(proc.forced_summon_maxc);
	buf.write_bool(proc.attack_cancelable);
	buf.write_u8(proc.attack_cost_paid);
	buf.write_bool(proc.attack_rollback);
	buf.write_u8(proc.effect_damage_step);
	buf.write_i32(proc.battle_damage[0]);
	buf.write_i32(proc.battle_damage[1]);
	buf.write_i32(proc.summon_count[0]);
	buf.write_i32(proc.summon_count[1]);
	buf.write_u8(proc.extra_summon[0]);
	buf.write_u8(proc.extra_summon[1]);
	buf.write_i32(proc.spe_effect[0]);
	buf.write_i32(proc.spe_effect[1]);
	buf.write_u64(proc.duel_options);
	buf.write_u32(proc.copy_reset);
	buf.write_u8(proc.copy_reset_count);
	buf.write_u32(proc.last_control_changed_id);
	buf.write_u32(proc.set_group_used_zones);
	for(int i = 0; i < 7; i++)
		buf.write_u8(proc.set_group_seq[i]);
	// dice_results: vector<uint8_t>
	buf.write_u32(static_cast<uint32_t>(proc.dice_results.size()));
	for(auto v : proc.dice_results)
		buf.write_u8(v);
	// coin_results: vector<bool>
	buf.write_u32(static_cast<uint32_t>(proc.coin_results.size()));
	for(bool v : proc.coin_results)
		buf.write_bool(v);

	buf.write_bool(proc.to_bp);
	buf.write_bool(proc.to_m2);
	buf.write_bool(proc.to_ep);
	buf.write_bool(proc.skip_m2);
	buf.write_bool(proc.chain_attack);
	buf.write_u32(proc.chain_attacker_id);
	buf.write_card_id(proc.chain_attack_target, maps);
	buf.write_u8(proc.attack_player);
	buf.write_bool(proc.selfdes_disabled);
	buf.write_bool(proc.overdraw[0]);
	buf.write_bool(proc.overdraw[1]);
	buf.write_i32(proc.check_level);
	buf.write_bool(proc.shuffle_check_disabled);
	buf.write_bool(proc.shuffle_hand_check[0]);
	buf.write_bool(proc.shuffle_hand_check[1]);
	buf.write_bool(proc.shuffle_deck_check[0]);
	buf.write_bool(proc.shuffle_deck_check[1]);
	buf.write_bool(proc.deck_reversed);
	buf.write_bool(proc.remove_brainwashing);
	buf.write_bool(proc.flip_delayed);
	buf.write_bool(proc.damage_calculated);
	buf.write_bool(proc.hand_adjusted);
	for(int i = 0; i < 2; i++) {
		buf.write_u8(proc.summon_state_count[i]);
		buf.write_u8(proc.normalsummon_state_count[i]);
		buf.write_u8(proc.flipsummon_state_count[i]);
		buf.write_u8(proc.spsummon_state_count[i]);
		buf.write_u8(proc.spsummon_state_count_rst[i]);
		buf.write_u8(proc.spsummon_state_count_tmp[i]);
		buf.write_u8(proc.attack_state_count[i]);
		buf.write_u8(proc.battle_phase_count[i]);
		buf.write_u8(proc.battled_count[i]);
	}
	buf.write_bool(proc.spsummon_rst);
	buf.write_bool(proc.phase_action);
	buf.write_u32(proc.hint_timing[0]);
	buf.write_u32(proc.hint_timing[1]);
	buf.write_u8(proc.current_player);
	buf.write_u8(proc.conti_player);
	buf.write_bool(proc.force_turn_end);

	serialize_action_counter(buf, proc.summon_counter, maps);
	serialize_action_counter(buf, proc.normalsummon_counter, maps);
	serialize_action_counter(buf, proc.spsummon_counter, maps);
	serialize_action_counter(buf, proc.flipsummon_counter, maps);
	serialize_action_counter(buf, proc.attack_counter, maps);
	serialize_action_counter(buf, proc.chain_counter, maps);

	serialize_processor_list(buf, proc.recover_damage_reserve, maps);
	buf.write_effect_set(proc.dec_count_reserve, maps);
}

void deserialize_processor(SerializeBuffer& buf, processor& proc, const IdMaps& maps, duel* pduel) {
	deserialize_processor_list(buf, proc.units, maps, pduel);
	deserialize_processor_list(buf, proc.subunits, maps, pduel);
	if(buf.read_bool()) {
		proc.reserved = deserialize_processor_unit(buf, maps, pduel);
	} else {
		proc.reserved.reset();
	}

	buf.read_card_set(proc.just_sent_cards, maps);
	buf.read_card_vector(proc.select_cards, maps);
	uint32_t n = buf.read_u32();
	proc.select_cards_codes.resize(n);
	for(uint32_t i = 0; i < n; i++) {
		proc.select_cards_codes[i].first = buf.read_u32();
		proc.select_cards_codes[i].second = buf.read_u32();
	}
	buf.read_card_vector(proc.unselect_cards, maps);
	buf.read_card_vector(proc.summonable_cards, maps);
	buf.read_card_vector(proc.spsummonable_cards, maps);
	buf.read_card_vector(proc.repositionable_cards, maps);
	buf.read_card_vector(proc.msetable_cards, maps);
	buf.read_card_vector(proc.ssetable_cards, maps);
	buf.read_card_vector(proc.attackable_cards, maps);
	buf.read_effect_set(proc.select_effects, maps);
	n = buf.read_u32();
	proc.select_options.resize(n);
	for(uint32_t i = 0; i < n; i++)
		proc.select_options[i] = buf.read_u64();
	buf.read_card_vector(proc.must_select_cards, maps);

	deserialize_event_list(buf, proc.point_event, maps, pduel);
	deserialize_event_list(buf, proc.instant_event, maps, pduel);
	deserialize_event_list(buf, proc.queue_event, maps, pduel);
	deserialize_event_list(buf, proc.delayed_activate_event, maps, pduel);
	deserialize_event_list(buf, proc.full_event, maps, pduel);
	deserialize_event_list(buf, proc.used_event, maps, pduel);
	deserialize_event_list(buf, proc.single_event, maps, pduel);
	deserialize_event_list(buf, proc.solving_event, maps, pduel);
	deserialize_event_list(buf, proc.sub_solving_event, maps, pduel);

	deserialize_chain_list(buf, proc.select_chains, maps, pduel);
	n = buf.read_u32();
	proc.current_chain.clear();
	proc.current_chain.reserve(n);
	for(uint32_t i = 0; i < n; i++) {
		chain ch{};
		deserialize_chain(buf, ch, maps, pduel);
		proc.current_chain.push_back(std::move(ch));
	}
	proc.real_chain_count = buf.read_i32();
	deserialize_chain_list(buf, proc.tpchain, maps, pduel);
	deserialize_chain_list(buf, proc.ntpchain, maps, pduel);
	deserialize_chain_list(buf, proc.ignition_priority_chains, maps, pduel);
	deserialize_chain_list(buf, proc.continuous_chain, maps, pduel);
	deserialize_chain_list(buf, proc.solving_continuous, maps, pduel);
	deserialize_chain_list(buf, proc.sub_solving_continuous, maps, pduel);
	deserialize_chain_list(buf, proc.delayed_continuous_tp, maps, pduel);
	deserialize_chain_list(buf, proc.delayed_continuous_ntp, maps, pduel);
	deserialize_chain_list(buf, proc.desrep_chain, maps, pduel);
	deserialize_chain_list(buf, proc.new_fchain, maps, pduel);
	deserialize_chain_list(buf, proc.new_fchain_s, maps, pduel);
	deserialize_chain_list(buf, proc.new_ochain, maps, pduel);
	deserialize_chain_list(buf, proc.new_ochain_s, maps, pduel);
	deserialize_chain_list(buf, proc.new_fchain_b, maps, pduel);
	deserialize_chain_list(buf, proc.new_ochain_b, maps, pduel);
	deserialize_chain_list(buf, proc.new_ochain_h, maps, pduel);
	deserialize_chain_list(buf, proc.new_chains, maps, pduel);

	deserialize_delayed_effect_collection(buf, proc.delayed_quick_tmp, maps, pduel);
	deserialize_delayed_effect_collection(buf, proc.delayed_quick, maps, pduel);
	deserialize_instant_f_list(buf, proc.quick_f_chain, maps, pduel);

	buf.read_card_set(proc.leave_confirmed, maps);
	buf.read_card_set(proc.special_summoning, maps);
	buf.read_card_set(proc.ss_tograve_set, maps);
	buf.read_card_set(proc.equiping_cards, maps);
	buf.read_card_set(proc.control_adjust_set[0], maps);
	buf.read_card_set(proc.control_adjust_set[1], maps);
	buf.read_card_set(proc.unique_destroy_set, maps);
	buf.read_card_set(proc.self_destroy_set, maps);
	buf.read_card_set(proc.self_tograve_set, maps);
	buf.read_card_set(proc.trap_monster_adjust_set[0], maps);
	buf.read_card_set(proc.trap_monster_adjust_set[1], maps);
	buf.read_card_set(proc.release_cards, maps);
	buf.read_card_set(proc.release_cards_ex, maps);
	buf.read_card_set(proc.release_cards_ex_oneof, maps);
	buf.read_card_set(proc.battle_destroy_rep, maps);
	buf.read_card_set(proc.fusion_materials, maps);
	buf.read_card_set(proc.operated_set, maps);
	buf.read_card_set(proc.discarded_set, maps);
	buf.read_card_set(proc.destroy_canceled, maps);
	buf.read_card_set(proc.delayed_enable_set, maps);
	buf.read_card_set(proc.set_group_pre_set, maps);
	buf.read_card_set(proc.set_group_set, maps);

	buf.read_effect_set(proc.disfield_effects, maps);
	buf.read_effect_set(proc.extra_mzone_effects, maps);
	buf.read_effect_set(proc.extra_szone_effects, maps);

	n = buf.read_u32();
	proc.reseted_effects.clear();
	for(uint32_t i = 0; i < n; i++)
		proc.reseted_effects.insert(buf.read_effect_id(maps));

	n = buf.read_u32();
	proc.readjust_map.clear();
	for(uint32_t i = 0; i < n; i++) {
		card* c = buf.read_card_id(maps);
		uint32_t v = buf.read_u32();
		proc.readjust_map[c] = v;
	}

	for(int p = 0; p < 2; p++) {
		n = buf.read_u32();
		proc.unique_cards[p].clear();
		for(uint32_t i = 0; i < n; i++)
			proc.unique_cards[p].insert(buf.read_card_id(maps));
	}

	deserialize_effect_count_map(buf, proc.effect_count_code);
	deserialize_effect_count_map(buf, proc.effect_count_code_duel);
	deserialize_effect_count_map(buf, proc.effect_count_code_chain);

	for(int p = 0; p < 2; p++) {
		n = buf.read_u32();
		proc.spsummon_once_map[p].clear();
		for(uint32_t i = 0; i < n; i++) {
			uint32_t k = buf.read_u32();
			uint32_t v = buf.read_u32();
			proc.spsummon_once_map[p][k] = v;
		}
	}
	for(int p = 0; p < 2; p++) {
		n = buf.read_u32();
		proc.spsummon_once_map_rst[p].clear();
		for(uint32_t i = 0; i < n; i++) {
			uint32_t k = buf.read_u32();
			uint32_t v = buf.read_u32();
			proc.spsummon_once_map_rst[p][k] = v;
		}
	}

	n = buf.read_u32();
	proc.xmaterial_lst.clear();
	for(uint32_t i = 0; i < n; i++) {
		int32_t k = buf.read_i32();
		card* c = buf.read_card_id(maps);
		proc.xmaterial_lst.emplace(k, c);
	}

	proc.global_flag = buf.read_u32();
	proc.pre_field[0] = buf.read_u32();
	proc.pre_field[1] = buf.read_u32();
	n = buf.read_u32();
	proc.opp_mzone.clear();
	for(uint32_t i = 0; i < n; i++)
		proc.opp_mzone.insert(buf.read_u32());

	deserialize_chain_limit_list(buf, proc.chain_limit);
	deserialize_chain_limit_list(buf, proc.chain_limit_p);

	proc.chain_solving = buf.read_bool();
	proc.conti_solving = buf.read_bool();
	proc.win_player = buf.read_u8();
	proc.win_reason = buf.read_u8();
	proc.re_adjust = buf.read_bool();
	proc.reason_effect = buf.read_effect_id(maps);
	proc.reason_player = buf.read_u8();
	proc.summoning_card = buf.read_card_id(maps);
	proc.summoning_proc_group_type = buf.read_u32();
	proc.summon_depth = buf.read_u8();
	proc.summon_cancelable = buf.read_u8();
	proc.attacker = buf.read_card_id(maps);
	proc.attack_target = buf.read_card_id(maps);
	proc.set_forced_attack = buf.read_bool();
	proc.forced_attacker = buf.read_card_id(maps);
	proc.forced_attack_target = buf.read_card_id(maps);
	proc.must_use_mats = deserialize_owned_group(buf, maps, pduel);
	proc.only_use_mats = deserialize_owned_group(buf, maps, pduel);
	proc.forced_summon_minc = buf.read_i32();
	proc.forced_summon_maxc = buf.read_i32();
	proc.attack_cancelable = buf.read_bool();
	proc.attack_cost_paid = buf.read_u8();
	proc.attack_rollback = buf.read_bool();
	proc.effect_damage_step = buf.read_u8();
	proc.battle_damage[0] = buf.read_i32();
	proc.battle_damage[1] = buf.read_i32();
	proc.summon_count[0] = buf.read_i32();
	proc.summon_count[1] = buf.read_i32();
	proc.extra_summon[0] = buf.read_u8();
	proc.extra_summon[1] = buf.read_u8();
	proc.spe_effect[0] = buf.read_i32();
	proc.spe_effect[1] = buf.read_i32();
	proc.duel_options = buf.read_u64();
	proc.copy_reset = buf.read_u32();
	proc.copy_reset_count = buf.read_u8();
	proc.last_control_changed_id = buf.read_u32();
	proc.set_group_used_zones = buf.read_u32();
	for(int i = 0; i < 7; i++)
		proc.set_group_seq[i] = buf.read_u8();
	n = buf.read_u32();
	proc.dice_results.resize(n);
	for(uint32_t i = 0; i < n; i++)
		proc.dice_results[i] = buf.read_u8();
	n = buf.read_u32();
	proc.coin_results.resize(n);
	for(uint32_t i = 0; i < n; i++)
		proc.coin_results[i] = buf.read_bool();

	proc.to_bp = buf.read_bool();
	proc.to_m2 = buf.read_bool();
	proc.to_ep = buf.read_bool();
	proc.skip_m2 = buf.read_bool();
	proc.chain_attack = buf.read_bool();
	proc.chain_attacker_id = buf.read_u32();
	proc.chain_attack_target = buf.read_card_id(maps);
	proc.attack_player = buf.read_u8();
	proc.selfdes_disabled = buf.read_bool();
	proc.overdraw[0] = buf.read_bool();
	proc.overdraw[1] = buf.read_bool();
	proc.check_level = buf.read_i32();
	proc.shuffle_check_disabled = buf.read_bool();
	proc.shuffle_hand_check[0] = buf.read_bool();
	proc.shuffle_hand_check[1] = buf.read_bool();
	proc.shuffle_deck_check[0] = buf.read_bool();
	proc.shuffle_deck_check[1] = buf.read_bool();
	proc.deck_reversed = buf.read_bool();
	proc.remove_brainwashing = buf.read_bool();
	proc.flip_delayed = buf.read_bool();
	proc.damage_calculated = buf.read_bool();
	proc.hand_adjusted = buf.read_bool();
	for(int i = 0; i < 2; i++) {
		proc.summon_state_count[i] = buf.read_u8();
		proc.normalsummon_state_count[i] = buf.read_u8();
		proc.flipsummon_state_count[i] = buf.read_u8();
		proc.spsummon_state_count[i] = buf.read_u8();
		proc.spsummon_state_count_rst[i] = buf.read_u8();
		proc.spsummon_state_count_tmp[i] = buf.read_u8();
		proc.attack_state_count[i] = buf.read_u8();
		proc.battle_phase_count[i] = buf.read_u8();
		proc.battled_count[i] = buf.read_u8();
	}
	proc.spsummon_rst = buf.read_bool();
	proc.phase_action = buf.read_bool();
	proc.hint_timing[0] = buf.read_u32();
	proc.hint_timing[1] = buf.read_u32();
	proc.current_player = buf.read_u8();
	proc.conti_player = buf.read_u8();
	proc.force_turn_end = buf.read_bool();

	deserialize_action_counter(buf, proc.summon_counter);
	deserialize_action_counter(buf, proc.normalsummon_counter);
	deserialize_action_counter(buf, proc.spsummon_counter);
	deserialize_action_counter(buf, proc.flipsummon_counter);
	deserialize_action_counter(buf, proc.attack_counter);
	deserialize_action_counter(buf, proc.chain_counter);

	deserialize_processor_list(buf, proc.recover_damage_reserve, maps, pduel);
	buf.read_effect_set(proc.dec_count_reserve, maps);
}

// ============================================================
// field (top-level)
// ============================================================
void serialize_field(SerializeBuffer& buf, const field* f, const IdMaps& maps) {
	for(int p = 0; p < 2; p++)
		serialize_player_info(buf, f->player[p], maps);
	// temp_card is always present -- serialize by card_id
	buf.write_card_id(f->temp_card, maps);
	serialize_field_info(buf, f->infos);
	serialize_field_effect(buf, f->effects, maps);
	serialize_processor(buf, f->core, maps);
	// returns: ProgressiveBuffer
	buf.write_bytes(f->returns.data);
	// return_cards
	buf.write_bool(f->return_cards.canceled);
	buf.write_card_vector(f->return_cards.list, maps);
	// return_card_codes
	buf.write_bool(f->return_card_codes.canceled);
	buf.write_u32(static_cast<uint32_t>(f->return_card_codes.list.size()));
	for(auto& [a, b] : f->return_card_codes.list) {
		buf.write_u32(a);
		buf.write_u32(b);
	}
	// nil_event
	serialize_tevent(buf, f->nil_event, maps);
}

void deserialize_field(SerializeBuffer& buf, field* f, const IdMaps& maps, duel* pduel) {
	for(int p = 0; p < 2; p++)
		deserialize_player_info(buf, f->player[p], maps);
	f->temp_card = buf.read_card_id(maps);
	deserialize_field_info(buf, f->infos);
	deserialize_field_effect(buf, f->effects, maps);
	deserialize_processor(buf, f->core, maps, pduel);
	f->returns.data = buf.read_bytes();
	f->return_cards.canceled = buf.read_bool();
	buf.read_card_vector(f->return_cards.list, maps);
	f->return_card_codes.canceled = buf.read_bool();
	uint32_t n = buf.read_u32();
	f->return_card_codes.list.resize(n);
	for(uint32_t i = 0; i < n; i++) {
		f->return_card_codes.list[i].first = buf.read_u32();
		f->return_card_codes.list[i].second = buf.read_u32();
	}
	deserialize_tevent(buf, f->nil_event, maps, pduel);
}
