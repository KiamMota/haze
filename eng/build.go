package main

import (
	"bufio"
	"fmt"
	"net"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/vmihailenco/msgpack/v5"
)

const (
	MsgTypeRequest      = 0
	MsgTypeResponse     = 1
	MsgTypeNotification = 2
)

// Request representa a estrutura de requisição RPC do MessagePack
type Request struct {
	MsgID  uint32
	Method string
	Params []interface{}
}

func (r *Request) Marshal() ([]byte, error) {
	// MessagePack-RPC exige um array de exatos 4 elementos
	return msgpack.Marshal([]interface{}{
		MsgTypeRequest, r.MsgID, r.Method, r.Params,
	})
}

// Metrics armazena e formata os resultados dos testes
type Metrics struct {
	Name               string
	Requests           int
	Received           int
	Errors             int
	ConnectionFailures int
	Latencies          []time.Duration
	Elapsed            time.Duration
}

func (m *Metrics) AddLatency(d time.Duration) {
	m.Latencies = append(m.Latencies, d)
}

func (m *Metrics) Print() {
	rps := 0.0
	if m.Elapsed.Seconds() > 0 {
		rps = float64(m.Received) / m.Elapsed.Seconds()
	}

	fmt.Println("\n" + strings.Repeat("=", 68))
	fmt.Printf(" %s\n", m.Name)
	fmt.Println(strings.Repeat("=", 68))
	fmt.Printf(" Sent requests       : %s\n", formatInt(m.Requests))
	fmt.Printf(" Received responses  : %s\n", formatInt(m.Received))
	fmt.Printf(" RPC/protocol errors : %s\n", formatInt(m.Errors))
	fmt.Printf(" Connection failures : %s\n", formatInt(m.ConnectionFailures))
	fmt.Printf(" Total time          : %.4f s\n", m.Elapsed.Seconds())
	fmt.Printf(" Throughput          : %s req/s\n", formatFloat(rps))

	if len(m.Latencies) > 0 {
		sort.Slice(m.Latencies, func(i, j int) bool {
			return m.Latencies[i] < m.Latencies[j]
		})

		percentile := func(p float64) float64 {
			idx := int(float64(len(m.Latencies)) * p)
			if idx >= len(m.Latencies) {
				idx = len(m.Latencies) - 1
			}
			return float64(m.Latencies[idx].Microseconds()) / 1000.0
		}

		var sum time.Duration
		for _, d := range m.Latencies {
			sum += d
		}
		avg := (float64(sum.Microseconds()) / 1000.0) / float64(len(m.Latencies))

		fmt.Printf(" Average latency     : %.3f ms\n", avg)
		fmt.Printf(" Minimum latency     : %.3f ms\n", float64(m.Latencies[0].Microseconds())/1000.0)
		fmt.Printf(" Latency p50         : %.3f ms\n", percentile(0.50))
		fmt.Printf(" Latency p95         : %.3f ms\n", percentile(0.95))
		fmt.Printf(" Latency p99         : %.3f ms\n", percentile(0.99))
		fmt.Printf(" Maximum latency     : %.3f ms\n", float64(m.Latencies[len(m.Latencies)-1].Microseconds())/1000.0)
	}
	fmt.Println(strings.Repeat("=", 68))
}

// testSimpleConnection implementa a lógica dos testes 1 e 2
func testSimpleConnection(host string, port int, totalRequests int, testName string) *Metrics {
	m := &Metrics{Name: testName, Requests: totalRequests}
	start := time.Now()
	addr := fmt.Sprintf("%s:%d", host, port)

	for i := 0; i < totalRequests; i++ {
		conn, err := net.Dial("tcp", addr)
		if err != nil {
			m.ConnectionFailures++
			continue
		}

		req := Request{MsgID: uint32(i), Method: "ping", Params: []interface{}{}}
		data, _ := req.Marshal()

		sent := time.Now()
		conn.Write(data)

		var respArr []interface{}
		dec := msgpack.NewDecoder(conn)
		err = dec.Decode(&respArr)
		received := time.Now()

		if err == nil && len(respArr) == 4 {
			m.Received++
			m.AddLatency(received.Sub(sent))
			if respArr[2] != nil { // verifica se tem erro na resposta
				m.Errors++
			}
		} else {
			m.Errors++
		}
		conn.Close()
	}

	m.Elapsed = time.Since(start)
	return m
}

