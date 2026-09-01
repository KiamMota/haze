#ifndef HAZE_SERVER_MESSAGE_HPP
#define HAZE_SERVER_MESSAGE_HPP

#include <cstdint>

namespace Haze {

// 'enum class' garante type-safety e evita poluição do namespace global.
// Definimos o tipo base como uint8_t para economizar memória (já que são poucos valores).
enum class RPCType : uint8_t {
    Request = 0,
    Response = 1,
    Notification = 2
};

enum class RPCError : uint8_t {
    None = 0,
    MalformedRequest,
    ParseError,
    InvalidRequest,
    MethodNotFound,
    InvalidParams,
    InternalError,
    ServerError,
    Timeout,
    ConnectionClosed,
    Unknown
};

} // namespace Haze

#endif // HAZE_SERVER_MESSAGE_HPP
