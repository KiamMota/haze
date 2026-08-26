import asyncio
import msgpack

class AudioClient:
    def __init__(self, host='127.0.0.1', port=7193):
        self.host = host
        self.port = port
        self.msg_id = 0
        self.writer = None
        self.reader = None
        # O Unpacker resolve o erro de ExtraData isolando apenas um objeto MsgPack por vez
        self.unpacker = msgpack.Unpacker()

    async def connect(self):
        self.reader, self.writer = await asyncio.open_connection(self.host, self.port)

    async def call(self, method: str, params: list):
        self.msg_id += 1
        # Formato MsgPack-RPC Request: [type (0 = Request), msgid, method, params]
        request = [0, self.msg_id, method, params]
        
        # use_bin_type=False força o Python a serializar strings como STR, não como BIN (Type 4)
        payload = msgpack.packb(request, use_bin_type=False)
        
        self.writer.write(payload)
        await self.writer.drain()

        # Lê a resposta do stream TCP
        data = await self.reader.read(4096)
        if not data:
            raise ConnectionError("Servidor fechou a conexão antes de responder.")

        # Alimenta o unpacker com os bytes recebidos
        self.unpacker.feed(data)
        
        # Extrai apenas a primeira mensagem válida do buffer, ignorando sobras ou lixos de rede
        response = next(self.unpacker)
        
        # Formato Response: [type (1 = Response), msgid, error, result]
        return response

async def main():
    client = AudioClient()
    await client.connect()
    
    # Exemplo de chamadas RPC
    res = await client.call("play_sound", ["bgm_track1.wav", 1.0])
    print("Resposta do servidor:", res)

if __name__ == "__main__":
    asyncio.run(main())
