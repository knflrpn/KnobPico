# KnobPico

KnobPico is a Raspberry Pi Pico firmware project for KNOB (https://baselinedesign.tech/products/knobv21) that turns the rotary knob into a scroll wheel and the three buttons into a macro keypad.

# Configuration

Open configurator.html in a Chromium-based browser (Chrome / Edge). It uses WebHID to connect directly to the device and read/write the config.

You can configure:

- Debounce and long-press threshold
- Scroll divider and direction
- Macros for each button (short + long press)
- LED press colors and idle sequences

Default button mappings:

- Left short: Ctrl+X
- Middle short: Ctrl+C
- Right short: Ctrl+V

Long-press macros are empty by default.