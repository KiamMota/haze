#ifndef HAZE_RESPONSE_HPP
#define HAZE_RESPONSE_HPP

#include "Object.hpp"
#include "MessagePackRPC.hpp" // Inclui o novo enum class RPCType
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace Haze {

class Response {
public:
    // Construtores
    Response();
    Response(uint32_t msgid, RPCType type);

    // Construtores de Cópia e Movimento padrão (RAII cuida da memória)
    Response(const Response&) = default;
    Response(Response&&) noexcept = default;
    Response& operator=(const Response&) = default;
    Response& operator=(Response&&) noexcept = default;
    ~Response() = default;

    // Serialização / Desserialização
    BinaryBuffer Marshal() const;
    static std::optional<Response> Unmarshal(const BinaryBuffer& b);

    // Getters
    const Object& GetResult() const;
    const Object& GetError() const;
    uint32_t GetMsgId() const;
    RPCType GetType() const;

    // Setters
    void SetResult(const Object& result);
    void SetError(const Object& error);
    void SetMsgId(uint32_t msgid);
    void SetType(RPCType type);

    // Factory Methods
    static Response CreateStrResult(uint32_t msgid, const std::string& result);
    static Response CreateStrArrayResult(uint32_t msgid, const std::vector<std::string>& vec);
    static Response CreateError(uint32_t msgid, const std::string& err);
    static Response CreateNilResult(uint32_t msgid);

private:
    RPCType type_{RPCType::Response}; // Valor padrão usando o enum class
    uint32_t msgid_{0};
    
    // Os objetos armazenados por valor
    Object error_{Object::CreateNil()};
    Object result_{Object::CreateNil()};
};

} // namespace Haze

#endif // HAZE_RESPONSE_HPP
