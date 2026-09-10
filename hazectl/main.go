package main

import (
	"hazectl/cli"
	"hazectl/hazefs"
	"os"
)

func main() {
	hazefs.PathsInstance.InitPaths()

	args := os.Args[1:]

	if len(args) == 0 {
		cli.Help()
		return
	}

	switch args[0] {
	case "-v", "--version":
		cli.Version()
		return

	case "-h", "--help":
		cli.Help()
		return
	}

	cli.TraitArgs(args)
}
