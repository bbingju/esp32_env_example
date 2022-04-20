#include "env_device.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define ENV_NV_NAMESPACE__USING_CONFIG "using"
#define ENV_NV_KEY__CONFIG "config"


static const char* TAG = "ENV_CONFIG";


static int _load(void *self);
static int _save(void *self);
static int _reset(void *self);


static struct env_obj_funcs _nv_config_funcs = {
	.load_fp = _load,
	.save_fp = _save,
	.reset_fp = _reset,
};


static struct env_obj* _config_obj;

static const char* _factory_config = "{\"network\":{\"default_mode\":\"wifi_sta\",\"wifi_sta\":[{\"ssid\":\"litmus_lte\",\"password\":\"litmus@081213\"}]},\"sntp\":{\"servers\":[\"2.kr.pool.ntp.org\",\"ntp2.kornet.net\",\"time.google.com\",\"time.bora.net\"]},\"mqtt\":{\"enabled\":true,\"broker_uri\":\"mqtt://wnt.litmuscloud.com\",\"broker_port\":1883,\"broker_username\":\"mqttmasteruser\",\"broker_password\":\"V1ZJDQHQ4QFUjUZ3yqY0HrhhFWDMv\"},\"wirepas\":{\"wnt_enabled\":true,\"sinks\":[{\"node_address\":0,\"network_address\":10597064,\"network_channel\":8}]},\"th\":{\"temperature_enabled\":true,\"humidity_enabled\":true,\"measuring_interval_s\":60,\"uri\":\"collect.litmuscloud.com\",\"port\":14000},\"system\":{\"req_host\":\"tower.litmuscloud.com\",\"req_host_enabled\":false,\"timezone\":\"KST-9\"}}";

struct env_obj* env_config()
{
	if (!_config_obj) {
		struct env_obj* obj;
		obj = env_obj_create(&_nv_config_funcs);
		assert(obj);
		obj->readonly = false;
		_config_obj = obj;
	}

	return _config_obj;
}


void env_config_delete()
{
	if (_config_obj) {
		env_obj_delete(_config_obj);
		_config_obj = NULL;
	}
}


const char* env_config_get__wifi_ssid(int index)
{
	char path[48] = { 0 };
	sprintf(path, "/network/wifi_sta/%d/ssid", index);
	return cJSON_GetStringValue(env_obj_get(env_config(), path));
}


const char* env_config_get__wifi_password(int index)
{
	char path[48] = { 0 };
	sprintf(path, "/network/wifi_sta/%d/password", index);
	return cJSON_GetStringValue(env_obj_get(env_config(), path));
}


static esp_err_t _from_factory_config(void *self)
{
	struct env_obj* obj = self;
	if (!obj)
		return ESP_FAIL;

	cJSON* jsonobj = cJSON_Parse(_factory_config);
	if (!jsonobj) {
		return ESP_FAIL;
	}

	cJSON_Delete(obj->jsonobj);

	obj->jsonobj = jsonobj;

	return ESP_OK;
}


static int _load(void *self)
{
	struct env_obj* obj = self;
	if (!obj)
		return -1;

	nvs_handle_t handle;
	esp_err_t err;

	err = nvs_open(ENV_NV_NAMESPACE__USING_CONFIG, NVS_READONLY, &handle);
	if (err == ESP_ERR_NVS_NOT_FOUND) {
		/* plan B */
		err = _from_factory_config(obj);
		return (err != ESP_OK) ? -1 : 0;
	}

	size_t jsonstrlen = 0;
	err = nvs_get_str(handle, ENV_NV_KEY__CONFIG, NULL, &jsonstrlen);
	ESP_LOGI(TAG, "%s: The key \'%s\' required string length: %d",
			 __func__, ENV_NV_KEY__CONFIG, jsonstrlen);
	if (jsonstrlen == 0) {
		ESP_LOGW(TAG, "Maybe the key \'%s\' is not exist.", ENV_NV_KEY__CONFIG);
		nvs_close(handle);
		return -1;
	}

	char* jsonstr = (char*) calloc(1, jsonstrlen);
	if (jsonstr == NULL) {
		ESP_LOGE(TAG, "%s: memory alloc error", __func__);
		nvs_close(handle);
		return -1;
	}

	err = nvs_get_str(handle, ENV_NV_KEY__CONFIG, jsonstr, &jsonstrlen);
	nvs_close(handle);

	obj->jsonobj = cJSON_Parse(jsonstr);

	return err == ESP_OK ? 0 : -1;
}


static int _save(void *self)
{
	struct env_obj* obj = self;
	if (!obj)
		return -1;

	if (!obj->dirty)
		return 0;

	nvs_handle_t handle;
	esp_err_t err;

	err = nvs_open(ENV_NV_NAMESPACE__USING_CONFIG, NVS_READWRITE, &handle);
	if (err != ESP_OK) {
		/* return err; */
		return -1;
	}

	char* jsonstr = cJSON_PrintUnformatted(obj->jsonobj);
	if (!jsonstr) {
		nvs_close(handle);
		return -1;
	}

	err = nvs_set_str(handle, ENV_NV_KEY__CONFIG, jsonstr);
	if (err != ESP_OK) {
		free(jsonstr);
		nvs_close(handle);
		/* return err; */
		return -1;
	}

	err = nvs_commit(handle);
	if (err != ESP_OK) {
		free(jsonstr);
		nvs_close(handle);
		/* return err; */
		return -1;
	}

	obj->dirty = false;

	free(jsonstr);
	nvs_close(handle);
	return 0 /* ESP_OK */;
}


static int _reset(void *self)
{
	return (_from_factory_config(self) == ESP_OK) ? 0 : -1;
}
