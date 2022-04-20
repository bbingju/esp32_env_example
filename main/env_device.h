#pragma once

#include "env.h"

struct env_obj* env_device();
void env_device_delete();

/*
 * helper functions
 */
const char* env_device_get__base_mac();
uint32_t env_device_get__gw_id();
