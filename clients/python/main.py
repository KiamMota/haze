import asyncio
import msgpack
import time
import statistics
from collections import Counter

from proto import Request, Response


HOST = "127.0.0.1"
PORT = 7193

TOTAL_REQUESTS = 10_000
CONCURRENCY = 1000  # Número de conexões simultâneas (sockets abertos)

METHOD = "session/create"
PARAMS = ["benchmark"]


async def worker(
    worker_id: int,
    requests_per_worker: int,
    latencies: list,
    failures: list,
    conn_semaphore: asyncio.Semaphore,
):
    # 1. Abre a conexão usando o semáforo para evitar o WinError 52
    try:
        async with conn_semaphore:
            reader, writer = await asyncio.open_connection(HOST, PORT)
    except Exception as e:
        for i in range(requests_per_worker):
            failures.append((worker_id, type(e).__name__, f"Connect error: {str(e)}"))
        return

    unpacker = msgpack.Unpacker(raw=False)

    try:
        # 2. Reutiliza a conexão para múltiplas requisições (FORA do semáforo, rodando tudo em paralelo)
        for i in range(requests_per_worker):
            msg_id = (worker_id * 1_000_000) + i
            start = time.perf_counter_ns()

            try:
                # --- ENVIA ---
                req = Request(
                    msg_id=msg_id,
                    method=METHOD,
                    params=PARAMS,
                )
                writer.write(req.marshal())
                await writer.drain()

                # --- RECEBE ---
                response_received = False
                while not response_received:
                    # Verifica se já temos respostas no buffer do msgpack
                    for value in unpacker:
                        response = Response.unmarshal(
                            msgpack.packb(value, use_bin_type=True)
                        )

                        if response.msg_id == msg_id:
                            elapsed = time.perf_counter_ns() - start
                            latencies.append(elapsed / 1_000_000)
                            response_received = True
                            break # Achou a resposta correta

                    if response_received:
                        break

                    # Se não achou, lê mais dados do socket
                    data = await reader.read(4096)
                    if not data:
                        raise ConnectionError("Server closed connection prematurely")
                    
                    unpacker.feed(data)

            except Exception as e:
                failures.append((msg_id, type(e).__name__, str(e)))
                # Se a conexão quebrou no meio, aborta o resto dos requests desse worker
                raise 

    except Exception:
        pass # Os erros já foram registrados na lista de failures
    finally:
        # 3. Fecha a conexão apenas quando o lote terminar
        writer.close()
        try:
            await writer.wait_closed()
        except Exception:
            pass


async def benchmark():
    print("========================================")
    print("            HAZE BENCHMARK")
    print("========================================")
    print(f"Target       : {HOST}:{PORT}")
    print(f"Requests     : {TOTAL_REQUESTS:,}")
    print(f"Connections  : {CONCURRENCY:,}")
    
    requests_per_conn = TOTAL_REQUESTS // CONCURRENCY
    
    print(f"Reqs/Conn    : {requests_per_conn:,}")
    print("Model        : TCP Keep-Alive (Persistent)")
    print("========================================")
    print()

    latencies = []
    failures = []

    # Semáforo para limitar a taxa de CRIAÇÃO de conexões (handshake)
    conn_semaphore = asyncio.Semaphore(50)

    start = time.perf_counter()

    # Cria as tarefas (cada tarefa é uma conexão persistente)
    tasks = [
        asyncio.create_task(
            worker(
                worker_id=i,
                requests_per_worker=requests_per_conn,
                latencies=latencies,
                failures=failures,
                conn_semaphore=conn_semaphore,
            )
        )
        for i in range(CONCURRENCY)
    ]

    await asyncio.gather(*tasks)

    elapsed = time.perf_counter() - start

    print("========================================")
    print("              RESULTS")
    print("========================================")

    print(f"Successful   : {len(latencies):,}")
    print(f"Failed       : {len(failures):,}")
    print(f"Total time   : {elapsed:.3f} s")

    if latencies:
        throughput = len(latencies) / elapsed
        latencies.sort()

        def percentile(value):
            index = int(len(latencies) * value)
            index = min(index, len(latencies) - 1)
            return latencies[index]

        print(f"Throughput   : {throughput:,.0f} req/s")
        print()
        print(f"Min          : {min(latencies):.3f} ms")
        print(f"Mean         : {statistics.mean(latencies):.3f} ms")
        print(f"p50          : {percentile(0.50):.3f} ms")
        print(f"p90          : {percentile(0.90):.3f} ms")
        print(f"p95          : {percentile(0.95):.3f} ms")
        print(f"p99          : {percentile(0.99):.3f} ms")
        print(f"p99.9        : {percentile(0.999):.3f} ms")
        print(f"Max          : {max(latencies):.3f} ms")

    if failures:
        print()
        print("Failure types:")

        failure_types = Counter(
            (error_type, message)
            for _, error_type, message in failures
        )

        for (error_type, message), count in failure_types.most_common():
            print(f"  {count:,}x {error_type}: {message}")

        print()
        print("First failures:")
        for msg_id, error_type, message in failures[:10]:
            print(f"  #{msg_id}: {error_type}: {message}")

    print("========================================")


if __name__ == "__main__":
    asyncio.run(benchmark())
