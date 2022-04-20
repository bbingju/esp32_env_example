#include "env_device.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <assert.h>

#define ENV_NV_NAMESPACE__DEVICE "device_info"
#define KEY_GW_ID "gw_id"
#define KEY_BASE_MAC "base_mac"

static const char* TAG = "ENV_DEVICE";

static int _load(void *self);
static int _save(void *self);
static int _reset(void *self);


static struct env_obj_funcs _nv_device_funcs = {
	.load_fp = _load,
	.save_fp = _save,
	.reset_fp = _reset,
};


static struct env_obj* _device_obj;


struct env_obj* env_device()
{
	if (!_device_obj) {
		struct env_obj* obj;
		obj = env_obj_create(&_nv_device_funcs);
		assert(obj);
		obj->readonly = true;
		_device_obj = obj;
	}

	return _device_obj;
}


void env_device_delete()
{
	if (_device_obj) {
		env_obj_delete(_device_obj);
		_device_obj = NULL;
	}
}


const char* env_device_get__base_mac()
{
	cJSON* obj = env_obj_get(env_device(), "/base_mac");
	return obj ? obj->valuestring : NULL;
}


uint32_t env_device_get__gw_id()
{
	cJSON* obj = env_obj_get(env_device(), "/gw_id");
	return obj ? obj->valueint : 0xFFFFFFFF;
}


static int _load(void *self)
{
	struct env_obj* obj = self;
	if (!obj)
		return -1;

	nvs_handle h;
	esp_err_t err;
	int retval = -1;

	err = nvs_open(ENV_NV_NAMESPACE__DEVICE, NVS_READONLY, &h);
	if (err == ESP_ERR_NVS_NOT_FOUND) {
		ESP_LOGW(TAG, "error: there is no namespace \'%s\'", ENV_NV_NAMESPACE__DEVICE);
		/* need plan B */
		return -1;				/* _get_device_as_json_string(); */
	}

	if (err != ESP_OK) {
		return -1;
	}

	/* Get gateway id */
	uint32_t gw_id;
	err = nvs_get_u32(h, KEY_GW_ID, &gw_id);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "error (%d): get NVS (%d)", __LINE__, err);
		goto ret_error;
	}

	ESP_LOGD(TAG, "%s: %u", KEY_GW_ID, gw_id);

	/* Get base mac */
	size_t len = 6;
	uint8_t base_mac[6] = { 0 };
	err = nvs_get_blob(h, KEY_BASE_MAC, base_mac, &len);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "error (%d): nvs_get_blob base_mac (%d)", __LINE__, err);
		goto ret_error;
	}

	char* jsonstr;
	asprintf(&jsonstr, "{\"gw_id\":%d,\"base_mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\"}",
	    gw_id, base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);

	if (!jsonstr)
		goto ret_error;

	obj->jsonobj = cJSON_Parse(jsonstr);
	if (obj->jsonobj)
		retval = 0;

ret_error:
	nvs_close(h);
	return retval;
}


static int _save(void *self)
{
    /* do nothing because of readonly. */
	return 0;
}


static int _reset(void *self)
{
    /* do nothing because of readonly. */
	return 0;
}
