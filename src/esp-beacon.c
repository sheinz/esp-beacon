#include "osapi.h"
#include "user_interface.h"
#include "eagle_soc.h"
#include "user_config.h"

/* static const int pin = 1; */
/* static volatile os_timer_t some_timer; */
/*  */
/* void some_timerfunc(void *arg) */
/* { */ /*   //Do blinky stuff */ /*   if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin)) */
/*   { */
/*     // set gpio low */
/*     gpio_output_set(0, (1 << pin), 0, 0); */
/*   } */
/*   else */
/*   { */
/*     // set gpio high */
/*     gpio_output_set((1 << pin), 0, 0, 0); */
/*   } */
/* } */

static void wifi_handle_event_cb(System_Event_t *event)
{
    os_printf("event %x\n", event->event);

    switch (event->event) {
        case EVENT_SOFTAPMODE_STACONNECTED:
            // TODO: start timer to periodically check RSSI 
            os_printf("Clinet connected");
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            // TODO: stop timer and indicate disconnected status
            os_printf("Client disconnected");
            break;
        case EVENT_SOFTAPMODE_PROBEREQRECVED:
            break;
        case EVENT_STAMODE_CONNECTED:
            // TODO: start timer to periodically check RSSI 
            os_printf("Connected to AP");
            break;
        case EVENT_STAMODE_GOT_IP:
            break;
        case EVENT_STAMODE_DHCP_TIMEOUT:
            break;
        case EVENT_STAMODE_DISCONNECTED:
            os_printf("Disconnected from AP");
            // TODO: indicate disconnection and stop timer
            break;
    } 
}

static void station_init()
{
    struct station_config config;

    wifi_set_opmode(STATION_MODE);    // station mode
    
    os_memcpy(config.ssid, BEACON_SSID, sizeof(BEACON_SSID));
    os_memcpy(config.password, BEACON_PASSWORD, sizeof(BEACON_PASSWORD));

    wifi_station_set_config_current(&config);
}

static void ap_init()
{
    wifi_set_opmode(SOFTAP_MODE);    // soft-AP mode

    struct softap_config config;
    os_memcpy(config.ssid, BEACON_SSID, sizeof(BEACON_SSID));
    os_memcpy(config.password, BEACON_PASSWORD, sizeof(BEACON_PASSWORD));
    config.beacon_interval = 100;
    config.max_connection = 1;
    config.ssid_hidden = 0;
    config.ssid_len = sizeof(BEACON_SSID);
    config.authmode = AUTH_WPA_PSK;
    wifi_softap_set_config_current(&config);
    
}

void ICACHE_FLASH_ATTR user_init()
{
    uart_div_modify(0, UART_CLK_FREQ / 115200);
    wifi_status_led_install(2, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

#ifdef BEACON_AP
    ap_init();
#else
    station_init();
#endif

    wifi_set_event_handler_cb(wifi_handle_event_cb);


  /* // init gpio sussytem */
  /* gpio_init(); */
  /*  */
  /* // configure UART TXD to be GPIO1, set as output */
  /* PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);  */
  /* gpio_output_set(0, 0, (1 << pin), 0); */
  /*  */
  /* // setup timer (500ms, repeating) */
  /* os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL); */
  /* os_timer_arm(&some_timer, 500, 1); */
}
