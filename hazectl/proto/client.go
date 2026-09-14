package proto

import (
	"encoding/binary"
	"fmt"
	"io"
	"net"
)

func SendRequest(address string, req Request) (Response, error) {
	conn, err := net.Dial("tcp", address)
	if err != nil {
		return Response{}, err
	}
	defer conn.Close()

	data, err := req.Marshal()
	if err != nil {
		return Response{}, err
	}

	var size [4]byte
	binary.BigEndian.PutUint32(size[:], uint32(len(data)))

	if _, err := conn.Write(size[:]); err != nil {
		return Response{}, err
	}

	if _, err := conn.Write(data); err != nil {
		return Response{}, err
	}

	if _, err := io.ReadFull(conn, size[:]); err != nil {
		return Response{}, err
	}

	responseSize := binary.BigEndian.Uint32(size[:])
	if responseSize > 16*1024*1024 {
		return Response{}, fmt.Errorf("response too large: %d bytes", responseSize)
	}

	responseData := make([]byte, responseSize)

	if _, err := io.ReadFull(conn, responseData); err != nil {
		return Response{}, err
	}

	var response Response

	if err := response.Unmarshal(responseData); err != nil {
		return Response{}, err
	}

	return response, nil
}
