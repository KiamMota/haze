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

        if len(value) != 4:
            raise ValueError("Invalid response")

        if value[0] != cls.TYPE:
            raise ValueError(f"Invalid response type: {value[0]}")

        return cls(
            msg_id=value[1],
            error=value[2],
            result=value[3],
        )
