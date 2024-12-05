#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "onewire.h"
#include "rom/ets_sys.h"


static const char *TAG = "DS18B20";
static uint8_t ledState = 0;
#define GPIO_INPUT    5
#define GPIO_OUTPUT  19



// OneWire commands
#define DS18x20_READROM            0x33  // Read ROM (family code + serial number + CRC)
#define DS18x20_TEMP_CONVERT       0x44  // Initiate a single temperature conversion
#define DS18x20_SCRATCHPAD_WRITE   0x4E  // Write 3 bytes of data to the device scratchpad at positions 2, 3 and 4
#define DS18x20_SCRATCHPAD_READ    0xBE  // Read 9 bytes of data (including CRC) from the device scratchpad
#define DS18x20_SCRATCHPAD_COPY    0x48  // Copy the contents of the scratchpad to the device EEPROM
#define DS18x20_EEPROM_RECALL      0xB8  // Restore alarm trigger values and configuration data from EEPROM to the scratchpad
#define ds18x20_READ_PWRSUPPLY     0xB4  // Determine if a device is using parasitic power
#define DS18x20_ALARM_SEARCH       0xEC  // Query bus for devices with an alarm condition

// Scratchpad locations
#define SP_TEMP_LSB                0
#define SP_TEMP_MSB                1
#define SP_HIGH_ALARM_TEMP         2
#define SP_LOW_ALARM_TEMP          3
#define SP_CONFIGURATION           4
#define SP_INTERNAL_BYTE           5
#define SP_COUNT_REMAIN            6
#define SP_COUNT_PER_C             7
#define SP_SCRATCHPAD_CRC          8

#define CONVERSION_TIMEOUT_9_BIT   94
#define CONVERSION_TIMEOUT_10_BIT  188
#define CONVERSION_TIMEOUT_11_BIT  375
#define CONVERSION_TIMEOUT_12_BIT  750

// ERROR CODES
#define E_onewire_reset            1
#define E_onewire_skip_rom         2
#define E_onewire_write            3
#define E_onewire_read_crc         4
// FUNCTION WRAPPERS
#define try(func)  error=func; if(error) return error;
#define try_onewire_reset(pin)     if( !onewire_reset(pin)     ) return E_onewire_reset
#define try_onewire_skip_rom(pin)  if( !onewire_skip_rom(pin)  ) return E_onewire_skip_rom
#define try_onewire_write(pin,val) if( !onewire_write(pin,val) ) return E_onewire_write

static uint8_t buffer[10];

int sendCommand(gpio_num_t pin, uint8_t command){
    try_onewire_reset(pin); // Step 1. Initialization
    try_onewire_skip_rom(pin); // Step 2. ROM Command (followed by any required data exchange
    try_onewire_write(pin, command); // Step 3. DS18B20 Function Command : initiate a temperature measurement
    return 0;
}

int readScratchpad(gpio_num_t pin, uint8_t *buffer){
    // Read data    
    for (int i = 0; i < 8; i++)
    buffer[i] = onewire_read(pin);

    // Verify CRC
    uint8_t crc;
    uint8_t expected_crc; 
    crc = onewire_read(pin);
    expected_crc = onewire_crc8(buffer, 8);     
    if (crc != expected_crc) return E_onewire_read_crc;
    return 0;
}

int readTempereture(gpio_num_t pin){
    int error;
    // initiate a temperature measurement
    try (sendCommand(pin, DS18x20_TEMP_CONVERT))
    // Wait for the measurement 
    ets_delay_us(750000); // 750ms
    // Ask for reading
    try ( sendCommand(pin, DS18x20_SCRATCHPAD_READ) )
    try ( readScratchpad(pin, buffer) )
    return 0;
}

void app_main(void)
{
    int pt = portTICK_PERIOD_MS;
    ESP_LOGI(TAG, "***** APP STARTED *****" );
    ESP_LOGI(TAG, "portTICK_PERIOD_MS: %u", pt);

    // CONFIG OUTPUT
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL<<GPIO_OUTPUT ; //((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    // CONFIG INPUT
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL<<GPIO_INPUT ;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    int error;
    while(1){
        // TEMPERATURE
        error = readTempereture(GPIO_INPUT);
        if(error){
            ESP_LOGE(TAG,"error code: %d",error);
        }else{
            // SUCCESS
            ESP_LOGI(TAG, "Sensor [%d]: scratchpad: "
                "temp: %02X %02X, alarm: %02X %02X, config: %02X, reserved: %02X %02X %02X", 
                GPIO_INPUT, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
        }

        // LED
        ledState = !ledState;
        ESP_LOGI(TAG, "ledState: %s!", ledState == true ? "ON" : "OFF");
        gpio_set_level(GPIO_OUTPUT, ledState);

        
        vTaskDelay( 3000 / portTICK_PERIOD_MS );
    }

}

