/*
 * Duel state serialization for O(1) MCTS save/restore.
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include <cassert>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

class card;
class effect;
class group;

struct IdMaps {
	std::unordered_map<card*, uint32_t> card_to_id;
	std::unordered_map<uint32_t, card*> id_to_card;
	std::unordered_map<effect*, uint32_t> effect_to_id;
	std::unordered_map<uint32_t, effect*> id_to_effect;
	std::unordered_map<group*, uint32_t> group_to_id;
	std::unordered_map<uint32_t, group*> id_to_group;

	void clear() {
		card_to_id.clear();
		id_to_card.clear();
		effect_to_id.clear();
		id_to_effect.clear();
		group_to_id.clear();
		id_to_group.clear();
	}
};

class SerializeBuffer {
	std::vector<uint8_t> data_;
	size_t read_pos_{0};
public:
	// --- Write primitives ---
	void write_u8(uint8_t v) { data_.push_back(v); }
	void write_u16(uint16_t v) { write_raw(&v, sizeof(v)); }
	void write_u32(uint32_t v) { write_raw(&v, sizeof(v)); }
	void write_u64(uint64_t v) { write_raw(&v, sizeof(v)); }
	void write_i32(int32_t v) { write_raw(&v, sizeof(v)); }
	void write_i16(int16_t v) { write_raw(&v, sizeof(v)); }
	void write_bool(bool v) { write_u8(v ? 1 : 0); }

	void write_raw(const void* src, size_t len) {
		auto sz = data_.size();
		data_.resize(sz + len);
		std::memcpy(data_.data() + sz, src, len);
	}

	void write_bytes(const std::vector<uint8_t>& v) {
		write_u32(static_cast<uint32_t>(v.size()));
		if(!v.empty())
			write_raw(v.data(), v.size());
	}

	// --- Read primitives ---
	uint8_t read_u8() {
		assert(read_pos_ < data_.size());
		return data_[read_pos_++];
	}
	uint16_t read_u16() { uint16_t v; read_raw(&v, sizeof(v)); return v; }
	uint32_t read_u32() { uint32_t v; read_raw(&v, sizeof(v)); return v; }
	uint32_t peek_u32() const {
		assert(read_pos_ + sizeof(uint32_t) <= data_.size());
		uint32_t v;
		std::memcpy(&v, data_.data() + read_pos_, sizeof(v));
		return v;
	}
	uint64_t read_u64() { uint64_t v; read_raw(&v, sizeof(v)); return v; }
	int32_t read_i32() { int32_t v; read_raw(&v, sizeof(v)); return v; }
	int16_t read_i16() { int16_t v; read_raw(&v, sizeof(v)); return v; }
	bool read_bool() { return read_u8() != 0; }

	void read_raw(void* dst, size_t len) {
		assert(read_pos_ + len <= data_.size());
		std::memcpy(dst, data_.data() + read_pos_, len);
		read_pos_ += len;
	}

	std::vector<uint8_t> read_bytes() {
		uint32_t sz = read_u32();
		std::vector<uint8_t> v(sz);
		if(sz > 0)
			read_raw(v.data(), sz);
		return v;
	}

	// --- Pointer-as-ID write/read ---
	void write_card_id(card* p, const IdMaps& maps) {
		if(!p) { write_u32(0); return; }
		auto it = maps.card_to_id.find(p);
		assert(it != maps.card_to_id.end());
		write_u32(it->second);
	}
	card* read_card_id(const IdMaps& maps) {
		uint32_t id = read_u32();
		if(id == 0) return nullptr;
		auto it = maps.id_to_card.find(id);
		assert(it != maps.id_to_card.end());
		return it->second;
	}

	void write_effect_id(effect* p, const IdMaps& maps) {
		if(!p) { write_u32(0); return; }
		auto it = maps.effect_to_id.find(p);
		assert(it != maps.effect_to_id.end());
		write_u32(it->second);
	}
	effect* read_effect_id(const IdMaps& maps) {
		uint32_t id = read_u32();
		if(id == 0) return nullptr;
		auto it = maps.id_to_effect.find(id);
		assert(it != maps.id_to_effect.end());
		return it->second;
	}

	void write_group_id(group* p, const IdMaps& maps) {
		if(!p) { write_u32(0); return; }
		auto it = maps.group_to_id.find(p);
		assert(it != maps.group_to_id.end());
		write_u32(it->second);
	}
	group* read_group_id(const IdMaps& maps) {
		uint32_t id = read_u32();
		if(id == 0) return nullptr;
		auto it = maps.id_to_group.find(id);
		assert(it != maps.id_to_group.end());
		return it->second;
	}

	// --- Container helpers ---
	template<typename SetType>
	void write_card_set(const SetType& s, const IdMaps& maps) {
		write_u32(static_cast<uint32_t>(s.size()));
		for(card* c : s)
			write_card_id(c, maps);
	}

	template<typename SetType>
	void read_card_set(SetType& s, const IdMaps& maps) {
		uint32_t n = read_u32();
		s.clear();
		for(uint32_t i = 0; i < n; i++) {
			card* c = read_card_id(maps);
			if(c) s.insert(s.end(), c);
		}
	}

	void write_card_vector(const std::vector<card*>& v, const IdMaps& maps) {
		write_u32(static_cast<uint32_t>(v.size()));
		for(card* c : v)
			write_card_id(c, maps);
	}

	void read_card_vector(std::vector<card*>& v, const IdMaps& maps) {
		uint32_t n = read_u32();
		v.resize(n);
		for(uint32_t i = 0; i < n; i++)
			v[i] = read_card_id(maps);
	}

	void write_effect_set(const std::vector<effect*>& v, const IdMaps& maps) {
		write_u32(static_cast<uint32_t>(v.size()));
		for(effect* e : v)
			write_effect_id(e, maps);
	}

	void read_effect_set(std::vector<effect*>& v, const IdMaps& maps) {
		uint32_t n = read_u32();
		v.resize(n);
		for(uint32_t i = 0; i < n; i++)
			v[i] = read_effect_id(maps);
	}

	template<typename Container>
	void write_effect_container(const Container& c, const IdMaps& maps) {
		write_u32(static_cast<uint32_t>(c.size()));
		for(auto& [key, eff] : c) {
			write_u32(key);
			write_effect_id(eff, maps);
		}
	}

	template<typename Container>
	void read_effect_container(Container& c, const IdMaps& maps) {
		uint32_t n = read_u32();
		c.clear();
		for(uint32_t i = 0; i < n; i++) {
			uint32_t key = read_u32();
			effect* eff = read_effect_id(maps);
			c.emplace(key, eff);
		}
	}

	// --- Access ---
	const uint8_t* raw_data() const { return data_.data(); }
	size_t size() const { return data_.size(); }
	void reset_read() { read_pos_ = 0; }
	bool at_end() const { return read_pos_ >= data_.size(); }
	size_t read_position() const { return read_pos_; }

	void load(const void* buf, size_t len) {
		data_.assign(static_cast<const uint8_t*>(buf),
		             static_cast<const uint8_t*>(buf) + len);
		read_pos_ = 0;
	}
};

// Forward declarations for serialize functions (implemented in serialize.cpp)
struct card_state;
struct tevent;
struct optarget;
struct chain;
class duel;

void serialize_card_state(SerializeBuffer& buf, const card_state& cs, const IdMaps& maps);
void deserialize_card_state(SerializeBuffer& buf, card_state& cs, const IdMaps& maps);
void serialize_card(SerializeBuffer& buf, card* c, const IdMaps& maps);
void deserialize_card(SerializeBuffer& buf, card* c, const IdMaps& maps, duel* pduel);
void serialize_effect(SerializeBuffer& buf, effect* e, const IdMaps& maps);
void deserialize_effect(SerializeBuffer& buf, effect* e, const IdMaps& maps);
void serialize_tevent(SerializeBuffer& buf, const tevent& ev, const IdMaps& maps);
void deserialize_tevent(SerializeBuffer& buf, tevent& ev, const IdMaps& maps, duel* pduel);
void serialize_optarget(SerializeBuffer& buf, const optarget& ot, const IdMaps& maps);
void deserialize_optarget(SerializeBuffer& buf, optarget& ot, const IdMaps& maps, duel* pduel);
void serialize_chain(SerializeBuffer& buf, const chain& ch, const IdMaps& maps);
void deserialize_chain(SerializeBuffer& buf, chain& ch, const IdMaps& maps, duel* pduel);

struct field_info;
struct player_info;
struct field_effect;
struct processor;
class field;

void serialize_field_info(SerializeBuffer& buf, const field_info& fi);
void deserialize_field_info(SerializeBuffer& buf, field_info& fi);
void serialize_player_info(SerializeBuffer& buf, const player_info& pi, const IdMaps& maps);
void deserialize_player_info(SerializeBuffer& buf, player_info& pi, const IdMaps& maps);
void serialize_field_effect(SerializeBuffer& buf, const field_effect& fe, const IdMaps& maps);
void deserialize_field_effect(SerializeBuffer& buf, field_effect& fe, const IdMaps& maps);
void serialize_processor(SerializeBuffer& buf, const processor& proc, const IdMaps& maps);
void deserialize_processor(SerializeBuffer& buf, processor& proc, const IdMaps& maps, duel* pduel);
void serialize_field(SerializeBuffer& buf, const field* f, const IdMaps& maps);
void deserialize_field(SerializeBuffer& buf, field* f, const IdMaps& maps, duel* pduel);

#endif /* SERIALIZE_H_ */
