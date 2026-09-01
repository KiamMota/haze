#include "Object.hpp"
#include <algorithm>
#include <cstring>

namespace Haze {

// --- Implementação do ObjectMapEntry ---

bool ObjectMapEntry::operator==(const ObjectMapEntry &other) const {
  return key == other.key && value == other.value;
}

// --- Construtores da Classe Object ---

Object::Object() : type_(ObjectType::Und), value_(std::monostate{}) {}

Object::Object(ObjectType type) : type_(type), value_(std::monostate{}) {}

Object::Object(bool v) : type_(ObjectType::Bool), value_(v) {}

Object::Object(int64_t v) : type_(ObjectType::Int), value_(v) {}

Object::Object(uint64_t v) : type_(ObjectType::UInt), value_(v) {}

Object::Object(float v) : type_(ObjectType::Float), value_(v) {}

Object::Object(double v) : type_(ObjectType::Double), value_(v) {}

Object::Object(const char *str)
    : type_(ObjectType::Str),
      value_(str ? std::string(str) : std::string("NULL")) {}

Object::Object(std::string str)
    : type_(ObjectType::Str), value_(std::move(str)) {}

Object::Object(BinaryBuffer bin) : type_(ObjectType::Bin), value_(std::move(bin)) {}

Object::Object(ObjectArray arr)
    : type_(ObjectType::Array), value_(std::move(arr)) {}

Object::Object(ObjectMapTable map)
    : type_(ObjectType::Map), value_(std::move(map)) {}

// --- Factory Methods ---

Object Object::CreateNil() { return Object(ObjectType::Nil); }

Object Object::CreateBool(bool v) { return Object(v); }

Object Object::CreateInt(int64_t v) { return Object(v); }

Object Object::CreateUInt(uint64_t v) { return Object(v); }

Object Object::CreateFloat(float v) { return Object(v); }

Object Object::CreateDouble(double v) { return Object(v); }

Object Object::CreateStr(const std::string &str) { return Object(str); }

// --- Getters ---

ObjectType Object::GetType() const { return type_; }

const Object::ValueType &Object::GetValue() const { return value_; }

size_t Object::GetSize() const {
  switch (type_) {
  case ObjectType::Bool:
    return sizeof(bool);
  case ObjectType::Int:
    return sizeof(int64_t);
  case ObjectType::UInt:
    return sizeof(uint64_t);
  case ObjectType::Float:
    return sizeof(float);
  case ObjectType::Double:
    return sizeof(double);
  case ObjectType::Str:
    if (auto pval = std::get_if<std::string>(&value_)) {
      return pval->size();
    }
    return 0;
  case ObjectType::Array:
    return ArrayLen();
  case ObjectType::Map:
    return MapLen();
  default:
    return 0;
  }
}

// --- Operações de Array ---

bool Object::ArrayAppend(const Object &obj) {
  if (type_ != ObjectType::Array) {
    type_ = ObjectType::Array;
    value_ = ObjectArray{};
  }

  auto *arr = std::get_if<ObjectArray>(&value_);
  if (!arr)
    return false;

  arr->push_back(obj);
  return true;
}

Object *Object::ArrayGet(size_t index) {
  auto *arr = std::get_if<ObjectArray>(&value_);
  if (!arr || index >= arr->size())
    return nullptr;
  return &((*arr)[index]);
}

const Object *Object::ArrayGet(size_t index) const {
  const auto *arr = std::get_if<ObjectArray>(&value_);
  if (!arr || index >= arr->size())
    return nullptr;
  return &((*arr)[index]);
}

bool Object::ArrayRemove(size_t index) {
  auto *arr = std::get_if<ObjectArray>(&value_);
  if (!arr || index >= arr->size())
    return false;

  arr->erase(arr->begin() + index);
  return true;
}

size_t Object::ArrayLen() const {
  const auto *arr = std::get_if<ObjectArray>(&value_);
  return arr ? arr->size() : 0;
}

// --- Operações de Map ---

bool Object::MapSet(const Object &key, const Object &value) {
  if (type_ != ObjectType::Map) {
    type_ = ObjectType::Map;
    value_ = ObjectMapTable{};
  }

  auto *map = std::get_if<ObjectMapTable>(&value_);
  if (!map)
    return false;

  for (auto &entry : *map) {
    if (entry.key == key) {
      entry.value = value;
      return true;
    }
  }

  map->push_back(ObjectMapEntry{key, value});
  return true;
}

Object *Object::MapGet(const Object &key) {
  auto *map = std::get_if<ObjectMapTable>(&value_);
  if (!map)
    return nullptr;

  for (auto &entry : *map) {
    if (entry.key == key) {
      return &entry.value;
    }
  }
  return nullptr;
}

const Object *Object::MapGet(const Object &key) const {
  const auto *map = std::get_if<ObjectMapTable>(&value_);
  if (!map)
    return nullptr;

  for (const auto &entry : *map) {
    if (entry.key == key) {
      return &entry.value;
    }
  }
  return nullptr;
}

bool Object::MapRemove(const Object &key) {
  auto *map = std::get_if<ObjectMapTable>(&value_);
  if (!map)
    return false;

  auto it = std::remove_if(
      map->begin(), map->end(),
      [&key](const ObjectMapEntry &entry) { return entry.key == key; });

  if (it != map->end()) {
    map->erase(it, map->end());
    return true;
  }
  return false;
}

size_t Object::MapLen() const {
  const auto *map = std::get_if<ObjectMapTable>(&value_);
  return map ? map->size() : 0;
}

// --- Operadores de Comparação ---

bool Object::operator==(const Object &other) const {
  if (type_ != other.type_)
    return false;

  switch (type_) {
  case ObjectType::Und:
  case ObjectType::Nil:
    return true;
  case ObjectType::Bool:
    return std::get<bool>(value_) == std::get<bool>(other.value_);
  case ObjectType::Int:
    return std::get<int64_t>(value_) == std::get<int64_t>(other.value_);
  case ObjectType::UInt:
    return std::get<uint64_t>(value_) == std::get<uint64_t>(other.value_);
  case ObjectType::Float:
    return std::get<float>(value_) == std::get<float>(other.value_);
  case ObjectType::Double:
    return std::get<double>(value_) == std::get<double>(other.value_);
  case ObjectType::Str:
    return std::get<std::string>(value_) == std::get<std::string>(other.value_);
  case ObjectType::Bin:
    return std::get<BinaryBuffer>(value_) ==
           std::get<BinaryBuffer>(other.value_);
  case ObjectType::Array:
    return std::get<ObjectArray>(value_) == std::get<ObjectArray>(other.value_);
  case ObjectType::Map:
    return std::get<ObjectMapTable>(value_) ==
           std::get<ObjectMapTable>(other.value_);
  default:
    return false;
  }
}

bool Object::operator!=(const Object &other) const { return !(*this == other); }

} // namespace Haze