// testPersistentSocket implementa a lógica de multiplexação em apenas um Socket
func testPersistentSocket(host string, port int, totalRequests int) *Metrics {
	m := &Metrics{Name: "TEST 3 — PERSISTENT SOCKET + MULTIPLE REQUESTS", Requests: totalRequests}
	start := time.Now()
	addr := fmt.Sprintf("%s:%d", host, port)

	conn, err := net.Dial("tcp", addr)
	if err != nil {
		m.ConnectionFailures++
		m.Elapsed = time.Since(start)
		return m
	}
	defer conn.Close()

	var mu sync.Mutex
	pending := make(map[uint32]time.Time)
	var wg sync.WaitGroup

	wg.Add(1)
	go func() {
		defer wg.Done()
		dec := msgpack.NewDecoder(conn)

		for m.Received < totalRequests {
			var respArr []interface{}
			if err := dec.Decode(&respArr); err != nil {
				break
			}

			if len(respArr) == 4 {
				msgID := toInt(respArr[1])

				mu.Lock()
				sent, ok := pending[msgID]
				if ok {
					delete(pending, msgID)
					m.AddLatency(time.Since(sent))
				}
				mu.Unlock()

				if respArr[2] != nil {
					m.Errors++
				}
				m.Received++
			}
		}
	}()

	writer := bufio.NewWriterSize(conn, 65536)
	for i := 0; i < totalRequests; i++ {
		req := Request{MsgID: uint32(i), Method: "ping", Params: []interface{}{}}
		data, _ := req.Marshal()

		mu.Lock()
		pending[uint32(i)] = time.Now()
		mu.Unlock()

		writer.Write(data)

		// Simula o await drain()
		if i%1000 == 0 {
			writer.Flush()
		}
	}
	writer.Flush()

	// Implementação do await asyncio.wait_for()
	c := make(chan struct{})
	go func() {
		wg.Wait()
		close(c)
	}()

	select {
	case <-c:
		// Finalizado com sucesso
	case <-time.After(30 * time.Second):
		// Timeout - O conn.Close() no defer da main irá abortar a goroutine
	}

	m.Elapsed = time.Since(start)
	return m
}

func main() {
	host := "127.0.0.1"
	port := 7192
	totalRequests := 1000

	fmt.Println(strings.Repeat("=", 68))
	fmt.Println(" HAZE RPC BENCHMARK (GO)")
	fmt.Println(strings.Repeat("=", 68))
	fmt.Printf(" Target      : %s:%d\n", host, port)
	fmt.Printf(" Requests    : %s\n", formatInt(totalRequests))
	fmt.Println(strings.Repeat("=", 68))

	metrics1 := testSimpleConnection(host, port, totalRequests, "TEST 1 — NEW CONNECTION PER REQUEST")
	metrics1.Print()

	metrics2 := testSimpleConnection(host, port, totalRequests, "TEST 2 — OPEN SOCKET + 1 REQUEST")
	metrics2.Print()

	metrics3 := testPersistentSocket(host, port, totalRequests)
	metrics3.Print()
}

// ----- Funções utilitárias -----

// toInt converte a interface decodificada do MessagePack confiavelmente em uint32
func toInt(v interface{}) uint32 {
	switch i := v.(type) {
	case int: return uint32(i)
	case int8: return uint32(i)
	case int16: return uint32(i)
	case int32: return uint32(i)
	case int64: return uint32(i)
	case uint: return uint32(i)
	case uint8: return uint32(i)
	case uint16: return uint32(i)
	case uint32: return uint32(i)
	case uint64: return uint32(i)
	}
	return 0
}

// formatInt adiciona separadores de milhar a um inteiro (ex: 1,000)
func formatInt(n int) string {
	in := strconv.Itoa(n)
	var out []byte
	for i, c := range in {
		if i > 0 && (len(in)-i)%3 == 0 {
			out = append(out, ',')
		}
		out = append(out, byte(c))
	}
	return string(out)
}

// formatFloat formata os floats com virgula (para RPS)
func formatFloat(f float64) string {
	parts := strings.Split(fmt.Sprintf("%.2f", f), ".")
	in := parts[0]
	var out []byte
	for i, c := range in {
		if i > 0 && (len(in)-i)%3 == 0 {
			out = append(out, ',')
		}
		out = append(out, byte(c))
	}
	return string(out) + "." + parts[1]
}
