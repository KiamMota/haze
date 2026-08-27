from proto import Request, Response
import asyncio
import msgpack


class AudioClient:
    def __init__(self, host="127.0.0.1", port=7192):
        self.host = host
        self.port = port
        self.msg_id = 0
        self.reader = None
        self.writer = None

    async def connect(self):
        self.reader, self.writer = await asyncio.open_connection(
            self.host,
            self.port
        )

    async def call(self, method: str, params: list):
        self.msg_id += 1

        request = Request(
            msg_id=self.msg_id,
            method=method,
            params=params
        )

        self.writer.write(request.marshal())
        await self.writer.drain()

        unpacker = msgpack.Unpacker(raw=False)

        while True:
            data = await self.reader.read(4096)

            if not data:
                raise ConnectionError("Server closed the connection")

            unpacker.feed(data)

            for value in unpacker:
                response = Response.unmarshal(
                    msgpack.packb(value, use_bin_type=True)
                )

                if response.msg_id == self.msg_id:
                    return response


async def main():
    client = AudioClient()

    await client.connect()

    response = await client.call(
        "session/create",
        ["my_biggest_session"]
    )

    print("Message ID:", response.msg_id)
    print("Error:", response.error)
    print("Result:", response.result)


if __name__ == "__main__":
    asyncio.run(main())
