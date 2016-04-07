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

static os_timer_t rssi_timer;
static os_timer_t led_timer;
uint16_t led_change_timeout = 1000;

static void rssi_check_cb(void)
{
    int8_t rssi = wifi_station_get_rssi();
    os_printf("Rssi: %d\n", rssi);

    led_change_timeout = (0-rssi);
    if (led_change_timeout < 40) {
        led_change_timeout = 1;
    } else {
        led_change_timeout -= 40;
    }

    led_change_timeout *= 10;
    os_printf("timout: %d\n", led_change_timeout);
}

static void led_change_cb(void)
{
    static BOOL state = FALSE;

    state = !state;

    if (state) {
        gpio_output_set(BIT2, 0, 0, 0);   // 2 pin output high
        os_printf("on\n");
    } else {
        gpio_output_set(0, BIT2, 0, 0);   // 2 pin output low
        os_printf("off\n");
    }
    
    os_timer_disarm(&led_timer);
    os_timer_setfn(&led_timer, led_change_cb, NULL);
    os_timer_arm(&led_timer, led_change_timeout, FALSE);
}

static void enable(BOOL on)
{
    if (on) {
        os_timer_disarm(&rssi_timer);
        os_timer_setfn(&rssi_timer, rssi_check_cb, NULL);
        os_timer_arm(&rssi_timer, 1000, TRUE);
        os_timer_disarm(&led_timer);
        os_timer_setfn(&led_timer, led_change_cb, NULL);
        os_timer_arm(&led_timer, led_change_timeout, FALSE);
    } else {
        os_timer_disarm(&rssi_timer); 
        os_timer_disarm(&led_timer);
    }
}

static void wifi_handle_event_cb(System_Event_t *event)
{
    os_printf("event %x\n", event->event);

    switch (event->event) {
        case EVENT_SOFTAPMODE_STACONNECTED:
            os_printf("Clinet connected\n");
            enable(TRUE);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            os_printf("Client disconnected\n");
            enable(FALSE);
            break;
        case EVENT_SOFTAPMODE_PROBEREQRECVED:
            break;
        case EVENT_STAMODE_CONNECTED:
            os_printf("Connected to AP\n");
            enable(TRUE);
            break;
        case EVENT_STAMODE_GOT_IP:
            break;
        case EVENT_STAMODE_DHCP_TIMEOUT:
            break;
        case EVENT_STAMODE_DISCONNECTED:
            os_printf("Disconnected from AP\n");
            enable(FALSE);
            break;
    } 
}

static void station_init()
{
    struct station_config config;

    wifi_set_opmode(STATION_MODE);    // station mode
    
    os_memcpy(config.ssid, BEACON_SSID, sizeof(BEACON_SSID));
    os_memcpy(config.password, BEACON_PASSWORD, sizeof(BEACON_PASSWORD));
    config.bssid_set = 0;

    wifi_station_set_config_current(&config);
}

static void ap_init()
{
    if (!wifi_set_opmode(SOFTAP_MODE)){
        os_printf("Failed to set AP mode");
    }

    struct softap_config config = {0};
    wifi_softap_dhcps_stop();

    wifi_softap_get_config(&config);
    os_memcpy(config.ssid, BEACON_SSID, sizeof(BEACON_SSID));
    os_memcpy(config.password, BEACON_PASSWORD, sizeof(BEACON_PASSWORD));
    config.beacon_interval = 100;
    config.max_connection = 2;
    config.ssid_hidden = 0;
    config.channel = 7;
    config.ssid_len = sizeof(BEACON_SSID) - 1;
    config.authmode = AUTH_WPA_WPA2_PSK;
    if (!wifi_softap_set_config(&config)) {
        os_printf("Failed to set AP config");
    }

    if (!wifi_softap_dhcps_start()) {
        os_printf("Failed to start DHCP");
    }
}

void ICACHE_FLASH_ATTR user_init()
{
    uart_div_modify(0, UART_CLK_FREQ / 115200);
    /* wifi_status_led_install(2, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); */
    os_printf("\nStarted");

#ifdef BEACON_AP
    /* wifi_set_opmode(NULL_MODE); */
    /* os_timer_disarm(&start_timer); */
    /* os_timer_setfn(&start_timer, (os_timer_func_t *)timer_cb, NULL); */
    /* os_timer_arm(&start_timer, 500, 0); */
    ap_init();
#else
    station_init();
#endif

    wifi_set_event_handler_cb(wifi_handle_event_cb);


  // init gpio sussytem
  gpio_init();

  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); 
  gpio_output_set(BIT2, 0, BIT2, 0);   // 2 pin output low

  /*  */
  /* // configure UART TXD to be GPIO1, set as output */
  /* PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);  */
  /* gpio_output_set(0, 0, (1 << pin), 0); */
  /*  */
  /* // setup timer (500ms, repeating) */
  /* os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL); */
  /* os_timer_arm(&some_timer, 500, 1); */
}
