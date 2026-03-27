/*
 * Copyright (c) 2010-2015, Argon Sun (Fluorohydride)
 * Copyright (c) 2017-2025, Edoardo Lolletti (edo9300) <edoardo762@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <array>
#include <cstring> //std::memcpy
#include "card.h"
#include "duel.h"
#include "effect.h"
#include "field.h"
#include "interpreter.h"

duel::duel(const OCG_DuelOptions& options, bool& valid_lua_lib) :
	random({ options.seed[0], options.seed[1], options.seed[2], options.seed[3] }),
	read_card_callback(options.cardReader), read_script_callback(options.scriptReader),
	handle_message_callback(options.logHandler), read_card_done_callback(options.cardReaderDone),
	read_card_payload(options.payload1), read_script_payload(options.payload2),
	handle_message_payload(options.payload3), read_card_done_payload(options.payload4)
{
	lua = new interpreter(this, options, valid_lua_lib);
	if(!valid_lua_lib)
		return;
	game_field = new field(this, options);
	game_field->temp_card = new_card(0);
}
duel::~duel() {
	for(auto& pcard : cards)
		delete pcard;
	for(auto& pgroup : groups) {
		pgroup->container.clear();
		pgroup->is_iterator_dirty = true;
	}
	for(auto& peffect : effects)
		delete peffect;
	delete game_field;
	delete lua;
	for(auto& pgroup : groups)
		delete pgroup;
	for(auto& pgroup : orphaned_groups)
		delete pgroup;
}
#if defined(__GNUC__) || defined(__clang_analyzer__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
void duel::clear() {
	static constexpr OCG_DuelOptions default_options{ {},0,{8000,5,1},{8000,5,1} };
	for(auto& pcard : cards)
		delete pcard;
	for(auto& peffect : effects) {
		lua->unregister_effect(peffect);
		delete peffect;
	}
	delete game_field;
	//force full garbage collection to clean the groups
	lua->collect(true);
	cards.clear();
	/*
		TODO: how to properly handle groups that are still around after the field was destroyed?
		If they're still here, it means they're still living as global variables somewhere, for now we
		deal with them by clearing their containers that could have been pointing to now deleted cards
	*/
	for(auto& pgroup : groups) {
		pgroup->container.clear();
		pgroup->is_iterator_dirty = true;
	}
	effects.clear();
	game_field = new field(this, default_options);
	game_field->temp_card = new_card(0);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
