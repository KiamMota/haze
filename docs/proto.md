# Haze Protocol

Haze uses [MessagePack-RPC](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md) over a byte stream (TCP or local IPC).

### Message types

| Type         | Structure                          |
|--------------|------------------------------------|
| Request      | `[0, msgid, method, params]`       |
| Response     | `[1, msgid, error, result]`        |
| Notification | `[2, method, params]`              |

- `msgid` is a 32-bit unsigned integer used for correlation.
- `method` is a string (e.g. `track.create`, `transport.play`).
- `params` is an array of MessagePack values.
- On success `error` is `nil`. On failure `result` is `nil`.

Responses may arrive out of order (pipelining is supported).

### Conventions

- Method names use `domain.action` form (`track.create`, `mixer.set_volume`, `project.save`).
- Prefer maps for extensible objects, arrays when the schema is fixed and performance-critical.
- Binary data (audio chunks, files, plugin state) uses MessagePack `bin`, never `str`.
- RPC is for control and state only. Real-time audio does **not** go through this channel.

### Error model

Application errors are returned in the `error` field (typically a map with `code` and `message`).  
Protocol-level errors (malformed MessagePack, invalid message type, etc.) are rejected at the transport boundary.

Full method schemas live in `docs/api/`.
