package cli

import "fmt"

func Print(msg ...string) {
	fmt.Print(bold + "==> " + reset)
	for i, m := range msg {
		if i > 0 {
			fmt.Print(" ")
		}
		fmt.Print(m)
	}
	fmt.Println()
}

func PrintErr(msg ...string) {
	fmt.Println("Error: ")
	for i, m := range msg {
		if i > 0 {
			fmt.Print(" ")
		}
		fmt.Print(m)
	}
	fmt.Println()
}
