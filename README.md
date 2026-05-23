# AmberOrb

A light-up display orb for the Arduino Uno R4 WiFi that shows real-time energy pricing from the [Amber Electric](https://www.amber.com.au/) API. Helps you use cheap renewable energy by making the current price obvious at a glance.

## What it does

The orb changes colour based on your electricity price:

| Price | Colour | Meaning |
|-------|--------|---------|
| < 27c/kWh | Green | Cheap - use energy freely |
| 27-36c/kWh | Yellow | Moderate |
| 37-47c/kWh | Orange | Getting pricey |
| 48-99c/kWh | Red | Expensive |
| $1+/kWh | Bright Red | Very expensive (brightness cranked up) |

## Hardware

- Arduino Uno R4 WiFi
- WS2812B LED strip (20 LEDs on pin 6)
- Optional: 8x12 LED matrix (built into R4 WiFi) for sun animation / price digits
- Optional: Individual red/yellow/green LEDs for renewables percentage

## Project structure

```
AmberOrb/
  platformio.ini              # Build config (board, libs, feature flags)
  upload.sh                   # Upload script (uses arduino-cli)
  src/
    main.cpp                  # Main application
    test_lava_colors.cpp      # On-device colour integration test
  lib/
    lava_lamp/                # WS2812B lava lamp effect (price -> colour)
    amber_api/                # Amber Electric API client
    matrix_sun/               # LED matrix sun animation (renewables)
    matrix_digits/            # LED matrix price/percentage display
    traffic_leds/             # Simple red/yellow/green LEDs (renewables)
  include/
    config.h                  # Your secrets (gitignored)
    config.example.h          # Template - copy to config.h and fill in
  test/
    test_lava_unit/           # Unit tests (run on your laptop, no hardware)
```

## Setup

### Prerequisites

- [PlatformIO CLI](https://platformio.org/) - for compiling and testing
- [arduino-cli](https://arduino.github.io/arduino-cli/) - for uploading to the board

```bash
uv tool install platformio
brew install arduino-cli
arduino-cli core install arduino:renesas_uno
```

### Configuration

Copy the example config and fill in your details:

```bash
cp include/config.example.h include/config.h
```

Edit `include/config.h` with your WiFi credentials and [Amber API key](https://app.amber.com.au/developers/).

### Enabling modules

Module selection is in `platformio.ini` under `build_flags`. Uncomment the ones you want:

```ini
build_flags =
    -I include
    -D USE_LAVA_LAMP
    ; -D USE_MATRIX_SUN
    ; -D USE_MATRIX_DIGITS
    ; -D USE_TRAFFIC_LEDS
```

## Commands

| Command | What it does |
|---------|-------------|
| `pio run` | Compile |
| `pio run -t upload` | Compile and flash to board |
| `pio test -e native` | Run unit tests (on your laptop, no hardware needed) |
| `pio run -e test_colors -t upload` | Flash the colour test sketch |
| `pio device monitor` | Serial monitor (run in a separate terminal) |

### Typical workflow

```bash
# Edit code, then compile + flash
pio run -t upload

# Watch serial output in another terminal
pio device monitor

# Run unit tests after changing colour logic
pio test -e native
```

### Testing the colours

To visually validate that each price band shows the right colour on the LED strip:

```bash
# Upload the colour test sketch
pio run -e test_colors -t upload

# Watch serial output to see price -> RGB mappings
pio device monitor

# When done, flash the main sketch back
pio run -t upload
```

## Why PlatformIO + arduino-cli?

PlatformIO handles compiling, library management, and testing brilliantly. However, the Uno R4 WiFi has a [known upload issue](https://github.com/arduino/ArduinoCore-renesas/issues/73) where the board re-enumerates on a different USB port during upload. `arduino-cli` handles this correctly (same logic as Arduino IDE), so we use it for uploads via a custom `upload.sh` script. This is transparent - `pio run -t upload` calls arduino-cli under the hood.
