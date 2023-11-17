#include "lcd.h"

static i2c_port_t I2C_PORT;

esp_err_t err;
extern QueueHandle_t lcd_string_queue = NULL;

#define SLAVE_ADDRESS_LCD 0x4E >> 1
static const char *TAG = "LCD";

void lcd_send_cmd(char cmd) {
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (cmd & 0xf0);
  data_l = ((cmd << 4) & 0xf0);
  data_t[0] = data_u | 0x0C; // en=1, rs=0
  data_t[1] = data_u | 0x08; // en=0, rs=0
  data_t[2] = data_l | 0x0C; // en=1, rs=0
  data_t[3] = data_l | 0x08; // en=0, rs=0
  err = i2c_master_write_to_device(I2C_PORT, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
  if (err != 0) {
    ESP_LOGI(TAG, "Error in sending command");
  }
}

void lcd_send_data(char data) {
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (data & 0xf0);
  data_l = ((data << 4) & 0xf0);
  data_t[0] = data_u | 0x0D; // en=1, rs=1
  data_t[1] = data_u | 0x09; // en=0, rs=1
  data_t[2] = data_l | 0x0D; // en=1, rs=1
  data_t[3] = data_l | 0x09; // en=0, rs=1
  err = i2c_master_write_to_device(I2C_PORT, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
  if (err != 0) {
    ESP_LOGI(TAG, "Error in sending data");
  }
}

void lcd_clear(void) {
  lcd_send_cmd(0x01);
  usleep(5000);
}

void lcd_set_pos(int row, int col) {
  // 1 AD AD AD AD AD AD AD  - Установка адреса DDRAM
  // 0x40 + 0x04 | 1000 0000 (выставить на 4 колонку второй строки)

  // 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13
  // 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53
  // 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27
  // 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 60 61 62 63 64 65 66 67

  uint8_t pos = 0x01;
  switch (row) {
  case 0:
    pos = (0x00 + col) | 0x80;
    break;
  case 1:
    pos = (0x40 + col) | 0x80;
    break;
  case 2:
    pos = (0x14 + col) | 0x80;
    break;
  case 3:
    pos = (0x54 + col) | 0x80;
    break;
  }
  lcd_send_cmd(pos);
}

void lcd_init(i2c_port_t port) {
  I2C_PORT = port;
  // file://LCD1602.pdf:14
  // 4 bit initialisation
  usleep(50000); // wait for >40ms
  lcd_send_cmd(0x30);
  usleep(5000); // wait for >4.1ms
  lcd_send_cmd(0x30);
  usleep(200); // wait for >100us
  lcd_send_cmd(0x30);
  usleep(10000);
  lcd_send_cmd(0x20); // 4bit mode
  usleep(10000);
  // file://Хартов_МПС.pdf
  // display initialisation
  lcd_send_cmd(0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line
                      // display) F = 0 (5x8 characters)
  usleep(1000);
  lcd_send_cmd(
      0x08); // Display on/off control --> D=0,C=0, B=0  ---> display off
  usleep(1000);
  lcd_send_cmd(0x01); // clear display
  usleep(1000);
  usleep(1000);
  lcd_send_cmd(
      0x06); // Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
  usleep(1000);
  lcd_send_cmd(0x0C); // Display on/off control --> D = 1, C and B = 0. (Cursor
                      // and blink, last two bits)
  usleep(1000);
}

void lcd_send_string(char *str) {
  while (*str) {
    lcd_send_data(*str++);
  }
}

void lcd_process_queue_task(void *arg) {
  lcd_data_t lcd_data;
  while (true) {
    if (xQueueReceive(lcd_string_queue, &lcd_data, portMAX_DELAY)) {
      lcd_set_pos(lcd_data.row, lcd_data.col);
      lcd_send_string(lcd_data.str);
      xQueueSendToBack(ws_send_queue, &lcd_data.str, 0);
    }
  }
}
