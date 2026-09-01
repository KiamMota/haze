#include "Response.hpp"
#include "mpack/mpack-common.h"
#include "mpack/mpack-expect.h"
#include "mpack/mpack-reader.h"
#include "mpack/mpack-writer.h"

#include <cstdlib>
#include <cstring>

namespace Haze {

// --- Helpers de Serialização MPack para Object ---

static void ObjectMarshalMPack(mpack_writer_t* writer, const Object& obj) {
    switch (obj.GetType()) {
        case ObjectType::Und:
        case ObjectType::Nil:
            mpack_write_nil(writer);
            break;

        case ObjectType::Bool:
            mpack_write_bool(writer, std::get<bool>(obj.GetValue()));
            break;

        case ObjectType::Int:
            mpack_write_int(writer, std::get<int64_t>(obj.GetValue()));
            break;

        case ObjectType::UInt:
            mpack_write_uint(writer, std::get<uint64_t>(obj.GetValue()));
            break;

        case ObjectType::Float:
            mpack_write_float(writer, std::get<float>(obj.GetValue()));
            break;

        case ObjectType::Double:
            mpack_write_double(writer, std::get<double>(obj.GetValue()));
            break;

        case ObjectType::Str: {
            const std::string& str = std::get<std::string>(obj.GetValue());
            mpack_write_str(writer, str.c_str(), static_cast<uint32_t>(str.length()));
            break;
        }

        case ObjectType::Bin: {
            const BinaryBuffer& bin = std::get<BinaryBuffer>(obj.GetValue());
            if (!bin.empty()) {
                mpack_write_bin(writer, reinterpret_cast<const char*>(bin.data()), static_cast<uint32_t>(bin.size()));
            } else {
                mpack_write_nil(writer);
            }
            break;
        }

        case ObjectType::Array: {
            const ObjectArray& arr = std::get<ObjectArray>(obj.GetValue());
            mpack_start_array(writer, static_cast<uint32_t>(arr.size()));
            for (const auto& child : arr) {
                ObjectMarshalMPack(writer, child);
            }
            mpack_finish_array(writer);
            break;
        }
        
        case ObjectType::Map: {
            // Caso decida implementar serialização de Map depois
            mpack_write_nil(writer);
            break;
        }

        default:
            mpack_write_nil(writer);
            break;
    }
}

// --- Helpers de Deserialização MPack para Object ---

static Object ObjectUnmarshalMPack(mpack_reader_t* reader) {
    mpack_tag_t tag = mpack_read_tag(reader);
    if (mpack_reader_error(reader) != mpack_ok) {
        return Object::CreateNil();
    }

    switch (mpack_tag_type(&tag)) {
        case mpack_type_nil:
            return Object::CreateNil();

        case mpack_type_bool:
            return Object::CreateBool(mpack_tag_bool_value(&tag));

        case mpack_type_int:
            return Object::CreateInt(mpack_tag_int_value(&tag));

        case mpack_type_uint:
            return Object::CreateUInt(mpack_tag_uint_value(&tag));

        case mpack_type_float:
            return Object::CreateFloat(mpack_tag_float_value(&tag));

        case mpack_type_double:
            return Object::CreateDouble(mpack_tag_double_value(&tag));

        case mpack_type_str: {
            uint32_t len = mpack_tag_str_length(&tag);
            std::string str(len, '\0');
            mpack_read_bytes(reader, &str[0], len);
            mpack_done_str(reader);
            return Object::CreateStr(str);
        }

        case mpack_type_bin: {
            uint32_t len = mpack_tag_bin_length(&tag);
            BinaryBuffer bin(len);
            mpack_read_bytes(reader, reinterpret_cast<char*>(bin.data()), len);
            mpack_done_bin(reader);
            return Object(std::move(bin)); 
        }

        case mpack_type_array: {
            uint32_t count = mpack_tag_array_count(&tag);
            ObjectArray arr;
            arr.reserve(count);
            for (uint32_t i = 0; i < count; i++) {
                arr.push_back(ObjectUnmarshalMPack(reader));
            }
            mpack_done_array(reader);
            return Object(std::move(arr)); 
        }

        default:
            return Object::CreateNil();
    }
}


// --- Construtores ---

Response::Response() = default;

Response::Response(uint32_t msgid, RPCType type)
    : type_(type), msgid_(msgid) {}


// --- Serialização / Desserialização ---

BinaryBuffer Response::Marshal() const {
    char* data = nullptr;
    size_t size = 0;

    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &data, &size);

