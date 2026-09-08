package cmd

import (
	"fmt"
	"hazepkg/fs"
	"strings"
)

func ProcessStatus(instances []fs.InstanceState) string {
	var b strings.Builder

	b.WriteString("PID     SESSION              PORT    STATUS\n")

	for _, instance := range instances {
		fmt.Fprintf(
			&b,
			"%-7d %-20s %-7d %s\n",
			instance.PID,
			instance.Session,
			instance.Port,
			instance.Status,
		)
	}

	return b.String()
}
