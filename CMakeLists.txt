# Behaviors run on the central (or a non-split keyboard), never on peripherals.
if(CONFIG_ZMK_BEHAVIOR_PUSHBUTTON AND ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL))
    target_sources(app PRIVATE src/behaviors/behavior_pushbutton.c)
endif()
