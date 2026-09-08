package fs

import (
	"os"

	"github.com/vmihailenco/msgpack/v5"
)

type InstanceState struct {
	PID     int    `msgpack:"pid"`
	Session string `msgpack:"session"`
	Port    int    `msgpack:"port"`
	Status  string `msgpack:"status"`
}

func (p* Paths) ReadRunningFile() ([]InstanceState, error) {
	data, err := os.ReadFile(p.RunningFile)
	if err != nil {
		return nil, err
	}

	var running []InstanceState

	if err := msgpack.Unmarshal(data, &running); err != nil {
		return nil, err
	}

	return running, nil
}
