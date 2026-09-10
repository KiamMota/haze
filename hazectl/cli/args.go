package cli

import (
	"fmt"
	"hazectl/cmd"
	"hazectl/hazefs"
)

func TraitArgs(args []string) {
	if len(args) == 0 {
		return
	}
	args = args[:1]

	switch args[0] {
	case "ps":
		print("\n")
		instances, err := hazefs.PathsInstance.ReadRunningFile()
		if err != nil {
			fmt.Println(err)
			return
		}

		fmt.Print(cmd.ProcessStatus(instances))

	case "install":
		// hazepkg install <package>

	case "remove":
		// hazepkg remove <package>

	case "update":
		// hazepkg update <package>

	case "search":
		// hazepkg search <package>

	case "info":
		// hazepkg info <package>

	case "upgrade":
		// hazepkg upgrade

	case "doctor":
		// hazepkg doctor

	case "clean":
		// hazepkg clean

	case "help":
		// hazepkg help

	default:
		// comando desconhecido
	}
}
