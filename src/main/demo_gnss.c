/* firmware.h contains all relevant headers */
#include "../firmware.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>


int main(void){

    int32_t *longitude_p, *latitude_p, *altitude_p;
    int32_t longitude=0, latitude=0, altitude=0;
    uint8_t *long_sd_p, *lat_sd_p, *alt_sd_p;
    uint8_t long_sd=0, lat_sd=0, alt_sd=0;

    Board_init();
    WDT_kick();
    LED_on(LED_B);

    longitude_p = &longitude;
    latitude_p = &latitude;
    altitude_p = &altitude;
    long_sd_p = &long_sd;
    lat_sd_p = &lat_sd;
    alt_sd_p = &alt_sd;

    while(1){
        Delay_ms(2500);
        LED_on(LED_B);
        GNSS_Init();
        UART_init(UART_CAMERA, 9600);
        UART_puts(UART_CAMERA, "\n\rGNSS data collection:\n\r");
        char message[500];
        GNSS_getData(longitude_p, latitude_p, altitude_p, long_sd_p, lat_sd_p, alt_sd_p);
        UART_puts(UART_CAMERA, "\n\rData collection complete:");
        sprintf(message, "\n\rLongitude: %" PRId32 "\n\rLatitude: %" PRId32 "\n\rAltitude: %" PRId32 "\n\rLatitude s_d: %" PRId8 "\n\rLongitude s_d: %" PRId8 "\n\rAltitude s_d: %" PRId8 "\n\r", *longitude_p, *latitude_p, *altitude_p, *long_sd_p, *lat_sd_p, *alt_sd_p);
        UART_puts(UART_CAMERA, message);
        UART_puts(UART_CAMERA, "Waiting for next collection...\n\r");
        Delay_ms(2500);
        LED_off(LED_B);

    }
    return 0;
}
