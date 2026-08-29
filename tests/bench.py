import asyncio
import time
import msgpack
from dataclasses import dataclass, field
from typing import Any


MSGTYPE_REQUEST = 0
MSGTYPE_RESPONSE = 1
MSGTYPE_NOTIFICATION = 2


@dataclass
class Request:
    msg_id: int
    method: str
    params: list

    def marshal(self) -> bytes:
        return msgpack.packb(
            [MSGTYPE_REQUEST, self.msg_id, self.method, self.params],
            use_bin_type=True,
        )


@dataclass
class Response:
    msg_id: int
    error: Any
    result: Any

    @classmethod
    def from_value(cls, value) -> "Response":
        if not isinstance(value, list) or len(value) != 4:
            raise ValueError("Invalid MessagePack-RPC response payload.")

        msg_type, msg_id, error, result = value

        if msg_type != MSGTYPE_RESPONSE:
            raise ValueError(f"Unexpected message type: {msg_type}")

        return cls(msg_id=msg_id, error=error, result=result)


@dataclass
class Metrics:
    name: str
    requests: int = 0
    received: int = 0
    errors: int = 0
    connection_failures: int = 0
    latencies: list[float] = field(default_factory=list)
    elapsed: float = 0.0

    def add_latency(self, value: float):
        self.latencies.append(value)

    def print(self):
        rps = self.received / self.elapsed if self.elapsed > 0 else 0.0

        print("\n" + "=" * 68)
        print(f" {self.name}")
        print("=" * 68)
        print(f" Sent requests       : {self.requests:,}")
        print(f" Received responses  : {self.received:,}")
        print(f" RPC/protocol errors : {self.errors:,}")
        print(f" Connection failures : {self.connection_failures:,}")
        print(f" Total time          : {self.elapsed:.4f} s")
        print(f" Throughput          : {rps:,.2f} req/s")

        if self.latencies:
            lat = sorted(self.latencies)

            def percentile(p: float) -> float:
                idx = min(int(len(lat) * p), len(lat) - 1)
                return lat[idx] * 1000.0

            avg = (sum(lat) / len(lat)) * 1000.0
            print(f" Average latency     : {avg:.3f} ms")
            print(f" Minimum latency     : {lat[0] * 1000.0:.3f} ms")
            print(f" Latency p50         : {percentile(0.50):.3f} ms")
            print(f" Latency p95         : {percentile(0.95):.3f} ms")
            print(f" Latency p99         : {percentile(0.99):.3f} ms")
            print(f" Maximum latency     : {lat[-1] * 1000.0:.3f} ms")

        print("=" * 68)


async def recv_one(unpacker, reader) -> Response:
    while True:
        data = await reader.read(65536)
        if not data:
            raise ConnectionError("Server closed the connection.")

        unpacker.feed(data)

        for value in unpacker:
            return Response.from_value(value)


async def test_new_connection_per_request(
    host: str,
    port: int,
    total_requests: int,
) -> Metrics:
    metrics = Metrics(
        name="TEST 1 — NEW CONNECTION PER REQUEST",
        requests=total_requests,
    )

    start = time.perf_counter()

    for i in range(total_requests):
        try:
            reader, writer = await asyncio.open_connection(host, port)

            unpacker = msgpack.Unpacker(raw=False)
            request = Request(i, "ping", [])

            sent = time.perf_counter()
            writer.write(request.marshal())
            await writer.drain()

            response = await recv_one(unpacker, reader)
            received = time.perf_counter()

            metrics.received += 1
            metrics.add_latency(received - sent)

            if response.error:
                metrics.errors += 1

            writer.close()
            await writer.wait_closed()

        except Exception:
            metrics.connection_failures += 1

    metrics.elapsed = time.perf_counter() - start
    return metrics


async def test_open_connection_one_request(
    host: str,
    port: int,
    total_requests: int,
) -> Metrics:
    metrics = Metrics(
        name="TEST 2 — OPEN SOCKET + 1 REQUEST",
        requests=total_requests,
    )

    start = time.perf_counter()

    for i in range(total_requests):
        try:
            reader, writer = await asyncio.open_connection(host, port)

            unpacker = msgpack.Unpacker(raw=False)
            request = Request(i, "ping", [])

            sent = time.perf_counter()
            writer.write(request.marshal())
            await writer.drain()

            response = await recv_one(unpacker, reader)
            received = time.perf_counter()

            metrics.received += 1
            metrics.add_latency(received - sent)

            if response.error:
                metrics.errors += 1

            writer.close()
            await writer.wait_closed()

        except Exception:
            metrics.connection_failures += 1

    metrics.elapsed = time.perf_counter() - start
    return metrics


async def test_persistent_socket(
    host: str,
    port: int,
    total_requests: int,
) -> Metrics:
    metrics = Metrics(
        name="TEST 3 — PERSISTENT SOCKET + MULTIPLE REQUESTS",
        requests=total_requests,
    )

    start = time.perf_counter()

    try:
        reader, writer = await asyncio.open_connection(host, port)
        unpacker = msgpack.Unpacker(raw=False)

        pending: dict[int, float] = {}

        async def receiver():
            while metrics.received < total_requests:
                data = await reader.read(65536)

                if not data:
                    raise ConnectionError("Server closed the connection.")

                unpacker.feed(data)

                for value in unpacker:
                    response = Response.from_value(value)

                    sent = pending.pop(response.msg_id, None)
                    if sent is not None:
                        metrics.add_latency(time.perf_counter() - sent)

                    if response.error:
                        metrics.errors += 1

                    metrics.received += 1

        receiver_task = asyncio.create_task(receiver())

        for i in range(total_requests):
            pending[i] = time.perf_counter()

            writer.write(Request(i, "ping", []).marshal())

            if i % 1000 == 0:
                await writer.drain()

        await writer.drain()

        try:
            await asyncio.wait_for(receiver_task, timeout=30.0)
        except asyncio.TimeoutError:
            receiver_task.cancel()
            try:
                await receiver_task
            except asyncio.CancelledError:
                pass

        writer.close()
        await writer.wait_closed()

    except Exception:
        metrics.connection_failures += 1

    metrics.elapsed = time.perf_counter() - start
    return metrics


async def main():
    HOST = "127.0.0.1"
    PORT = 7192
    TOTAL_REQUESTS = 1000

    print("=" * 68)
    print(" HAZE RPC BENCHMARK")
    print("=" * 68)
    print(f" Target      : {HOST}:{PORT}")
    print(f" Requests    : {TOTAL_REQUESTS:,}")
    print("=" * 68)

    tests = [
        test_new_connection_per_request,
        test_open_connection_one_request,
        test_persistent_socket,
    ]

    for test in tests:
        metrics = await test(HOST, PORT, TOTAL_REQUESTS)
        metrics.print()


if __name__ == "__main__":
    asyncio.run(main())
