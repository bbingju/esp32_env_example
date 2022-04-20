#pragma once

#include "env.h"

struct env_obj* env_config();
void env_config_delete();

/*
 * helper functions
 */
const char* env_config_get__wifi_ssid(int index);
const char* env_config_get__wifi_password(int index);
