package cmd

<<<<<<< HEAD
import (
	"fmt"
	"net"
	"hazectl/clients"
	"hazectl/msgpackrpc"
)

func Request(address string, port int, req *msgpackrpc.Request) (msgpackrpc.Response, error) {
	target := fmt.Sprintf("%s:%d", address, port)

	conn, err := net.Dial("tcp", target)
	if err != nil {
		return msgpackrpc.Response{}, fmt.Errorf("fail to connect to server %s: %w", target, err)
	}

	client := clients.NewClient(conn)
	defer client.Close()

	result, err := client.Call(req.Func, req.Params...)
	if err != nil {
		return msgpackrpc.Response{}, fmt.Errorf("fail to execute '%s': %w", req.Func, err)
	}

	var resp msgpackrpc.Response	
	resp.Init(req.MsgId, nil, result)
	return resp, nil
=======
import "hazectl/proto"

func Request(port int, funcName string, params []any) {
	var req proto.Request
	req.Init(1, funcName, params)

>>>>>>> 84ea09b (add: more functions)
}