    /* Formato MsgPack-RPC Response: [type, msgid, error, result] */
    mpack_start_array(&writer, 4);

    mpack_write_u8(&writer, static_cast<uint8_t>(type_));
    mpack_write_u32(&writer, msgid_);

    /* Error */
    ObjectMarshalMPack(&writer, error_);

    /* Result */
    ObjectMarshalMPack(&writer, result_);

    mpack_finish_array(&writer);

    mpack_error_t err = mpack_writer_destroy(&writer);
    if (err != mpack_ok) {
        if (data) {
            std::free(data); 
        }
        return BinaryBuffer{};
    }

    BinaryBuffer buffer(data, data + size);
    std::free(data); 

    return buffer;
}

std::optional<Response> Response::Unmarshal(const BinaryBuffer& b) {
    if (b.empty()) {
        return std::nullopt;
    }

    mpack_reader_t reader;
    mpack_reader_init_data(&reader, reinterpret_cast<const char*>(b.data()), b.size());

    uint32_t count = mpack_expect_array(&reader);
    if (mpack_reader_error(&reader) != mpack_ok || count != 4) {
        mpack_reader_destroy(&reader);
        return std::nullopt;
    }

    Response response;

    /* 1. type */
    uint8_t raw_type = mpack_expect_u8(&reader);
    if (mpack_reader_error(&reader) != mpack_ok || static_cast<RPCType>(raw_type) != RPCType::Response) {
        mpack_reader_destroy(&reader);
        return std::nullopt;
    }
    response.SetType(static_cast<RPCType>(raw_type));

    /* 2. msgid */
    response.SetMsgId(mpack_expect_u32(&reader));
    if (mpack_reader_error(&reader) != mpack_ok) {
        mpack_reader_destroy(&reader);
        return std::nullopt;
    }

    /* 3. error */
    response.SetError(ObjectUnmarshalMPack(&reader));
    if (mpack_reader_error(&reader) != mpack_ok) {
        mpack_reader_destroy(&reader);
        return std::nullopt;
    }

    /* 4. result */
    response.SetResult(ObjectUnmarshalMPack(&reader));
    if (mpack_reader_error(&reader) != mpack_ok) {
        mpack_reader_destroy(&reader);
        return std::nullopt;
    }

    mpack_done_array(&reader);
    if (mpack_reader_error(&reader) != mpack_ok) {
        mpack_reader_destroy(&reader);
        return std::nullopt;
    }

    mpack_reader_destroy(&reader);
    return response;
}


// --- Getters ---

const Object& Response::GetResult() const { return result_; }
const Object& Response::GetError() const { return error_; }
uint32_t Response::GetMsgId() const { return msgid_; }
RPCType Response::GetType() const { return type_; }


// --- Setters ---

void Response::SetResult(const Object& result) { result_ = result; }
void Response::SetError(const Object& error) { error_ = error; }
void Response::SetMsgId(uint32_t msgid) { msgid_ = msgid; }
void Response::SetType(RPCType type) { type_ = type; }


// --- Factory Methods ---

Response Response::CreateStrResult(uint32_t msgid, const std::string& result) {
    Response r;
    r.SetMsgId(msgid);
    r.SetType(RPCType::Response);
    r.SetResult(Object::CreateStr(result));
    r.SetError(Object::CreateNil());
    return r;
}

Response Response::CreateError(uint32_t msgid, const std::string& err) {
    Response r;
    r.SetMsgId(msgid);
    r.SetType(RPCType::Response);
    r.SetError(Object::CreateStr(err));
    r.SetResult(Object::CreateNil());
    return r;
}

Response Response::CreateStrArrayResult(uint32_t msgid, const std::vector<std::string>& vec) {
    Response r;
    r.SetMsgId(msgid);
    r.SetType(RPCType::Response);
    
    ObjectArray arr;
    arr.reserve(vec.size());
    for (const auto& str : vec) {
        arr.push_back(Object::CreateStr(str));
    }
    
    r.SetResult(Object(std::move(arr)));
    r.SetError(Object::CreateNil());
    return r;
}

Response Response::CreateNilResult(uint32_t msgid) {
    Response r;
    r.SetMsgId(msgid);
    r.SetType(RPCType::Response);
    r.SetResult(Object::CreateNil());
    r.SetError(Object::CreateNil());
    return r;
}

} // namespace Haze
