package msgpackrpc

import (
	"github.com/vmihailenco/msgpack/v5"
)

type Request struct {
	_msgpack struct{} `msgpack:",as_array"`
	MsgType  uint32
	MsgId    uint32
	Func     string
	Params   []any
}

func (r *Request) Init(msgId uint32, method string, params []any) error {
	r.MsgType = 0 
	r.MsgId = msgId
	r.Func = method
	if params == nil {
		r.Params = []any{}
	} else {
		r.Params = params
	}
	
	return nil
}

func (r *Request) Marshal() ([]byte, error) {
	return msgpack.Marshal(r)
}

func (r *Request) Unmarshal(byt []byte) (Request, error) {
	err := msgpack.Unmarshal(byt, r)
	if err != nil {
		return Request{}, err
	}
	return *r, nil
}
