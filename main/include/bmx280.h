#ifndef _BMX280_H_
#define _BMX280_H_

#include <stdint.h>
#include <limits.h>
#include "driver/i2c.h"
#include "sdkconfig.h"

#include "lcd.h"

extern QueueHandle_t lcd_string_queue;
typedef struct bmx280_t bmx280_t;

#include "bmx280_bits.h"

bmx280_t* bmx280_create(i2c_port_t port);
void bmx280_close(bmx280_t* bmx280);
esp_err_t bmx280_init(bmx280_t* bmx280);
esp_err_t bmx280_configure(bmx280_t* bmx280, bmx280_config_t *cfg);
esp_err_t bmx280_setMode(bmx280_t* bmx280, bmx280_mode_t mode);
esp_err_t bmx280_getMode(bmx280_t* bmx280, bmx280_mode_t* mode);
bool bmx280_isSampling(bmx280_t* bmx280);
esp_err_t bmx280_readout(bmx280_t *bmx280, int32_t *temperature, uint32_t *pressure, uint32_t *humidity);

static inline void bmx280_readout2float(int32_t* tin, uint32_t *pin, uint32_t *hin, float *tout, float *pout, float *hout)
{
    if (tin && tout)
        *tout = (float)*tin * 0.01f;
    if (pin && pout)
        *pout = (float)*pin * (1.0f/256.0f);
    if (hin && hout)
        *hout = (*hin == UINT32_MAX) ? -1.0f : (float)*hin * (1.0f/1024.0f);
}

static inline esp_err_t bmx280_readoutFloat(bmx280_t *bmx280, float* temperature, float* pressure, float* humidity)
{
    int32_t t; uint32_t p, h;
    esp_err_t err = bmx280_readout(bmx280, &t, &p, &h);

    if (err == ESP_OK)
    {
        bmx280_readout2float(&t, &p, &h, temperature, pressure, humidity);
    }

    return err;
}

void bmx280_read_task(void *arg);

#endif
