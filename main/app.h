#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "compiler_port.h"
#include <esp_event.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spiffs.h"
/* #include "apdu.h" */

ESP_EVENT_DECLARE_BASE(APP_EVENTS);

enum {
	APP_EVENT__INIT_SINK,
	APP_EVENT__SINK_PREPARED,
	APP_EVENT__SINK_FAILED,
	APP_EVENT__NETWORK_CONNECTED,
	APP_EVENT__NETWORK_DISCONNECTED,
	APP_EVENT__TIME_OBTAINED,
	APP_EVENT__MQTT_CLIENT_CONNECTED,
	APP_EVENT__MQTT_CLIENT_DISCONNECTED,
	APP_EVENT__WNT_ESTABLISHED,
	APP_EVENT__LINK_CONNECTED,
	APP_EVENT__LINK_DISCONNECTED,
	APP_EVENT__DATA_CREATED,
	APP_EVENT__UPDATE_CTRL_STATE,
	APP_EVENT__FACTORY_RESET,
};


typedef struct app_fs_info {
	size_t total;
	size_t used;
} app_fs_info_t;


struct app_fs {
	esp_vfs_spiffs_conf_t conf;
	struct app_fs_info info;
};


struct app_ctx {
	bool initialized;
	struct app_fs fs;
};

typedef struct app_ctx* app_ctx_t;

APP_C__BEGIN_DECLS

app_ctx_t app_init(shutdown_handler_t, esp_event_handler_t);
app_ctx_t app_ctx();
app_fs_info_t* app_fs_info(app_ctx_t const);
void app_reboot();

APP_C__END_DECLS
