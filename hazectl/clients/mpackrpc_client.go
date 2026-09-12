package clients

import (
	"fmt"
	"hazectl/msgpackrpc"
	"net"
	"sync"
	"sync/atomic"

	"github.com/vmihailenco/msgpack/v5"
)

type Client struct {
	conn    net.Conn
	encoder *msgpack.Encoder
	decoder *msgpack.Decoder

	seq     uint32                               // Contador seguro para gerar MsgIDs únicos
	mu      sync.Mutex                           // Protege o mapa de requisições pendentes
	pending map[uint32]chan *msgpackrpc.Response // Guarda os canais esperando a RESPOSTA
}

// NewClient cria o cliente e já inicia a rotina de leitura em background
func NewClient(conn net.Conn) *Client {
	c := &Client{
		conn:    conn,
		encoder: msgpack.NewEncoder(conn),
		decoder: msgpack.NewDecoder(conn),
		pending: make(map[uint32]chan *msgpackrpc.Response),
	}

	// Inicia o loop de escuta em uma goroutine separada
	go c.readLoop()

	return c
}

// Call faz a chamada RPC e bloqueia até o servidor responder
// Call faz a chamada RPC e bloqueia até o servidor responder
func (c *Client) Call(method string, params ...any) (any, error) {
	// 1. Gera um MsgID único de forma atômica (thread-safe)
	msgID := atomic.AddUint32(&c.seq, 1)

	// 2. Cria um canal para receber a resposta desta chamada específica
	respChan := make(chan *msgpackrpc.Response, 1)

	// 3. Registra o canal no mapa de pendentes
	c.mu.Lock()
	c.pending[msgID] = respChan
	c.mu.Unlock()

	// PROTEÇÃO CONTRA NIL DO GO:
	// Garante que 'params' seja sempre um array, mesmo que vazio,
	// senão o MessagePack do C recusa a requisição!
	if params == nil {
		params = []any{}
	}

	// 4. Monta e envia a requisição usando o seu struct Request
	// Isso OBRIGA o pacote msgpack a serializar como um Array de 4 posições.
	reqPayload := []any{
		0,      // MsgType
		msgID,  // MsgId
		method, // Func
		params, // Params
	}

	// NOVIDADE: Transforma TUDO em bytes na memória primeiro!
	payloadBytes, err := msgpack.Marshal(reqPayload)
	if err != nil {
		c.removePending(msgID)
		return nil, fmt.Errorf("erro ao converter requisição: %w", err)
	}

	// Dispara o pacote inteiro em um único TCP Write!
	if _, err := c.conn.Write(payloadBytes); err != nil {
		c.removePending(msgID)
		return nil, fmt.Errorf("erro ao enviar requisição pela rede: %w", err)
	}
	if err := c.encoder.Encode(reqPayload); err != nil {
		c.removePending(msgID)
		return nil, fmt.Errorf("erro ao enviar requisição: %w", err)
	}

	if err := c.encoder.Encode(reqPayload); err != nil {
		c.removePending(msgID)
		return nil, fmt.Errorf("erro ao enviar requisição: %w", err)
	}

	// 5. Espera a resposta chegar do readLoop de forma segura!
	resp, ok := <-respChan
	if !ok {
		return nil, fmt.Errorf("conexão encerrada abruptamente pelo servidor")
	}

	if resp.Error != nil {
		return nil, fmt.Errorf("erro no servidor: %v", resp.Error)
	}

	return resp.Result, nil
}

// readLoop fica lendo a conexão infinitamente esperando respostas
func (c *Client) readLoop() {
	for {
		var resp msgpackrpc.Response // Usando o seu struct Response

		err := c.decoder.Decode(&resp)
		if err != nil {
			c.mu.Lock()
			for _, ch := range c.pending {
				close(ch)
			}
			c.pending = make(map[uint32]chan *msgpackrpc.Response)
			c.mu.Unlock()
			return // Sai do loop
		}

		if resp.MsgType != 1 {
			continue
		}

		c.mu.Lock()
		ch, ok := c.pending[resp.MsgId]
		if ok {
			delete(c.pending, resp.MsgId)
		}
		c.mu.Unlock()

		if ok {
			ch <- &resp
		}
	}
}

func (c *Client) removePending(msgID uint32) {
	c.mu.Lock()
	defer c.mu.Unlock()
	delete(c.pending, msgID)
}

func (c *Client) Close() error {
	return c.conn.Close()
}
