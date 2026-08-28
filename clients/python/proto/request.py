import msgpack


class Request:
    TYPE = 0

    def __init__(self, msg_id: int, method: str, params: list):
        self.msg_id = msg_id
        self.method = method
        self.params = params

    def marshal(self) -> bytes:
        return msgpack.packb(
            [
                self.TYPE,
                self.msg_id,
                self.method,
                self.params
            ],
            use_bin_type=False
        )

    @classmethod
    def unmarshal(cls, data: bytes) -> "Request":
        value = msgpack.unpackb(data, raw=False)

        if len(value) != 4:
            raise ValueError("Invalid MessagePack-RPC request")

        msg_type, msg_id, method, params = value

        if msg_type != cls.TYPE:
            raise ValueError("Invalid MessagePack-RPC request type")

        return cls(
            msg_id=msg_id,
            method=method,
            params=params
        )
