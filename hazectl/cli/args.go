package cli

import (
	"fmt"
	"hazectl/cmd"
	"hazectl/hazefs"
	"hazectl/msgpackrpc"
	"regexp"
	"strconv"
	"strings"
)

func TraitArgs(args []string) {
	if len(args) == 0 {
		return
	}
	
	// REMOVIDO: args = args[:1] 

	switch args[0] {
	case "ps":
		print("\n")
		instances, err := hazefs.PathsInstance.ReadRunningFile()
		if err != nil {
			fmt.Println(err)
			return
		}

		fmt.Print(cmd.ProcessStatus(instances))
		break

	case "request", "req":
		// Como args[0] é o "req", precisamos ter pelo menos tamanho 3 
		// (0: "req", 1: "localhost:7192", 2: "test/ping[]")
		if len(args) < 3 {
			fmt.Println("Error: request require: <ip:port> <module/acessor>[<parameters>]")
			break
		}

		// O IP:Porta agora é o segundo argumento (índice 1)
		ipport := args[1]
		ip, portStr, found := strings.Cut(ipport, ":")
		if !found {
			fmt.Println("Erro: Formato de IP:Porta inválido. Use ip:porta")
			break
		}

		port, err := strconv.Atoi(portStr)
		if err != nil {
			fmt.Println("Erro: A porta deve ser um número válido")
			break
		}

		// O RPC agora é o terceiro argumento (índice 2)
		rpc := args[2]
		function, _, _ := strings.Cut(rpc, "[")

		pre := regexp.MustCompile(`\[(.*?)\]`)
		match := pre.FindStringSubmatch(rpc)

		var params []any

		if len(match) > 1 && match[1] != "" {
			rawParams := strings.Split(match[1], ",")
			for _, p := range rawParams {
				p = strings.TrimSpace(p)
				if p == "" {
					continue
				}

				if num, err := strconv.Atoi(p); err == nil {
					params = append(params, num)
					continue
				}
				if b, err := strconv.ParseBool(p); err == nil {
					params = append(params, b)
					continue
				}
				p = strings.Trim(p, `"'`)
				params = append(params, p)
			}
		}

		req := &msgpackrpc.Request{}
		req.Init(1, function, params)

		// Executa a chamada
		resp, err := cmd.Request(ip, port, req)
		if err != nil {
			fmt.Println(err)
			break
		}

		// Imprime o resultado formatado!
		fmt.Println(resp.ToString())

		break

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
		fmt.Printf("Comando desconhecido: %s\n", args[0])
	}
}
