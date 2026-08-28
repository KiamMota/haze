import msgpack


class Response:
    TYPE = 1

    def __init__(
        self,
        msg_id: int,
        error=None,
        result=None
    ):
        self.msg_id = msg_id
        self.error = error
        self.result = result

    def marshal(self) -> bytes:
        return msgpack.packb(
            [
                self.TYPE,
                self.msg_id,
                self.error,
                self.result
            ],
            use_bin_type=False
        )

    @classmethod
    def unmarshal(cls, data: bytes) -> "Response":
        value = msgpack.unpackb(data, raw=False)
        print(repr(value))

        if len(value) != 4:
            raise ValueError("Invalid MessagePack-RPC response")

        msg_type, msg_id, error, result = value

        if msg_type != cls.TYPE:
            raise ValueError("Invalid MessagePack-RPC response type")

        return cls(
            msg_id=msg_id,
            error=error,
            result=result
        )
