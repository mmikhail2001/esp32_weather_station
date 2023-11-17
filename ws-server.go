package main

import (
	"encoding/hex"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"net/http"
	"strconv"
	"strings"

	"github.com/gorilla/websocket"
)

var addr = flag.String("addr", ":80", "http service address")

var upgrader = websocket.Upgrader{} // use default options

var (
	prsData []float64
	humData []float64
	gasData []float64
	tmpData []float64
)

type SystemInfo struct {
	MacAddress       string `json:"mac_address"`
	Model            string `json:"model"`
	CoreCount        int    `json:"core_count"`
	SiliconRevMajor  int    `json:"silicon_revision_major"`
	SiliconRevMinor  int    `json:"silicon_revision_minor"`
	CPUFrequencyMHz  int    `json:"cpu_frequency_mhz"`
	FreeHeapBytes    int    `json:"free_heap_bytes"`
	MinFreeHeapBytes int    `json:"minimum_free_heap_bytes"`
	FlashSizeMB      int    `json:"flash_size_mb"`
	ESPIDFVersion    string `json:"esp_idf_version"`
	UptimeSeconds    int64  `json:"uptime_seconds"`
	ElfSha256        string `json:"elf_sha256"`
}

var systemInfo SystemInfo

var commandChannel = make(chan string)

func restartHandlerHTTP(w http.ResponseWriter, r *http.Request) {
	commandChannel <- "restart"
	fmt.Fprintln(w, "Command 'restart' sent")
}

func statHandlerHTTP(w http.ResponseWriter, r *http.Request) {
	commandChannel <- "stat"
	fmt.Fprintln(w, "Command 'stat' sent")
}

func showHandlerHTTP(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "prs: %v\n", prsData)
	fmt.Fprintf(w, "hum: %v\n", humData)
	fmt.Fprintf(w, "gas: %v\n", gasData)
	fmt.Fprintf(w, "tmp: %v\n", tmpData)
}

func printSystemInfo() {
	fmt.Println("Model:", systemInfo.Model)
	fmt.Println("Core Count:", systemInfo.CoreCount)
	fmt.Println("Silicon Revision Major:", systemInfo.SiliconRevMajor)
	fmt.Println("Silicon Revision Minor:", systemInfo.SiliconRevMinor)
	fmt.Println("CPU Frequency MHz:", systemInfo.CPUFrequencyMHz)
	fmt.Println("Free Heap Bytes:", systemInfo.FreeHeapBytes)
	fmt.Println("Minimum Free Heap Bytes:", systemInfo.MinFreeHeapBytes)
	fmt.Println("Flash Size MB:", systemInfo.FlashSizeMB)
	fmt.Println("ESP-IDF Version:", systemInfo.ESPIDFVersion)
	fmt.Println("Uptime Seconds:", systemInfo.UptimeSeconds)

	fmt.Printf("Mac Address (Hex): %s\n", hex.EncodeToString([]byte(systemInfo.MacAddress)))
	fmt.Printf("Elf Sha256 (Hex): %s\n", hex.EncodeToString([]byte(systemInfo.ElfSha256)))
}

func readWriteWS(w http.ResponseWriter, r *http.Request) {
	c, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Print("upgrade:", err)
		return
	}
	defer c.Close()

	log.Printf("Client connected: %s", c.RemoteAddr())

	c.SetCloseHandler(func(code int, text string) error {
		log.Printf("connection closed with code %d: %s", code, text)
		return nil
	})

	for {
		mt, message, err := c.ReadMessage()
		if err != nil {
			log.Println("read:", err)
			break
		}
		log.Printf("recv: %s", message)

		if strings.Contains(string(message), `"mac_address"`) {
			err := json.Unmarshal(message, &systemInfo)
			if err != nil {
				log.Println("json unmarshal error:", err)
			}
			printSystemInfo()
			continue
		}

		parts := strings.Fields(string(message))
		if len(parts) == 2 {
			sensorType := parts[0]
			valueStr := parts[1]
			value, err := strconv.ParseFloat(valueStr, 64)
			if err != nil {
				log.Println("invalid value:", err)
			} else {
				switch sensorType {
				case "prs":
					prsData = append(prsData, value)
				case "hum":
					humData = append(humData, value)
				case "gas":
					gasData = append(gasData, value)
				case "tmp":
					tmpData = append(tmpData, value)
				default:
					log.Println("unknown sensor type:", sensorType)
				}
			}
		} else {
			log.Println("invalid message format")
		}

		select {
		case cmd := <-commandChannel:
			err := c.WriteMessage(mt, []byte(cmd))
			if err != nil {
				log.Println("write:", err)
			}
		default:
		}
	}
}

func main() {
	flag.Parse()
	log.SetFlags(0)
	http.HandleFunc("/", readWriteWS)
	http.HandleFunc("/show", showHandlerHTTP)
	http.HandleFunc("/restart", restartHandlerHTTP)
	http.HandleFunc("/stat", statHandlerHTTP)

	log.Fatal(http.ListenAndServe(*addr, nil))
}
