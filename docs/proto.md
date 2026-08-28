# Haze Protocol: MessagePack-RPC

## Overview

Haze uses MessagePack-RPC as the application protocol between clients and the Haze core. MessagePack-RPC combines a small binary serialization format (MessagePack) with a simple request/response RPC model.

MessagePack-RPC defines three message kinds: **Request**, **Response**, and **Notification**. A request identifies a method and its arguments; a response identifies the original request and carries either an error or a result; a notification is a one-way call with no response. Responses may be returned out of order, which allows pipelining and asynchronous processing. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

The protocol is transport-independent at the RPC level. A concrete Haze implementation can carry MessagePack-RPC messages over a byte-stream transport such as TCP, while the application handles framing and connection management separately.

---

## 1. The MessagePack Data Model

MessagePack is a binary serialization format. Serialization converts application values into MessagePack values and then into bytes; deserialization performs the inverse operation. [MessagePack Specification](https://github.com/msgpack/msgpack/blob/master/spec.md)

The fundamental types are:

| Type | Meaning | Example |
|---|---|---|
| `nil` | No value / null | `nil` |
| `bool` | Boolean value | `true`, `false` |
| `int` | Signed or unsigned integer | `42`, `-10` |
| `float` | Floating-point number | `3.14` |
| `str` | UTF-8 string | `"haze.ping"` |
| `bin` | Raw binary bytes | audio/file data |
| `array` | Ordered collection | `[1, 2, 3]` |
| `map` | Key/value collection | `{ "name": "Kick" }` |
| `ext` | Application-defined extension type | custom Haze value |

MessagePack chooses a compact binary representation for each value. Small integers and short strings can therefore occupy very little space, while larger values use formats with larger length fields. [MessagePack Specification](https://github.com/msgpack/msgpack/blob/master/spec.md)

### `nil`

Represents the absence of a value.

```text
nil
```

In RPC responses, `error` is `nil` when the method completed successfully. A failed request should normally have an error value and a `nil` result. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

### `bool`

Represents exactly two values:

```text
true
false
```

Useful for flags, switches, feature states, and predicates.

### Integers

MessagePack supports signed and unsigned integer representations, including 8-, 16-, 32-, and 64-bit forms, as well as compact fixed-width forms for small values. [MessagePack Specification](https://github.com/msgpack/msgpack/blob/master/spec.md)

Typical Haze uses include:

```text
track_id = 12
channel = 3
sample_rate = 48000
```

### Floating-point values

MessagePack supports 32-bit and 64-bit IEEE floating-point representations.

```text
volume = 0.75
pan = -0.25
pitch = 1.0
```

### Strings (`str`)

`str` stores UTF-8 text. Method names such as `haze.ping` belong to this type.

```text
"haze.ping"
"audio.track.create"
```

A `str` value has a byte length in its MessagePack representation. In C, do not assume an unpacked MessagePack string is NUL-terminated; use the explicit length when copying it.

### Binary (`bin`)

`bin` stores arbitrary bytes and is intended for non-text data.

```text
[raw audio bytes]
[serialized file data]
[opaque plugin data]
```

Binary data should not be treated as a C string.

### Arrays

An array is an ordered sequence of MessagePack values.

```text
[1, 2, 3]
```

Arrays are the core container used by MessagePack-RPC messages themselves.

### Maps

A map stores key/value pairs.

```text
{
    "name": "Kick",
    "volume": 0.8
}
```

Maps are useful when field names are more important than positional layout.

### Extension types (`ext`)

Extension types allow an application to define application-specific values while keeping them inside the MessagePack format. An extension carries an application-defined type identifier and raw bytes. [MessagePack Specification](https://github.com/msgpack/msgpack/blob/master/spec.md)

Haze can use extensions later for values that deserve a compact, typed wire representation without changing the base MessagePack type system.

---

## 2. What RPC Adds

MessagePack only defines how values become bytes. **RPC defines what those values mean as messages.**

The basic idea is:

```text
Client
  |
  |  Request
  v
Server / Haze Core
  |
  |  Response
  v
Client
```

An RPC request is effectively a remote function call:

```text
call("haze.add", 10, 20)
```

The server receives the method name and parameters, executes the corresponding handler, and returns the result.

---

## 3. Request Message

A MessagePack-RPC Request is a four-element array:

```text
[type, msgid, method, params]
```

The fields are defined as follows. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

| Field | Type | Purpose |
|---|---|---|
| `type` | integer | Must be `0` for Request |
| `msgid` | uint32 | Unique request identifier |
| `method` | str | RPC method name |
| `params` | array | Method arguments |

Example conceptually:

```text
[0, 41, "haze.ping", []]
```

A request with parameters could be:

```text
[0, 42, "haze.add", [10, 20]]
```

### `type`

The value `0` identifies the message as a Request.

### `msgid`

`msgid` is a 32-bit unsigned sequence number. The response uses the same value so the client can associate the response with the original request. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

This is especially important when multiple requests are in flight simultaneously.

### `method`

The method is a string containing the operation requested from the server.

Examples:

```text
haze.ping
haze.get_state
track.create
channel.set_volume
```

The RPC specification does not define the names of application methods; Haze owns that namespace.

### `params`

`params` is an array of arbitrary MessagePack values.

```text
[10, 20]
["Kick", 0.8]
[]
```

This lets a method define its own parameter contract.

---

## 4. Response Message

A Response is a four-element array:

```text
[type, msgid, error, result]
```

The fields are: [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

| Field | Type | Purpose |
|---|---|---|
| `type` | integer | Must be `1` for Response |
| `msgid` | uint32 | ID of the original request |
| `error` | any / nil | Error value, or `nil` on success |
| `result` | any / nil | Method result, or `nil` on error |

Successful response:

```text
[1, 42, nil, 30]
```

Failed response:

```text
[1, 42, {"code": 404, "message": "Track not found"}, nil]
```

The protocol leaves the concrete error and result types to the application.

---

## 5. Notification Message

A Notification is a three-element array:

```text
[type, method, params]
```

Its `type` is `2`. A notification has no `msgid` and does not receive a Response. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

Example:

```text
[2, "haze.transport.play", []]
```

Notifications are useful when the client does not need a return value and does not need request tracking.

---

## 6. Message Type Summary

```text
Request:
[0, msgid, method, params]

Response:
[1, msgid, error, result]

Notification:
[2, method, params]
```

The first element identifies the RPC message kind.

---

## 7. Request / Response Flow

A synchronous call conceptually works like this:

```text
Client
  |
  | [0, 100, "haze.add", [10, 20]]
  |
  v
Haze Core
  |
  | execute haze.add(10, 20)
  |
  v
  | [1, 100, nil, 30]
  |
  v
Client
```

The client uses `100` to correlate the response with the original request.

With asynchronous calls, several requests can be outstanding at the same time:

```text
Client                         Haze Core
  |                              |
  | Request 100 ---------------->|
  | Request 101 ---------------->|
  | Request 102 ---------------->|
  |                              |
  |<--------------- Response 101 |
  |<--------------- Response 102 |
  |<--------------- Response 100 |
```

MessagePack-RPC explicitly allows responses to be returned out of order. This is important for pipelining: a slow request does not have to block faster requests behind it. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

---

## 8. Serialization vs. RPC

These are separate layers:

```text
Application object
       |
       v
MessagePack type
       |
       v
MessagePack bytes
       |
       v
RPC message
       |
       v
Transport
```

MessagePack answers:

> How do I encode this value?

MessagePack-RPC answers:

> What does this encoded value mean in a request/response conversation?

For Haze, this distinction is important because the same MessagePack types can be used for RPC arguments, return values, state snapshots, and other serialized data.

---

## 9. Haze Request Model

The Haze `Request` abstraction maps naturally onto the MessagePack-RPC Request structure:

```text
MessagePack-RPC
[ type, msgid, method, params ]
       |    |      |       |
       |    |      |       +--> Request parameters
       |    |      +----------> Request method
       |    +-----------------> Request ID
       +----------------------> Message kind
```

A C representation can therefore keep the message type, identifier, method, and a dynamically sized parameter collection as separate fields.

The Haze `RequestParam` abstraction is responsible for representing the individual MessagePack-compatible values used by the `params` array.

---

## 10. Parameter Types in Haze

A typical Haze parameter layer can map application types to MessagePack types as follows:

| Haze parameter | MessagePack |
|---|---|
| `PARAM_NIL` | `nil` |
| `PARAM_BOOL` | `bool` |
| `PARAM_INT` | integer |
| `PARAM_FLOAT` | float32/float64 |
| `PARAM_STR` | `str` |
| `PARAM_BIN` | `bin` |
| array-like parameter | `array` |
| object/dictionary parameter | `map` |

The exact Haze enum and C representation are an implementation detail. The wire format must remain consistent between client and server.

---

## 11. Memory Ownership in C

MessagePack parsers may expose data directly from the input buffer (zero-copy / in-place access). This is useful for performance, but it creates an ownership rule:

```text
Input buffer
    |
    +----> parser returns pointer into buffer
                       |
                       +----> borrowed memory
```

A borrowed pointer must not be freed by the object that merely references it.

If Haze needs the value to outlive the input buffer, it should copy the bytes into memory owned by the Haze object:

```text
MessagePack buffer
       |
       v
borrowed pointer
       |
       v
copy once
       |
       v
Haze-owned memory
```

For strings, the parser-provided byte length should be respected. A MessagePack string's length is part of the encoded value; C code should not rely on a NUL terminator unless it explicitly creates one.

---

## 12. Binary vs. String Data

Do not confuse `str` and `bin`.

Use `str` for text:

```text
"haze.ping"
"Track 01"
```

Use `bin` for opaque bytes:

```text
[audio bytes]
[compressed data]
[file chunks]
```

This distinction matters for correctness and for interoperability between different MessagePack implementations.

---

## 13. Why MessagePack-RPC Fits Haze

MessagePack-RPC is a useful fit for a DAW-style core because it provides a compact binary protocol and a straightforward method-call abstraction without forcing the entire application state into a text format. The RPC specification also supports asynchronous/pipelined operation, and MessagePack supports binary values directly. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

A Haze client can therefore operate at a high level:

```text
client.call("track.create", ...)
client.call("mixer.set_volume", ...)
client.notify("transport.play", ...)
```

while the core remains responsible for audio processing and authoritative state.

RPC should be treated as a control/state mechanism, not as the audio sample transport itself. Audio buffers and realtime DSP should use structures designed for deterministic and low-latency processing.

---

## 14. Error Handling

An RPC implementation should distinguish at least three cases:

```text
1. Valid request + successful execution
2. Valid request + application error
3. Invalid protocol/message
```

For a valid request that fails during execution, the Response should place the application error in `error` and normally use `nil` for `result`. [MessagePack-RPC Specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md)

Protocol errors are different: malformed MessagePack, invalid RPC structure, invalid message type, invalid field type, or an impossible request should be rejected at the protocol boundary rather than silently interpreted as an application-level result.

---

## 15. Extensibility

Haze should treat the RPC method namespace and the payload schema as versioned interfaces.

Prefer explicit methods such as:

```text
track.create
track.delete
track.set_name
mixer.set_volume
transport.play
transport.stop
project.save
project.load
```

rather than exposing internal C function names directly.

Likewise, payloads should evolve in a way that allows older clients to fail gracefully or ignore fields they do not understand where the chosen schema permits it.

MessagePack maps are often useful for extensible objects because fields can be identified by name. Arrays are more compact and faster to parse when the positional schema is stable and tightly controlled.

---

## 16. Example Haze Messages

### Ping

Request:

```text
[0, 1, "haze.ping", []]
```

Response:

```text
[1, 1, nil, "pong"]
```

### Add values

Request:

```text
[0, 2, "haze.add", [10, 20]]
```

Response:

```text
[1, 2, nil, 30]
```

### Fire-and-forget notification

```text
[2, "transport.play", []]
```

### Error

```text
[1, 7, {"code": "NOT_FOUND", "message": "Track not found"}, nil]
```

---

## 17. Layered Haze Architecture

The protocol can be understood as four layers:

```text
┌─────────────────────────────────────┐
│ Haze application API                │
│ track.create / transport.play ...   │
├─────────────────────────────────────┤
│ MessagePack-RPC                     │
│ Request / Response / Notification   │
├─────────────────────────────────────┤
│ MessagePack                         │
│ nil / bool / int / float / str ...  │
├─────────────────────────────────────┤
│ Transport                           │
│ TCP / local IPC / other transport   │
└─────────────────────────────────────┘
```

This separation is valuable because each layer has a different responsibility:

- **Application API:** defines what Haze can do.
- **RPC:** defines calls, responses, IDs, and notifications.
- **MessagePack:** defines the binary representation of values.
- **Transport:** moves bytes between endpoints.

---

## 18. Summary

MessagePack-RPC is built from a small set of rules:

```text
Request      = [0, msgid, method, params]
Response     = [1, msgid, error, result]
Notification = [2, method, params]
```

MessagePack provides the underlying type system and binary encoding:

```text
nil
bool
integer
float
str
bin
array
map
ext
```

For Haze, this provides a compact and language-independent protocol between the client and the core while keeping the application API separate from the transport and serialization layers.

---

## References

- MessagePack Specification: https://github.com/msgpack/msgpack/blob/master/spec.md
- MessagePack-RPC Specification: https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md
- MessagePack project: https://msgpack.org/
