# Building an Amber Electric Energy Orb with Arduino

**A glowing glass orb that changes colour based on real-time electricity prices.**

I built an ambient energy display using an Arduino UNO R4 WiFi and a WS2812B LED strip stuffed inside a frosted glass sphere. It connects to the Amber Electric API and shifts from green (cheap/renewable) through yellow and orange to angry red during price spikes — complete with a fast trailing comet animation when prices really spike. The whole thing has a slow floating glow effect so it feels alive. Here's how I built it, step by step.

---

## What you need

**Hardware:**
- Arduino UNO R4 WiFi (has built-in WiFi and a 12x8 LED matrix)
- WS2812B LED strip (I used 20 LEDs cut from a 2-metre, 60 LEDs/m strip)
- 12cm frosted glass sphere lampshade (mine was from a lighting shop)
- USB-C cable (for programming)
- 9V DC mains adaptor (for permanent power, plugs into the Arduino's barrel jack)
- Solid core wire and wire cutters
- Electrical tape

**Software & accounts:**
- [PlatformIO](https://platformio.org/) — build system, library management, and testing
- [arduino-cli](https://arduino.github.io/arduino-cli/) — for uploading to the board (handles R4 WiFi's USB quirks)
- Amber Electric account with API key (under Settings → Developer in the Amber app)

```bash
uv tool install platformio
brew install arduino-cli
arduino-cli core install arduino:renesas_uno
```

---

## Step 1: Get the Arduino working

Plug the Arduino into your computer via USB-C. Test with a simple blink sketch to confirm uploads work.

**Troubleshooting uploads:** The R4 WiFi has a known quirk where it re-enumerates on a different USB port during upload. If uploads fail with "No device found", just retry — the project's `upload.sh` script handles this using `arduino-cli`. If you're using a USB hub, try connecting directly.

---

## Step 2: Connect to WiFi

The UNO R4 WiFi has built-in wireless (2.4GHz only, not 5GHz). Create your config file:

```bash
cp include/config.example.h include/config.h
```

Fill in your credentials:

```cpp
#define WIFI_SSID      "your_wifi_name"
#define WIFI_PASSWORD   "your_password"
#define AMBER_API_KEY   "your_amber_api_key"
#define AMBER_SITE_ID   "your_site_id"
```

---

## Step 3: Get your Amber Site ID

Make a one-off API call to `GET /v1/sites` with your API key as a Bearer token. The response includes your site ID. Save it to your `config.h`.

---

## Step 4: Wire up the LED strip

Cut 20 LEDs from the DIN (data in) end of the strip. Look for the arrows printed on the strip — they point in the direction data flows.

Wiring is just three connections:

| Strip wire | Arduino pin |
|-----------|-------------|
| Red (5V)  | 5V          |
| White (GND) | GND       |
| Green (DIN) | Pin 6     |

**Power considerations:** At moderate brightness with 20 LEDs, you'll draw roughly 200-300mA which is well within the Arduino's voltage regulator capacity when powered via the 9V barrel jack (~900mA max).

---

## Step 5: Build the orb

I made a ~5cm diameter ring from a strip of plastic, wrapped the 20 LEDs around it in two coils of 10 (LEDs facing outward), and placed it inside the frosted glass sphere. The ring being a few centimetres smaller than the sphere gives a nice even glow since the light has room to diffuse.

---

## Step 6: The code

The project uses PlatformIO with a modular library structure:

```
AmberOrb/
  platformio.ini              # Build config (board, libs, feature flags)
  upload.sh                   # Upload script (uses arduino-cli)
  src/
    main.cpp                  # Main application
    test_lava_colors.cpp      # On-device colour integration test
  lib/
    lava_lamp/                # WS2812B floating glow + comet effect
      lava_colors.h           # Price-to-colour mapping (shared with tests)
      lava_lamp.h/.cpp        # Animation engine
    amber_api/                # Amber Electric API client (ArduinoJson)
    matrix_sun/               # LED matrix sun animation (optional)
    matrix_digits/            # LED matrix price/percentage display (optional)
    traffic_leds/             # Simple red/yellow/green LEDs (optional)
  include/
    config.h                  # Your secrets (gitignored)
  test/
    test_lava_unit/           # Unit tests (run on your laptop, no hardware)
```

Features are toggled in `platformio.ini`:

```ini
build_flags =
    -D USE_LAVA_LAMP
    ; -D USE_MATRIX_SUN
    ; -D USE_MATRIX_DIGITS
    ; -D USE_TRAFFIC_LEDS
```

### The floating glow effect

The core visual is a Gaussian blob that drifts smoothly back and forth along the strip:

```cpp
float glowPos = (NUM_LEDS / 2.0) + sin(phase) * (NUM_LEDS / 2.0 - 1.0);

for (int i = 0; i < NUM_LEDS; i++) {
  float dist = fabs(glowPos - i);
  float brightness = exp(-(dist * dist) / (2.0 * spread * spread));
  // ...
}
```

The center of the glow follows a sine wave, and each LED's brightness falls off with a Gaussian curve based on its distance from the center. Through frosted glass it looks like a glowing blob slowly floating inside the orb. LEDs below 10% brightness are cut off entirely to avoid colour distortion at low intensities (integer truncation shifts yellow toward red).

### Price spike comet

When prices hit the spike threshold (57c+), the animation switches to a fast trailing comet:

```cpp
cometPos += 0.05 * speed;
// Fade all pixels toward black each frame (creates the trail)
for (int i = 0; i < NUM_LEDS; i++) {
  r = (int)(r * 0.88);  // 12% fade per frame
  // ...
}
// Paint the comet head at full brightness
strip.setPixelColor(head, strip.Color(c.r, c.g, c.b));
```

The comet races around the strip leaving a fading red trail. It's impossible to ignore — which is exactly the point during a price spike.

### Price-to-colour mapping

Prices are rounded to the nearest cent (matching the Amber app's display) before selecting a band:

| Price | Colour | Animation |
|-------|--------|-----------|
| < 27c/kWh | Green | Floating glow |
| 27-36c/kWh | Yellow | Floating glow |
| 37-46c/kWh | Orange | Floating glow |
| 47-56c/kWh | Red | Floating glow |
| 57c+/kWh | Red | Fast trailing comet |

The colour logic lives in a single shared header (`lava_colors.h`) that's used by both the firmware and the unit tests — one source of truth.

### API integration

The sketch calls `GET /v1/sites/{siteId}/prices/current` over HTTPS. The response is parsed with [ArduinoJson](https://arduinojson.org/):

```cpp
JsonDocument doc;
deserializeJson(doc, body);
for (JsonObject obj : doc.as<JsonArray>()) {
  if (obj["channelType"] == "general") {
    currentPrice = obj["perKwh"] / 100.0;
    currentRenewables = obj["renewables"];
  }
}
```

In production, fetches are aligned to half-hour boundaries via NTP (Amber prices update every 30 minutes). If the first fetch fails, it retries every 10 seconds until it gets data — the LEDs stay off until then so you never see a misleading green.

---

## Dev mode

The project has a dedicated dev environment for debugging:

```bash
pio run -e dev -t upload    # fetches every 60s, verbose logging
pio device monitor          # in a separate terminal
```

Output looks like this:

```
[00:00:05] DEV_MODE active — fetching every 60s
[00:00:22] Fetching...
[API] OK: 27.7858c/kWh ($0.277858)  Renewables: 16.3%
[00:00:25] [COLOR] $0.277858 (27.7858c) -> R=255 G=150 B=0 bright=30 spd=1.00
[00:00:30] [PERF] work=2096us  frame=15ms  cpu=13%
[00:00:40] [PERF] work=2040us  frame=15ms  cpu=13%
[00:01:05] Fetching...
[API] OK: 27.5003c/kWh ($0.275003)  Renewables: 16.4%
[00:01:08] [COLOR] $0.275003 (27.5003c) -> R=255 G=150 B=0 bright=30 spd=1.00
```

- **[API]** — fetch results or errors (connection failed, timeout, JSON parse error)
- **[COLOR]** — which price band was selected, with full-precision values for debugging boundary issues
- **[PERF]** — CPU utilisation every 10 seconds (work time vs 15ms frame budget)

This was invaluable for tracking down a bug where the orb would sometimes flash green during a yellow period. Turned out the API returns multiple price channels (general, feed-in) and we were accidentally grabbing the feed-in price. The verbose logging made it obvious.

---

## Unit testing (no hardware needed)

The colour logic is extracted into a pure header file with no Arduino dependencies, so it runs on your laptop:

```bash
pio test -e native
```

```
test_green_at_zero     [PASSED]
test_green_at_10c      [PASSED]
test_yellow_at_27c     [PASSED]
...
14 test cases: 14 succeeded
```

This catches boundary issues (like floating-point rounding causing 26.65c to show green instead of yellow) before you flash to the board.

---

## Optional extras

### LED matrix sun animation
A 7x7 pixel sun on the built-in matrix that grows from a tiny dot (0% renewables) to a full starburst (100% renewables). Eight stages, animating at 400ms per step for a visible transition.

### LED matrix digit display
A custom 3x7 pixel font that displays the price as "X.XX" and renewables as "XX%" with a tiny pixel-art percent symbol. The three displays (sun, price, percentage) cycle every 4 seconds.

### Traffic light LEDs
Three groups of coloured LEDs (red, yellow, green) wired through 220Ω resistors to PWM pins. Shows renewables status at a glance.

---

## Running it 24/7

Plug the 9V adaptor into the barrel jack and the orb runs indefinitely. The Arduino draws a few watts total. If WiFi drops, it reconnects automatically (checks every 5 seconds, 10 second timeout). Prices refresh on the half-hour when Amber publishes new data.

The loop runs at 66fps (15ms frame time) using only ~13% CPU, leaving plenty of headroom. The `millis()` timer handles overflow correctly so there are no issues after 49 days.

---

## What I learned

- **PlatformIO > Arduino IDE** for anything beyond a toy sketch. Library management, multiple build environments (prod, dev, test), and desktop unit testing with zero hardware in the loop.
- **The R4 WiFi upload bug** was the most frustrating issue. PlatformIO's built-in upload uses `bossac` which can't handle the USB port re-enumeration. `arduino-cli` handles it correctly — wrapping it in `upload.sh` with a port-conflict check solved it permanently.
- **Stream-parsing JSON from WiFiSSLClient is unreliable.** The SSL implementation on the R4 can drop data mid-read. Reading the full body first (with `String.reserve()` to avoid heap fragmentation) then parsing is much more robust.
- **Float precision bites at boundaries.** `27.00 / 100.0 * 100.0` can produce `26.999...` in single-precision float. Rounding to the nearest integer cent before comparing bands fixes it.
- **NeoPixels distort colour at low brightness** due to integer truncation. Yellow `{255, 150, 0}` at 1% brightness becomes `{2, 1, 0}` — perceptually red. Cutting off below 10% brightness and going fully dark is cleaner than trying to display inaccurate colours.
- **Dev mode is worth the 10 minutes to set up.** Timestamps + tagged log categories (`[API]`, `[COLOR]`, `[PERF]`) made debugging the "phantom green" bug trivial. Without them I'd have been guessing.
- **Unit tests for embedded code** are surprisingly easy with PlatformIO's native environment. Extract pure logic into header-only files, test on your laptop, flash to hardware with confidence.

---

## What's next

- 3D print a housing for the Arduino and wires
- Add more LED coils inside the orb for a denser, more even glow
- Use the Amber API's forecast data to predict price changes and shift the colour ahead of time
- Add a "bedtime mode" that dims the orb after a certain hour
