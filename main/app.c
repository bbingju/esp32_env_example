#include "app.h"

#include "esp_err.h"
#include "esp_netif.h"
#include "freertos/projdefs.h"
#include "freertos/timers.h"
#include "nvs_flash.h"

#define WAITING_TIME_BEFORE_REBOOT pdMS_TO_TICKS(3 * 1000)

const static char* TAG = "APP";

static struct app_ctx _ctx = {
	.fs = {
	    .conf = {
	        .base_path = "/envconf",
	        .partition_label = NULL,
	        .max_files = 5,
	        .format_if_mount_failed = true,
	    },
	},
};

ESP_EVENT_DEFINE_BASE(APP_EVENTS);

extern void app_network_set_shutdown();


static void _shutdown_handler()
{
	/* esp_netif_deinit(); */
	esp_vfs_spiffs_unregister(_ctx.fs.conf.partition_label);
	nvs_flash_deinit();
}

static void _timer_callback_reboot(TimerHandle_t xTimer)
{
	app_network_set_shutdown();
	esp_restart();
}

static void _init_nvs()
{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		/* NVS partition was truncated and needs to be erased Retry
		 * nvs_flash_init */
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);
}

static void _init_fs()
{
	esp_err_t err = esp_vfs_spiffs_register(&_ctx.fs.conf);
	if (err != ESP_OK) {
		if (err == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (err == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(err));
		}
		return;
	}

	app_fs_info_t *info = app_fs_info(&_ctx);
	if (info) {
		ESP_LOGI(TAG, "Partition size: total: %d, used: %d", info->total, info->used);
	}
}


app_ctx_t app_ctx()
{
	return &_ctx;
}


app_ctx_t app_init(shutdown_handler_t shutdown_handler,
				   esp_event_handler_t event_handler)
{
	_init_nvs();
	_init_fs();

	/* /\* Initialize esp netif *\/ */
	/* ESP_ERROR_CHECK(esp_netif_init()); */

	/* /\* Initialize pre-defined event loop *\/ */
	/* ESP_ERROR_CHECK(esp_event_loop_create_default()); */


	ESP_ERROR_CHECK(esp_register_shutdown_handler(_shutdown_handler));
	if (shutdown_handler) {
		ESP_ERROR_CHECK(esp_register_shutdown_handler(shutdown_handler));
	}

	_ctx.initialized = true;

	/* ESP_ERROR_CHECK(esp_event_handler_register(APP_EVENTS, ESP_EVENT_ANY_ID, event_handler, &_ctx)); */

	return &_ctx;
}

app_fs_info_t* app_fs_info(app_ctx_t const ctx)
{
	struct app_fs* const fs = &ctx->fs;
	struct app_fs_info* const info = &fs->info;
	esp_err_t err;

	err = esp_spiffs_info(fs->conf.partition_label, &info->total, &info->used);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(err));
		return NULL;
	}

	return info;
}

void app_reboot()
{
	TimerHandle_t _timer  = xTimerCreate( "reboot_timer",
									 WAITING_TIME_BEFORE_REBOOT,
									 pdFALSE,
									 (void *) 0,
									 _timer_callback_reboot );
	if (!_timer) {
		_timer_callback_reboot(NULL);
	} else {
		xTimerStart(_timer, 0);
	}
}
