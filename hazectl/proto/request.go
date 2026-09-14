package proto

import (
	"fmt"

	"github.com/vmihailenco/msgpack/v5"
)

type Request struct {
	MsgType uint8
	MsgId   uint32
	Method  string
	Params  []any
}

func (r *Request) Init(msgid uint32, method string, params []any) {
	r.MsgType = 0
	r.MsgId = msgid
	r.Method = method
	r.Params = params
}

func (r *Request) Marshal() ([]byte, error) {
	return msgpack.Marshal([]any{
		r.MsgType,
		r.MsgId,
		r.Method,
		r.Params,
	})
}

func (r *Request) Unmarshal(data []byte) error {
	var raw []any

	if err := msgpack.Unmarshal(data, &raw); err != nil {
		return err
	}

	if len(raw) != 4 {
		return fmt.Errorf("invalid request: expected 4 elements, got %d", len(raw))
	}

	r.MsgType = raw[0].(uint8)
	r.MsgId = raw[1].(uint32)
	r.Method = raw[2].(string)

	params, ok := raw[3].([]any)
	if !ok {
		return fmt.Errorf("invalid params")
	}

	r.Params = params

	return nil
}
