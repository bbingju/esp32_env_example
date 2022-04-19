set(srcs "app.c" "new_env_main.c")

set(requires json nvs_flash spiffs)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
		    REQUIRES "${requires}")