void duel::reset_for_reuse(const OCG_DuelOptions& opts) {
	assumes.clear();
	uncopy.clear();
	for(auto& pcard : cards)
		delete pcard;
	for(auto& peffect : effects) {
		lua->unregister_effect(peffect);
		delete peffect;
	}
	delete game_field;
	lua->collect(true);
	cards.clear();
	for(auto& pgroup : groups) {
		pgroup->container.clear();
		pgroup->is_iterator_dirty = true;
	}
	effects.clear();
	game_field = new field(this, opts);
	game_field->temp_card = new_card(0);
	buff.clear();
	query_buffer.clear();
	clear_pending_messages();
	random = RNG::Xoshiro256StarStar({opts.seed[0], opts.seed[1], opts.seed[2], opts.seed[3]});
}
card* duel::new_card(uint32_t code) {
	card* pcard = new card(this);
	cards.insert(pcard);
	if(code)
		pcard->data = read_card(code);
	pcard->data.code = code;
	lua->register_card(pcard);
	return pcard;
}
effect* duel::new_effect() {
	effect* peffect = new effect(this);
	effects.insert(peffect);
	lua->register_effect(peffect);
	return peffect;
}
void duel::delete_card(card* pcard) {
	cards.erase(pcard);
	delete pcard;
}
void duel::delete_group(group* pgroup) {
	if(groups.erase(pgroup))
		delete pgroup;
}
void duel::delete_effect(effect* peffect) {
	lua->unregister_effect(peffect);
	effects.erase(peffect);
	delete peffect;
}
void duel::generate_buffer() {
	for(auto& message : messages) {
		uint32_t size = static_cast<uint32_t>(message.data.size());
		if(size == 0)
			continue;
		write_buffer(&size, sizeof(size));
		write_buffer(message.data.data(), size);
	}
	messages.clear();
}
void duel::restore_assumes() {
	for(auto& pcard : assumes)
		pcard->assume.clear();
	assumes.clear();
}
void duel::write_buffer(const void* data, size_t size) {
	if(size == 0)
		return;
	const auto vec_size = buff.size();
	buff.resize(vec_size + size);
	std::memcpy(&buff[vec_size], data, size);
}
void duel::clear_buffer() {
	buff.clear();
}
void duel::set_response(const void* resp, size_t len) {
	game_field->returns.data.resize(len);
	if(len == 0)
		return;
	std::memcpy(game_field->returns.data.data(), resp, len);
}
// uniform integer distribution
int32_t duel::get_next_integer(int32_t l, int32_t h) {
	assert(l <= h);
	const uint64_t range = int64_t(h) - int64_t(l) + 1;
	const uint64_t lim = random.max() % range;
	uint64_t n;
	do {
		n = random();
	} while(n <= lim);
	return static_cast<int32_t>((n % range) + l);
}
duel::duel_message* duel::new_message(uint8_t message) {
	return &(messages.emplace_back(message));
}
const card_data& duel::read_card(uint32_t code) {
	if(auto search = data_cache.find(code); search != data_cache.end())
		return search->second;
	OCG_CardData data{};
	read_card_callback(read_card_payload, code, &data);
	auto ret = &(data_cache.emplace(code, data).first->second);
	read_card_done_callback(read_card_done_payload, &data);
	return *ret;
}
duel::duel_message::duel_message(uint8_t message) {
	write<uint8_t>(message);
}
void duel::duel_message::write(const void* buff, size_t size) {
	if(size == 0)
	   return;
	const auto vec_size = data.size();
	data.resize(vec_size + size);
	std::memcpy(&data[vec_size], buff, size);
}
void duel::duel_message::write(loc_info loc) {
	write<uint8_t>(loc.controler);
	write<uint8_t>(loc.location);
	write<uint32_t>(loc.sequence);
	write<uint32_t>(loc.position);
}

void duel::build_id_maps(IdMaps& maps) {
	maps.clear();
	// ID 0 is reserved for nullptr
	for(auto* c : cards)
		maps.card_to_id[c] = c->cardid;
	for(auto& [ptr, id] : maps.card_to_id)
		maps.id_to_card[id] = ptr;
	for(auto* e : effects) {
		maps.effect_to_id[e] = e->id;
		maps.id_to_effect[e->id] = e;
	}
	uint32_t gid = 1;
	for(auto* g : groups) {
		maps.group_to_id[g] = gid;
		maps.id_to_group[gid] = g;
		gid++;
	}
}

void duel::free_snapshot_refs() {
	for(int32_t ref : snapshot_lua_refs) {
		if(ref)
			luaL_unref(lua->lua_state, LUA_REGISTRYINDEX, ref);
	}
	snapshot_lua_refs.clear();
}

