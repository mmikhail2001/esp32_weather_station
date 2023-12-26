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
	"time"

	"github.com/gorilla/websocket"

	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api/v5"
)

// TODO:
// после перезагрузки МК не ловит событие подключения к websocket клиенту

var bot *tgbotapi.BotAPI
var addr = flag.String("addr", ":80", "http service address")
var upgrader = websocket.Upgrader{} // use default options
var location *time.Location

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
	LastUpdate       time.Time
}

var oldSystemInfo SystemInfo
var initOldSystemInfo = false

type command struct {
	cmd    string
	chatID int64
}

var commandChannel = make(chan command)
var statChannel = make(chan []byte)

func restartHandler(chatID int64) {
	cmd := command{cmd: "restart", chatID: chatID}
	commandChannel <- cmd
}

func statHandler(chatID int64) {
	cmd := command{cmd: "stat", chatID: chatID}
	commandChannel <- cmd
}

func showHandler(chatID int64) error {
	for len(prsData) == 0 && len(humData) == 0 && len(gasData) == 0 && len(tmpData) == 0 {
		log.Println("massives with sensors data are empty")
		time.Sleep(500 * time.Millisecond)
	}

	prsCurrent := prsData[len(prsData)-1]
	humCurrent := humData[len(humData)-1]
	gasCurrent := gasData[len(gasData)-1]
	tmpCurrent := tmpData[len(tmpData)-1]
	message := fmt.Sprintf("pressure: %v\nhumidity: %v\ngas: %v\ntemperature: %v\n",
		prsCurrent, humCurrent, gasCurrent, tmpCurrent)
	msg := tgbotapi.NewMessage(chatID, message)
	_, err := bot.Send(msg)
	if err != nil {
		return err
	}
	return nil
}

func sendSystemInfo(chatID int64, systemInfo SystemInfo) {
	message := fmt.Sprintln("Last update stats:", systemInfo.LastUpdate.In(location).Format(time.RFC822))
	message += fmt.Sprintln("Model:", systemInfo.Model)
	message += fmt.Sprintln("Core Count:", systemInfo.CoreCount)
	message += fmt.Sprintln("Silicon Revision Major:", systemInfo.SiliconRevMajor)
	message += fmt.Sprintln("Silicon Revision Minor:", systemInfo.SiliconRevMinor)
	message += fmt.Sprintln("CPU Frequency MHz:", systemInfo.CPUFrequencyMHz)
	message += fmt.Sprintln("Free Heap Bytes:", systemInfo.FreeHeapBytes)
	message += fmt.Sprintln("Minimum Free Heap Bytes:", systemInfo.MinFreeHeapBytes)
	message += fmt.Sprintln("Flash Size MB:", systemInfo.FlashSizeMB)
	message += fmt.Sprintln("ESP-IDF Version:", systemInfo.ESPIDFVersion)
	message += fmt.Sprintln("Uptime Seconds:", systemInfo.UptimeSeconds)
	message += fmt.Sprintf("Mac Address (Hex): %s\n", hex.EncodeToString([]byte(systemInfo.MacAddress)))
	message += fmt.Sprintf("Elf Sha256 (Hex): %s\n", hex.EncodeToString([]byte(systemInfo.ElfSha256)))
	msg := tgbotapi.NewMessage(chatID, message)
	bot.Send(msg)
}

func WriteWS(stopChannel <-chan struct{}, c *websocket.Conn) {
	for {
		select {
		case cmd := <-commandChannel:
			if cmd.cmd == "stat" {
				if initOldSystemInfo && time.Now().Before(oldSystemInfo.LastUpdate.Add(1*time.Minute)) {
					sendSystemInfo(cmd.chatID, oldSystemInfo)
					continue
				}
			}
			err := c.WriteMessage(websocket.TextMessage, []byte(cmd.cmd))
			if err != nil {
				log.Println("write:", err)
			}
			if cmd.cmd == "stat" {
				// не принимаем команды, пока не отдадим текущую статистику
				statBytes := <-statChannel
				var systemInfo SystemInfo
				err := json.Unmarshal(statBytes, &systemInfo)
				if err != nil {
					log.Println("json unmarshal error:", err)
				}
				systemInfo.LastUpdate = time.Now()
				sendSystemInfo(cmd.chatID, systemInfo)
				oldSystemInfo = systemInfo
				initOldSystemInfo = true
			}
		case _ = <-stopChannel:
			log.Println("WriteWS stopChannel")
			return
		default:
		}
	}

}

func ReadWS(w http.ResponseWriter, r *http.Request) {
	c, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Print("upgrade:", err)
		return
	}
	defer c.Close()
	stopChannel := make(chan struct{})
	go WriteWS(stopChannel, c)
	defer c.Close()

	log.Printf("Client connected: %s", c.RemoteAddr())

	c.SetCloseHandler(func(code int, text string) error {
		log.Printf("connection closed with code %d: %s", code, text)
		return nil
	})

	for {
		// стоит на этой функции после отключения МК от питания...
		// не обрабатывается закрытие соединения
		mt, message, err := c.ReadMessage()
		if err != nil {
			log.Println("error read:", err)
			break
		}
		if mt == websocket.CloseMessage {
			log.Printf("connection closed with code")
			stopChannel <- struct{}{}
			return
		}

		if strings.Contains(string(message), `"mac_address"`) {
			statChannel <- message
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

	}
}

func main() {
	flag.Parse()
	log.SetFlags(0)
	http.HandleFunc("/", ReadWS)
	server := &http.Server{
		Addr:         "0.0.0.0:80",
		ReadTimeout:  10 * time.Second,
		WriteTimeout: 10 * time.Second,
	}
	go func() {
		log.Fatal(server.ListenAndServe())
	}()

	var err error

	location, err = time.LoadLocation("Europe/Moscow")
	if err != nil {
		panic(err)
	}

	bot, err = tgbotapi.NewBotAPI("6621611037:AAGha-ODnoEssGLaGM0Wbdfc5DtbgBriEP4")
	if err != nil {
		panic(err)
	}
	updateConfig := tgbotapi.NewUpdate(0)
	bot.Debug = true

	commands := []tgbotapi.BotCommand{
		{Command: "start", Description: "Описание бота"},
		{Command: "stat", Description: "Системная статистика esp32"},
		{Command: "restart", Description: "Перезагрузка esp32"},
		{Command: "sensors", Description: "Показать показания датчиков"},
	}
	cfg := tgbotapi.NewSetMyCommands(commands...)

	_, err = bot.Request(cfg)
	if err != nil {
		log.Println("Request cfg", err)
	}

	updateConfig.Timeout = 30
	updates := bot.GetUpdatesChan(updateConfig)

	for update := range updates {
		if update.Message == nil {
			continue
		}

		chatID := update.Message.Chat.ID

		switch update.Message.Text {
		case "/start":
			msg := tgbotapi.NewMessage(chatID, "Добро пожаловать в бота 'BMSTU Microclimate Bot'!")
			bot.Send(msg)
		case "/stat":
			statHandler(chatID)
		case "/sensors":
			if err := showHandler(chatID); err != nil {
				log.Println("statHandler err", err)
			}
		case "/restart":
			restartHandler(chatID)
		default:
			msg := tgbotapi.NewMessage(chatID, "Unknown command")
			msg.ReplyToMessageID = update.Message.MessageID
			if _, err := bot.Send(msg); err != nil {
				panic(err)
			}
		}
	}
}

