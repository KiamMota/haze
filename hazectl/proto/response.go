package proto

import (
	"fmt"

	"github.com/vmihailenco/msgpack/v5"
)

type Response struct {
	MsgType uint8
	MsgId   uint32
	Error   any
	Result  any
}

func (r *Response) Init(msgid uint32, err any, result any) {
	r.MsgType = 1
	r.MsgId = msgid
	r.Error = err
	r.Result = result
}

func (r *Response) Marshal() ([]byte, error) {
	return msgpack.Marshal([]any{
		r.MsgType,
		r.MsgId,
		r.Error,
		r.Result,
	})
}

func (r *Response) Unmarshal(data []byte) error {
	var raw []any

	if err := msgpack.Unmarshal(data, &raw); err != nil {
		return err
	}

	if len(raw) != 4 {
		return fmt.Errorf("invalid response: expected 4 elements, got %d", len(raw))
	}

	msgType, ok := raw[0].(uint8)
	if !ok {
		return fmt.Errorf("invalid message type")
	}

	msgId, ok := raw[1].(uint32)
	if !ok {
		return fmt.Errorf("invalid message id")
	}

	r.MsgType = msgType
	r.MsgId = msgId
	r.Error = raw[2]
	r.Result = raw[3]

	return nil
}