int duel::serialize(void** out_buffer, uint32_t* out_size) {
	// Free any previous snapshot refs
	free_snapshot_refs();

	IdMaps maps;
	build_id_maps(maps);
	SerializeBuffer buf;

	// Magic + version
	buf.write_u32(0x59474F53); // "YGOS"
	buf.write_u32(2);          // version 2: manifest + cloned refs

	// RNG state
	auto& rng_state = random.state();
	for(int i = 0; i < 4; i++)
		buf.write_u64(rng_state[i]);

	// Messages queue
	buf.write_u32(static_cast<uint32_t>(messages.size()));
	for(auto& msg : messages)
		buf.write_bytes(msg.data);

	// Buff and query_buffer
	buf.write_bytes(buff);
	buf.write_bytes(query_buffer);

	// === Manifests (ID lists for reconciliation) ===
	// Card manifest
	buf.write_u32(static_cast<uint32_t>(cards.size()));
	for(auto* c : cards)
		buf.write_u32(c->cardid);
	// Effect manifest
	buf.write_u32(static_cast<uint32_t>(effects.size()));
	for(auto* e : effects)
		buf.write_u32(e->id);
	// Group manifest
	buf.write_u32(static_cast<uint32_t>(groups.size()));
	for(auto* g : groups)
		buf.write_u32(maps.group_to_id[g]);

	// === Object data ===

	// Cards
	buf.write_u32(static_cast<uint32_t>(cards.size()));
	for(auto* c : cards)
		serialize_card(buf, c, maps);

	// Effects -- clone Lua refs so they survive if originals are freed
	buf.write_u32(static_cast<uint32_t>(effects.size()));
	for(auto* e : effects) {
		serialize_effect(buf, e, maps);
		// Clone the 5 Lua function refs and write them
		bool val_is_ref = e->is_flag(EFFECT_FLAG_FUNC_VALUE);
		int32_t refs[5] = {
			e->condition ? lua->clone_lua_ref(e->condition) : 0,
			e->cost      ? lua->clone_lua_ref(e->cost)      : 0,
			e->target    ? lua->clone_lua_ref(e->target)    : 0,
			(val_is_ref && e->value) ? lua->clone_lua_ref(e->value) : 0,
			e->operation ? lua->clone_lua_ref(e->operation) : 0,
		};
		buf.write_bool(val_is_ref);
		for(int i = 0; i < 5; i++) {
			buf.write_i32(refs[i]);
			if(refs[i])
				snapshot_lua_refs.push_back(refs[i]);
		}
	}

	// Uncopy effects (just IDs)
	buf.write_u32(static_cast<uint32_t>(uncopy.size()));
	for(auto* e : uncopy)
		buf.write_effect_id(e, maps);

	// Assumes (just card IDs)
	buf.write_u32(static_cast<uint32_t>(assumes.size()));
	for(auto* c : assumes)
		buf.write_card_id(c, maps);

	// Groups
	buf.write_u32(static_cast<uint32_t>(groups.size()));
	for(auto* g : groups) {
		buf.write_u32(maps.group_to_id[g]);
		buf.write_card_set(g->container, maps);
		buf.write_bool(g->is_readonly);
	}

	// Field
	serialize_field(buf, game_field, maps);

	*out_size = static_cast<uint32_t>(buf.size());
	*out_buffer = ::malloc(buf.size());
	if(!*out_buffer)
		return -1;
	std::memcpy(*out_buffer, buf.raw_data(), buf.size());
	return 0;
}

