#include "env_device.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static const char* TAG = "ENV_GOLFZONE";
static const char* filename_golfzone = "/envconf/golfzone.json";


static int _load(void *self);
static int _save(void *self);
static int _reset(void *self);


static struct env_obj_funcs _fs_golfzone_funcs = {
	.load_fp = _load,
	.save_fp = _save,
	.reset_fp = _reset,
};


static struct env_obj* _golfzone_obj;

static const char* _factory_golfzone = "{}";


struct env_obj* env_golfzone()
{
	if (!_golfzone_obj) {
		struct env_obj* obj;
		obj = env_obj_create(&_fs_golfzone_funcs);
		assert(obj);
		obj->readonly = false;
		_golfzone_obj = obj;
	}

	return _golfzone_obj;
}


void env_golfzone_delete()
{
	if (_golfzone_obj) {
		env_obj_delete(_golfzone_obj);
		_golfzone_obj = NULL;
	}
}


static esp_err_t _fs_factory_golfzone_set(void *self)
{
	struct env_obj* obj = self;
	if (!obj)
		return ESP_FAIL;

	cJSON* jsonobj = cJSON_Parse(_factory_golfzone);
	if (!jsonobj) {
		return ESP_FAIL;
	}

	cJSON_Delete(obj->jsonobj);

	obj->jsonobj = jsonobj;

	obj->funcs->save_fp(obj);

	return ESP_OK;
}

static int _load(void *self)
{
	struct env_obj* obj = self;
	if (!obj)
		return -1;

	FILE* f = fopen(filename_golfzone, "r");
	if (!f) {
		/* plan B */
		printf("%s: using plan B\n", __func__);
		obj->jsonobj = cJSON_CreateObject();
		obj->readonly = false;
		obj->dirty = true;
		obj->funcs->save_fp(obj);
		return 0;
	}

	struct stat status;
	stat(filename_golfzone, &status);
	printf("%s: json file size: %ld\r\n", __func__, status.st_size);

	char* jsonstr = (char*) calloc(1, status.st_size);
	if (jsonstr == NULL) {
		ESP_LOGE(TAG, "%s: memory alloc error", __func__);
		fclose(f);
		return -1;
	}

	size_t count = fread(jsonstr, status.st_size, 1, f);
	fclose(f);

	obj->jsonobj = cJSON_Parse(jsonstr);
	obj->readonly = false;

	return 0;
}


static int _save(void *self)
{
	struct env_obj* obj = self;
	if (!obj)
		return -1;

	if (!obj->dirty)
		return 0;

	char* jsonstr = cJSON_PrintUnformatted(obj->jsonobj);
	if (!jsonstr) {
		return -1;
	}

	size_t written;
	FILE* f;

	f = fopen(filename_golfzone, "w");
	if (!f) {
		perror("fopen");
		return -1;
	}

	written = fwrite(jsonstr, strlen(jsonstr), 1, f);

	fclose(f);

	return written;
}


static int _reset(void *self)
{
	return (_fs_factory_golfzone_set(self) == ESP_OK) ? 0 : -1;
}
