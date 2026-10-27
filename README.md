# zmk-behavior-pushbutton

ZMK behavior that drives a GPIO active for a fixed time when its key is pressed,
e.g. to "press" a physical button (PC power/reset switch) through a transistor or
optocoupler wired to a spare pin.

- The pulse length is fixed (`press-duration-ms`); releasing the key early does
  not shorten it, holding it does not lengthen it.
- Pressing again while the pin is still active restarts the pulse.
- Runs on the central (or a non-split keyboard): put the GPIO on that board.
- Use the GPIO flags for polarity (`GPIO_ACTIVE_LOW` for an active-low driver).

## Usage

Add the module (ZMK_EXTRA_MODULES or west.yml), then define an instance and bind it:

```dts
#include <dt-bindings/gpio/gpio.h>

/ {
    behaviors {
        pwr: power_button {
            compatible = "zmk,behavior-pushbutton";
            #binding-cells = <0>;
            gpios = <&gpio0 9 GPIO_ACTIVE_HIGH>;
            press-duration-ms = <300>;   /* default 200 */
        };
    };

    keymap {
        /* ... */ &pwr /* ... */
    };
};
```

## Tests

`tests/pushbutton` holds ZMK native_sim tests (emulated GPIO, mock kscan):
tap, hold longer than the pulse, retrigger during the pulse, and two instances
(one active-low) running alongside a normal key. From `zmk/app`:

```sh
ZMK_EXTRA_MODULES=/path/to/zmk-behavior-pushbutton \
    ./run-test.sh /path/to/zmk-behavior-pushbutton/tests/pushbutton
```