int duel::deserialize(const void* buffer, uint32_t size) {
	// Stop Lua GC during deserialization to prevent collection of
	// weakly-referenced groups while we're rebuilding state.
	lua_gc(lua->lua_state, LUA_GCSTOP, 0);

	SerializeBuffer buf;
	buf.load(buffer, size);

	// Magic + version
	uint32_t magic = buf.read_u32();
	if(magic != 0x59474F53) {
		lua_gc(lua->lua_state, LUA_GCRESTART, 0);
		return -1;
	}
	uint32_t version = buf.read_u32();
	if(version != 2) {
		lua_gc(lua->lua_state, LUA_GCRESTART, 0);
		return -2;
	}

	// RNG state
	RNG::Xoshiro256StarStar::StateType rng_state;
	for(int i = 0; i < 4; i++)
		rng_state[i] = buf.read_u64();
	random.set_state(rng_state);

	// Messages queue
	uint32_t msg_count = buf.read_u32();
	messages.clear();
	for(uint32_t i = 0; i < msg_count; i++) {
		auto data = buf.read_bytes();
		if(!data.empty()) {
			auto& msg = messages.emplace_back(data[0]);
			if(data.size() > 1)
				msg.write(data.data() + 1, data.size() - 1);
		}
	}

	// Buff and query_buffer
	buff = buf.read_bytes();
	query_buffer = buf.read_bytes();

	// === Read manifests ===
	// Card manifest
	uint32_t snap_card_count = buf.read_u32();
	std::unordered_set<uint32_t> snap_card_ids;
	for(uint32_t i = 0; i < snap_card_count; i++)
		snap_card_ids.insert(buf.read_u32());

	// Effect manifest
	uint32_t snap_effect_count = buf.read_u32();
	std::unordered_set<uint32_t> snap_effect_ids;
	for(uint32_t i = 0; i < snap_effect_count; i++)
		snap_effect_ids.insert(buf.read_u32());

	// Group manifest
	uint32_t snap_group_count = buf.read_u32();
	std::unordered_set<uint32_t> snap_group_ids;
	for(uint32_t i = 0; i < snap_group_count; i++)
		snap_group_ids.insert(buf.read_u32());


	// === Reconcile object sets ===

	// Cards: build current cardid map, delete extras, create missing
	std::unordered_map<uint32_t, card*> cur_cards;
	for(auto* c : cards)
		cur_cards[c->cardid] = c;
	// Delete cards not in snapshot
	for(auto it = cards.begin(); it != cards.end(); ) {
		card* c = *it;
		if(snap_card_ids.find(c->cardid) == snap_card_ids.end()) {
			it = cards.erase(it);
			delete c;
		} else {
			++it;
		}
	}
	// Create cards that are in snapshot but not current
	// (tokens created before snapshot, destroyed after -- rare in LOB)
	for(uint32_t cid : snap_card_ids) {
		if(cur_cards.find(cid) == cur_cards.end()) {
			card* nc = new card(this);
			nc->cardid = cid;
			cards.insert(nc);
			lua->register_card(nc);
		}
	}


	// Effects: delete extras, create missing
	std::unordered_map<uint32_t, effect*> cur_effects;
	for(auto* e : effects)
		cur_effects[e->id] = e;
	// Delete effects not in snapshot (unregister frees their Lua refs, that's fine)
	for(auto it = effects.begin(); it != effects.end(); ) {
		effect* e = *it;
		if(snap_effect_ids.find(e->id) == snap_effect_ids.end()) {
			it = effects.erase(it);
			lua->unregister_effect(e);
			delete e;
		} else {
			++it;
		}
	}
	// Create effects that are in snapshot but not current
	for(uint32_t eid : snap_effect_ids) {
		if(cur_effects.find(eid) == cur_effects.end()) {
			effect* ne = new effect(this);
			ne->id = eid;
			effects.insert(ne);
			lua->register_effect(ne);
		}
	}


	// Groups: orphan Lua userdata (so __gc becomes a no-op), then remove
	// from tracking set. We intentionally leak the C++ objects because
	// tevent's owned_lua<group> may still reference them; they'll be
	// cleaned up when the event lists are deserialized below.
	for(auto* g : groups) {
		lua->orphan_group(g);
		g->container.clear();
		g->is_iterator_dirty = true;
		// Prevent decr_ref from ever reaching 0 (which would call
		// luaL_unref with ref_handle=0, corrupting the Lua registry).
		// We'll delete these ourselves in the destructor after Lua is gone.
		g->num_ref = 1 << 30;
		orphaned_groups.push_back(g);
	}
	groups.clear();
	std::unordered_map<uint32_t, group*> gid_map;
	for(uint32_t gid : snap_group_ids) {
		group* ng = new group(this);
		groups.insert(ng);
		lua->register_group(ng);
		gid_map[gid] = ng;
	}


	// === Build ID maps from reconciled objects ===
	IdMaps maps;
	maps.clear();
	for(auto* c : cards) {
		maps.card_to_id[c] = c->cardid;
		maps.id_to_card[c->cardid] = c;
	}
	for(auto* e : effects) {
		maps.effect_to_id[e] = e->id;
		maps.id_to_effect[e->id] = e;
	}
	for(auto& [gid, g] : gid_map) {
		maps.group_to_id[g] = gid;
		maps.id_to_group[gid] = g;
	}


	// === Deserialize object data ===

	// Cards
	uint32_t card_count = buf.read_u32();
	for(uint32_t i = 0; i < card_count; i++) {
		uint32_t cid = buf.peek_u32();
		auto it = maps.id_to_card.find(cid);
		assert(it != maps.id_to_card.end());
		deserialize_card(buf, it->second, maps, this);
	}


	// Effects + cloned Lua refs
	uint32_t effect_count = buf.read_u32();
	for(uint32_t i = 0; i < effect_count; i++) {
		uint32_t eid = buf.peek_u32();
		auto it = maps.id_to_effect.find(eid);
		assert(it != maps.id_to_effect.end());
		effect* e = it->second;
		// Unref old Lua refs before overwriting
		if(e->condition) luaL_unref(lua->lua_state, LUA_REGISTRYINDEX, e->condition);
		if(e->cost) luaL_unref(lua->lua_state, LUA_REGISTRYINDEX, e->cost);
		if(e->target) luaL_unref(lua->lua_state, LUA_REGISTRYINDEX, e->target);
		if(e->value && e->is_flag(EFFECT_FLAG_FUNC_VALUE))
			luaL_unref(lua->lua_state, LUA_REGISTRYINDEX, e->value);
		if(e->operation) luaL_unref(lua->lua_state, LUA_REGISTRYINDEX, e->operation);
		deserialize_effect(buf, e, maps);
		bool val_is_ref = buf.read_bool();
		// Read cloned refs and assign them (these are our owned clones)
		e->condition = buf.read_i32();
		e->cost      = buf.read_i32();
		e->target    = buf.read_i32();
		int32_t snap_value = buf.read_i32();
		if(val_is_ref)
			e->value = snap_value;
		// else: value was already restored by deserialize_effect
		e->operation = buf.read_i32();
	}


	// The cloned refs are now owned by the effects -- remove them from snapshot_lua_refs
	// so they don't get double-freed
	snapshot_lua_refs.clear();

	// Uncopy effects
	uint32_t uncopy_count = buf.read_u32();
	uncopy.clear();
	for(uint32_t i = 0; i < uncopy_count; i++) {
		effect* e = buf.read_effect_id(maps);
		if(e) uncopy.insert(e);
	}

	// Assumes
	uint32_t assumes_count = buf.read_u32();
	assumes.clear();
	for(uint32_t i = 0; i < assumes_count; i++) {
		card* c = buf.read_card_id(maps);
		if(c) assumes.insert(c);
	}

	// Groups
	uint32_t group_count = buf.read_u32();
	for(uint32_t i = 0; i < group_count; i++) {
		uint32_t gid = buf.read_u32();
		auto it = gid_map.find(gid);
		assert(it != gid_map.end());
		group* g = it->second;
		buf.read_card_set(g->container, maps);
		g->is_readonly = buf.read_bool();
		g->is_iterator_dirty = true;
	}


	// Field
	deserialize_field(buf, game_field, maps, this);

	// Resume GC and collect orphaned group userdata immediately so they
	// don't accumulate across repeated save/restore cycles (MCTS).
	lua_gc(lua->lua_state, LUA_GCRESTART, 0);
	lua_gc(lua->lua_state, LUA_GCCOLLECT, 0);

	return 0;
}

card_data::card_data(const OCG_CardData& data) {
#define COPY(val) do { val = data.val; } while(0)
	COPY(code);
	COPY(alias);
	COPY(type);
	COPY(level);
	COPY(attribute);
	COPY(race);
	COPY(attack);
	COPY(defense);
	COPY(lscale);
	COPY(rscale);
	COPY(link_marker);
#undef COPY
	if(data.setcodes == nullptr)
		return;
	uint16_t sc = 0;
	uint16_t* ptr = data.setcodes;
	for(;;) {
		std::memcpy(&sc, ptr++, sizeof(uint16_t));
		if(sc == 0)
			break;
		setcodes.insert(sc);
	}
}
