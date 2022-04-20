#include "env.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <stdlib.h>
#include <string.h>


static const char* TAG = "ENV_OBJ";


#define _LOCK(obj) xSemaphoreTake((obj)->lock, (TickType_t)50);
#define _UNLOCK(obj) xSemaphoreGive((obj)->lock)


struct env_obj* env_obj_create(struct env_obj_funcs* funcs)
{
	struct env_obj* obj = NULL;

	obj = calloc(1, sizeof(struct env_obj));
	if (!obj)
		return NULL;

	obj->lock = xSemaphoreCreateMutex();
	obj->funcs = funcs;

	obj->funcs->load_fp(obj);

	return obj;
}


void env_obj_delete(struct env_obj* obj)
{
	if (obj) {
		if (obj->jsonobj)
			cJSON_Delete(obj->jsonobj);
		vSemaphoreDelete(obj->lock);
		free(obj);
	}
}


cJSON* env_obj_get(struct env_obj* obj, const char* path)
{
	if (!obj || !path)
		return NULL;
	return cJSONUtils_GetPointerCaseSensitive(obj->jsonobj, path);
}


int env_obj_set(struct env_obj* obj, const char* path, cJSON* value)
{
	if (!obj || !path || !value)
		return -1;

	if (obj->readonly) {
		ESP_LOGW(TAG, "error: this object is readonly.");
		return 0;
	}

	cJSON* from = cJSONUtils_GetPointerCaseSensitive(obj->jsonobj, path);
	if (!from) {
		return -1;
	}

	if (!cJSON_IsBool(value) && (from->type != value->type)) {
		ESP_LOGE(TAG, "%s: value type is not matched (%d != %d)",
				 __func__, from->type, value->type);
		return -1;
	}

	_LOCK(obj);

	if (cJSON_IsNumber(value)) {
		cJSON_SetNumberValue(from, value->valuedouble);
	} else if (cJSON_IsString(value)) {
		if (from->valuestring) {
			cJSON_free(from->valuestring);
		}
		from->valuestring = strdup(value->valuestring);
	} else if (cJSON_IsBool(value)) {
		from->type = value->type;
	} else if (cJSON_IsObject(value)) {

		cJSON* velem = NULL;
		cJSON_ArrayForEach(velem, value)
		{
			cJSON* felem = cJSON_GetObjectItem(from, velem->string);
			if (felem) {
				cJSON_ReplaceItemInObjectCaseSensitive(from, velem->string, cJSON_Duplicate(velem, true));
			}
		}
	} else {
		_UNLOCK(obj);
		return -1;
	}

	obj->dirty = true;

	_UNLOCK(obj);

	return 0;
}


void env_obj_commit(struct env_obj* obj)
{
	if (!obj || !obj->jsonobj)
		return;

	if (obj->readonly)
		return;

	if (obj->dirty) {
		_LOCK(obj);
		int ret = obj->funcs->save_fp(obj);
		if (!ret) {
			ESP_LOGE(TAG, "error: could not save the jsonobj.");
			_UNLOCK(obj);
			return;
		}
		obj->dirty = false;
		_UNLOCK(obj);
	}
}


int env_obj_factory_reset(struct env_obj* obj)
{
	int err = -1;

	if (!obj || obj->readonly)
		return -1;

	_LOCK(obj);
	err = obj->funcs->reset_fp(obj);
	if (err != 0) {
		_UNLOCK(obj);
		return err;
	}
	err = obj->funcs->save_fp(obj);
	obj->dirty = false;
	_UNLOCK(obj);

	return err;
}


void env_obj_print(struct env_obj* obj, const char* path)
{
	if (!obj || !path)
		return;

	cJSON* toprint = cJSONUtils_GetPointerCaseSensitive(obj->jsonobj, path);
	if (!toprint) {
		return;
	}

	ESP_LOGI(TAG, "%s", cJSON_Print(toprint));
}
