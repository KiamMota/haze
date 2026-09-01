#include "Request.hpp"
#include "msgpack.h"

#include <utility>

namespace Haze {

Object* RequestParamGet(Request& rq, uint32_t index) {
    if (index >= rq.parameters.size())
        return nullptr;

    return &rq.parameters[index];
}

const Object* RequestParamGet(const Request& rq, uint32_t index) {
    if (index >= rq.parameters.size())
        return nullptr;

    return &rq.parameters[index];
}

bool RequestParamAppend(Request& rq, Object param) {
    rq.parameters.push_back(std::move(param));
    return true;
}

RPCType RequestTypeGet(const Request& rq) {
    return rq.type;
}

uint32_t RequestMsgId(const Request& rq) {
    return rq.msgid;
}

const std::string& RequestMethod(const Request& rq) {
    return rq.method;
}

uint32_t RequestParamCount(const Request& rq) {
    return static_cast<uint32_t>(rq.parameters.size());
}

bool RequestParamIsType(const Request& rq,
                        ObjectType type,
                        uint32_t index) {
    const Object* param = RequestParamGet(rq, index);

    return param && param->GetType() == type;
}

Request RequestNew() {
    Request request;

    request.type = RPCType::Request;
    request.msgid = 0;

    return request;
}

void RequestSetMethod(Request& request, const std::string& method) {
    request.method = method;
}

const char* RequestPrint(const Request& request) {
    static std::string output;

    output = "Request{";
    output += "type=" +
              std::to_string(static_cast<uint8_t>(request.type));
    output += ", msgid=" + std::to_string(request.msgid);
    output += ", method=\"" + request.method + "\"";
    output += ", params=" + std::to_string(request.parameters.size());
    output += "}";

    return output.c_str();
}

}
