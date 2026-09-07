package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)
type JSONFunction struct {
	Description string `json:"description"`
	Return      string `json:"return"`
}
type JSONRepresentation struct {
	Modules map[string]map[string]JSONFunction `json:"modules"`
}

type Represent struct {
	Module      string `json:"module"`
	Acessor     string `json:"acessor"`
	Description string `json:"description"`
	Return      string `json:"return"`
}

func ReadFile(path string) (string, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return "", err
	}

	return string(data), nil
}

func GetHeaderFiles(dirPath string) ([]string, error) {
	pattern := filepath.Join(dirPath, "*.h")

	files, err := filepath.Glob(pattern)
	if err != nil {
		return nil, err
	}

	return files, nil
}

func ParseTag(line string, tag string) string {
	line = strings.TrimSpace(line)

	if !strings.HasPrefix(line, tag) {
		return ""
	}

	return strings.TrimSpace(strings.TrimPrefix(line, tag))
}

func ParseHeader(content string) ([]Represent, error) {
	var result []Represent
	var comment []string

	lines := strings.Split(content, "\n")

	for _, line := range lines {
		line = strings.TrimSpace(line)

		if strings.HasPrefix(line, "///") {
			comment = append(
				comment,
				strings.TrimSpace(strings.TrimPrefix(line, "///")),
			)
			continue
		}

		if len(comment) == 0 {
			continue
		}

		rep := Represent{}

		for _, commentLine := range comment {
			if value := ParseTag(commentLine, "@module"); value != "" {
				rep.Module = value
			}

			if value := ParseTag(commentLine, "@acessor"); value != "" {
				rep.Acessor = value
			}

			if value := ParseTag(commentLine, "@description"); value != "" {
				rep.Description = value
			}

			if value := ParseTag(commentLine, "@return"); value != "" {
				rep.Return = value
			}
		}

		result = append(result, rep)
		comment = nil
	}

	return result, nil
}

func BuildJSON(represents []Represent) ([]byte, error) {
	result := JSONRepresentation{
		Modules: make(map[string]map[string]JSONFunction),
	}

	for _, rep := range represents {
		if _, exists := result.Modules[rep.Module]; !exists {
			result.Modules[rep.Module] = make(map[string]JSONFunction)
		}

		result.Modules[rep.Module][rep.Acessor] = JSONFunction{
			Description: rep.Description,
			Return:      rep.Return,
		}
	}

	return json.MarshalIndent(result, "", "  ")
}

func CreateFile(path string, content []byte) (error) {
	err := os.WriteFile(path, []byte(content), 0644)
	if err != nil {
		return err
	}

	return nil
}

func main() {
	path := "../functions/"

	fileNames, err := GetHeaderFiles(path)
	if err != nil {
		println("script err:", err.Error())
		return
	}

	represents := make([]Represent, 0)

	for _, fileName := range fileNames {
		file, err := ReadFile(fileName)
		if err != nil {
			println("erro ao ler arquivo:", err.Error())
			return
		}

		println("found:", fileName)

		rep, err := ParseHeader(file)
		if err != nil {
			return
		}

		represents = append(represents, rep...)
	}
	for _, rep := range represents {
		fmt.Printf("%+v\n", rep)
	}
	print("building json... ")
	json, e := BuildJSON(represents)
	if e != nil {
		println("script error: ", e.Error())
		return
	}

	err =CreateFile("generated.json", json)
	if err != nil {
		println("error")
		println("script error: ", err.Error())
		return
	} 
	print("done.")
	return


}
