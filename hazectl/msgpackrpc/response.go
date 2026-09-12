package msgpackrpc

import (
	"fmt"

	"github.com/vmihailenco/msgpack/v5"
)

type Response struct {
	_msgpack struct{} `msgpack:",as_array"`
	MsgType  uint32
	MsgId    uint32
	Error    any
	Result   any
}

func (r *Response) Init(msgId uint32, err any, result any) error {
	r.MsgType = 1
	r.MsgId = msgId
	r.Error = err
	r.Result = result

	return nil
}

func (r *Response) Marshal() ([]byte, error) {
	return msgpack.Marshal(r)
}

func (r *Response) Unmarshal(byt []byte) (Response, error) {
	err := msgpack.Unmarshal(byt, r)
	if err != nil {
		return Response{}, err
	}
	return *r, nil
}

func (r *Response) ToString() string {
	return fmt.Sprintf("[%d, %d, %v, %v]", r.MsgType, r.MsgId, r.Error, r.Result)

}
