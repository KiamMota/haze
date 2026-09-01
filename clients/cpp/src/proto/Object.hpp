#ifndef HAZE_OBJECT_HPP
#define HAZE_OBJECT_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace Haze {

class Object;
struct ObjectMapEntry; // Forward declaration

// Tipos de dados suportados
enum class ObjectType {
  Und = 0,
  Nil,
  Bool,
  Int,
  UInt,
  Float,
  Double,
  Str,
  Bin,
  Array,
  Map
};

// Classes de suporte para coleções
using ObjectArray = std::vector<Object>;
using ObjectMapTable = std::vector<ObjectMapEntry>;
using BinaryBuffer = std::vector<uint8_t>; // Substitui o RawBuffer

class Object {
public:
  // Define o tipo do valor contido na variante
  using ValueType = std::variant<std::monostate, // Representa Und / Nil
                                 bool,           // Bool
                                 int64_t,        // Int
                                 uint64_t,       // UInt
                                 float,          // Float
                                 double,         // Double
                                 std::string,    // Str
                                 BinaryBuffer,   // Bin
                                 ObjectArray,    // Array
                                 ObjectMapTable  // Map
                                 >;

  // Construtores Padrão
  Object(); // OBJ_UND por padrão
  explicit Object(ObjectType type);
  explicit Object(bool v);
  explicit Object(int64_t v);
  explicit Object(uint64_t v);
  explicit Object(float v);
  explicit Object(double v);
  explicit Object(const char *str);
  explicit Object(std::string str);
  explicit Object(BinaryBuffer bin); // Construtor binário
  explicit Object(ObjectArray arr);
  explicit Object(ObjectMapTable map);

  // Construtores de Cópia e Movimento Explícitos (Rule of 5/0)
  Object(const Object &) = default;
  Object(Object &&) noexcept = default;
  Object &operator=(const Object &) = default;
  Object &operator=(Object &&) noexcept = default;
  ~Object() = default;

  // Construtores estáticos (Factory Methods)
  static Object CreateNil();
  static Object CreateBool(bool v);
  static Object CreateInt(int64_t v);
  static Object CreateUInt(uint64_t v);
  static Object CreateFloat(float v);
  static Object CreateDouble(double v);
  static Object CreateStr(const std::string &str);

  // Getters
  ObjectType GetType() const;
  const ValueType &GetValue() const;
  size_t GetSize() const;

  // Métodos utilitários de checagem
  bool IsNil() const { return type_ == ObjectType::Nil; }
  bool IsBool() const { return type_ == ObjectType::Bool; }
  bool IsInt() const { return type_ == ObjectType::Int; }
  bool IsUInt() const { return type_ == ObjectType::UInt; }
  bool IsFloat() const { return type_ == ObjectType::Float; }
  bool IsDouble() const { return type_ == ObjectType::Double; }
  bool IsStr() const { return type_ == ObjectType::Str; }
  bool IsBin() const { return type_ == ObjectType::Bin; }
  bool IsArray() const { return type_ == ObjectType::Array; }
  bool IsMap() const { return type_ == ObjectType::Map; }

  // Métodos para Objetos do tipo Array
  bool ArrayAppend(const Object &obj);
  Object *ArrayGet(size_t index);
  const Object *ArrayGet(size_t index) const;
  bool ArrayRemove(size_t index);
  size_t ArrayLen() const;

  // Métodos para Objetos do tipo Map
  bool MapSet(const Object &key, const Object &value);
  Object *MapGet(const Object &key);
  const Object *MapGet(const Object &key) const;
  bool MapRemove(const Object &key);
  size_t MapLen() const;

  // Operadores de comparação
  bool operator==(const Object &other) const;
  bool operator!=(const Object &other) const;

private:
  ObjectType type_{ObjectType::Und};
  ValueType value_{std::monostate{}};
};

// Agora que a classe Object está totalmente declarada, o compilador
// sabe o tamanho de 'key' e 'value', resolvendo o erro de tipo incompleto.
struct ObjectMapEntry {
  Object key;
  Object value;

  bool operator==(const ObjectMapEntry &other) const;
};

} // namespace Haze

#endif // HAZE_OBJECT_HPP
