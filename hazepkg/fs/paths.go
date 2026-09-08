package fs

import (
	"os"
	"path/filepath"
)

type Paths struct {
	RootHazePath string

	ProjectsPath  string
	SamplesPath   string
	CachePath     string
	ConfigPath    string
	CommunityPath string
	LogsPath      string

	RunningFile 	string
}

var PathsInstance Paths

func (p *Paths) InitPaths() error {
	home, err := os.UserHomeDir()
	if err != nil {
		return err
	}

	p.RootHazePath = filepath.Join(home, "Haze")

	p.ProjectsPath = filepath.Join(p.RootHazePath, "projects")
	p.SamplesPath = filepath.Join(p.RootHazePath, "samples")
	p.CachePath = filepath.Join(p.RootHazePath, "cache")
	p.ConfigPath = filepath.Join(p.RootHazePath, "config")
	p.CommunityPath = filepath.Join(p.RootHazePath, "community")
	p.LogsPath = filepath.Join(p.RootHazePath, "logs")

	p.RunningFile = filepath.Join(p.RootHazePath, ".running.mp")

	return nil
}
