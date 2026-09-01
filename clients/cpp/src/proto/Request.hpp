#ifndef HAZE_SERVER_REQUEST_HPP
#define HAZE_SERVER_REQUEST_HPP

#include "MessagePackRPC.hpp"
#include "Object.hpp"

#include <cstdint>
#include <string>

namespace Haze {

struct Request {
  RPCType type;
  uint32_t msgid;
  std::string method;
  ObjectArray parameters;
};

Object* RequestParamGet(Request& rq, uint32_t index);
const Object* RequestParamGet(const Request& rq, uint32_t index);

bool RequestParamAppend(Request& rq, Object param);

RPCType RequestTypeGet(const Request& rq);
uint32_t RequestMsgId(const Request& rq);
const std::string& RequestMethod(const Request& rq);

uint32_t RequestParamCount(const Request& rq);

bool RequestParamIsType(const Request& rq,
                        ObjectType type,
                        uint32_t index);

Request RequestUnmarshal(const BinaryBuffer& buffer);
BinaryBuffer RequestMarshal(const Request& request);

Request RequestNew();

void RequestSetMethod(Request& request, const std::string& method);

const char* RequestPrint(const Request& request);

} // namespace Haze

#endif
