import asyncio
import msgpack

from proto import Request, Response


HOST = "127.0.0.1"
PORT = 7192


RPCS = [
    ("ping", []),
    ("session/create", ["benchmark"]),
    ("session/get_name", []),
    ("session/get_worktime", []),
]


async def call_rpc(reader, writer, unpacker, msg_id, method, params):
    print(f"[RPC] → {method}")

    request = Request(
        msg_id=msg_id,
        method=method,
        params=params,
    )

    writer.write(request.marshal())
    await writer.drain()

    while True:
        data = await reader.read(4096)

        if not data:
            raise ConnectionError("Server closed connection")

        unpacker.feed(data)

        for value in unpacker:
            response = Response.unmarshal(
                msgpack.packb(value, use_bin_type=True)
            )

            if response.msg_id == msg_id:
                print(
                    f"[RPC] ← {method} "
                    f"(msg_id={response.msg_id})"
                )
                print(f"       error  : {response.error}")
                print(f"       result : {response.result}")
                print()

                return response


async def main():
    print(f"Connecting to {HOST}:{PORT}...")

    reader, writer = await asyncio.open_connection(HOST, PORT)
    unpacker = msgpack.Unpacker(raw=False)

    print("Connected.\n")

    try:
        for msg_id, (method, params) in enumerate(RPCS):
            await call_rpc(
                reader,
                writer,
                unpacker,
                msg_id,
                method,
                params,
            )

    except Exception as e:
        print(f"[ERROR] {type(e).__name__}: {e}")

    finally:
        print("Closing connection...")
        writer.close()

        try:
            await writer.wait_closed()
        except Exception:
            pass

        print("Disconnected.")


if __name__ == "__main__":
    asyncio.run(main())
