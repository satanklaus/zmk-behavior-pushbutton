target_include_directories(app PRIVATE include)

if (CONFIG_ZMK_SPLIT AND CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
	target_sources(app PRIVATE src/behaviors/behavior_pushbutton.c)
endif()
