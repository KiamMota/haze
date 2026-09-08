package cli

import "fmt"

func Help() {
	fmt.Println()
	Print("Haze (awesome) Package Manager")
	fmt.Println()
	fmt.Println("Usage:")
	fmt.Println("  hazep <command> [package]")
	fmt.Println()
	fmt.Println("Commands:")
	fmt.Println("  install    Install a package")
	fmt.Println("  list       List installed packages")
	fmt.Println("  update     Update a package or all packages")
	fmt.Println("  remove     Remove a package")
	fmt.Println("  search     Search the Haze registry")
	fmt.Println("  info       Show package information")
	fmt.Println("  upgrade    Update Haze itself")
	fmt.Println("  doctor     Check the Haze installation")
	fmt.Println("  clean      Remove unused package data")
	fmt.Println("  help       Show this help message")
	fmt.Println()
}

func Version() {
	Print("Haze Package Manager: ", "1.0.0")
}
