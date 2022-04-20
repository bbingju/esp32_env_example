#pragma once

#include "app.h"
#include "cJSON_Utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


struct env_obj_funcs {
	int (*load_fp)(void* self);
	int (*save_fp)(void* self);
	int (*reset_fp)(void* self);
};

struct env_obj {
	SemaphoreHandle_t lock;
	bool dirty;
	cJSON* jsonobj;

	bool readonly;
	struct env_obj_funcs* funcs;
};

struct env_obj* env_obj_create(struct env_obj_funcs* funcs);
void env_obj_delete(struct env_obj* obj);

cJSON* env_obj_get(struct env_obj* obj, const char* path);
int env_obj_set(struct env_obj* obj, const char* path, cJSON* value);
void env_obj_commit(struct env_obj* obj);
int env_obj_factory_reset(struct env_obj* obj);

void env_obj_print(struct env_obj* obj, const char* path);
