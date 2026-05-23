# Building an Amber Electric Energy Orb with Arduino

**A glowing glass orb that changes colour based on real-time electricity prices.**

I built a ambient energy display using an Arduino UNO R4 WiFi and a WS2812B LED strip stuffed inside a frosted glass sphere. It connects to the Amber Electric API and shifts from green (cheap/renewable) through yellow and orange to angry red during price spikes. The whole thing has a slow lava-lamp effect so it feels alive. Here's how I built it, step by step.

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
- Arduino IDE 2.x (download from arduino.cc/en/software)
- Arduino UNO R4 Boards package (install via Boards Manager in the IDE)
- Adafruit NeoPixel library (install via Library Manager in the IDE)
- Amber Electric account with API key (under Settings → Developer in the Amber app)

---

## Step 1: Get the Arduino working

Plug the Arduino into your computer via USB-C. In the Arduino IDE, install the board support: Tools → Board → Boards Manager, search "UNO R4", install the Arduino UNO R4 Boards package. Select your board under Tools → Board → Arduino UNO R4 WiFi, and pick the port under Tools → Port.

Test with the built-in blink example: File → Examples → 01.Basics → Blink. Upload it. If the small LED on the board blinks, you're good.

**Troubleshooting uploads:** If uploads fail with "No device found", unplug the USB cable and plug it back in. If you're using a USB hub or dock, try connecting directly to your computer — hubs can interfere with the board's bootloader mode. You can also try double-tapping the reset button on the board to enter bootloader mode, then uploading immediately.

---

## Step 2: Connect to WiFi

The UNO R4 WiFi has built-in wireless (2.4GHz only, not 5GHz). Create a `config.h` file in your sketch folder to keep your credentials separate:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID      "your_wifi_name"
#define WIFI_PASSWORD   "your_password"
#define AMBER_API_KEY   "your_amber_api_key"
#define AMBER_SITE_ID   "your_site_id"

#endif
```

Test the WiFi connection — you should see your IP address printed in the Serial Monitor (Tools → Serial Monitor, set baud rate to 115200).

---

## Step 3: Get your Amber Site ID

Before you can fetch prices, you need your Site ID. Make a one-off API call to `GET /v1/sites` with your API key as a Bearer token. The response includes your site ID, network, and channel info. Save the site ID to your `config.h`.

---

## Step 4: Wire up the LED strip

Cut 20 LEDs from the DIN (data in) end of the strip. Look for the arrows printed on the strip — they point in the direction data flows. The DIN end is where arrows originate from. Cut on the copper pad marks between LEDs.

Wiring is just three connections:

| Strip wire | Arduino pin |
|-----------|-------------|
| Red (5V)  | 5V          |
| White (GND) | GND       |
| Green (DIN) | Pin 6     |

Push solid core wire into the strip's connector plug and into the Arduino's pin headers.

**Power considerations:** Each WS2812B LED draws up to 60mA at full white brightness. At moderate brightness (50/255) with 20 LEDs, you'll draw roughly 200-300mA which is well within the Arduino's voltage regulator capacity when powered via the 9V barrel jack (~900mA max). Don't try to run 100+ LEDs at full brightness through the Arduino — you'd need a separate 5V power supply for that.

---

## Step 5: Build the orb

I made a ~5cm diameter ring from a strip of plastic, wrapped the 20 LEDs around it in two coils of 10 (LEDs facing outward), and placed it inside the frosted glass sphere. The ring being a few centimetres smaller than the sphere gives a nice even glow since the light has room to diffuse. A loose bunch of LEDs also works — the frosted glass is very forgiving.

---

## Step 6: The code

The project is modular — each feature is in its own file pair (.h and .cpp):

```
AmberOrb/
  AmberOrb.ino        ← Main sketch
  config.h             ← Your secrets (gitignored)
  config.example.h     ← Template for others
  amber_api.h/.cpp     ← Fetches price & renewables from Amber
  lava_lamp.h/.cpp     ← WS2812B lava lamp effect
  matrix_sun.h/.cpp    ← LED matrix sun animation (optional)
  matrix_digits.h/.cpp ← LED matrix price/% display (optional)
  traffic_leds.h/.cpp  ← Individual LED traffic light (optional)
```

Features are toggled with `#define` lines at the top of the main sketch:

```cpp
#define USE_LAVA_LAMP        // WS2812B strip
//#define USE_MATRIX_SUN     // LED matrix sun
//#define USE_MATRIX_DIGITS  // LED matrix digits
//#define USE_TRAFFIC_LEDS   // Traffic light LEDs
```

### The lava lamp effect

The core visual effect uses layered sine waves to create organic brightness variation across the LEDs:

```cpp
float wave1 = sin(lavaTick * 0.002 + i * 0.6);
float wave2 = sin(lavaTick * 0.003 + i * 0.4 + 2.0);
float brightness = 0.6 + 0.4 * ((wave1 + wave2) / 2.0);
```

Each LED drifts on its own slow rhythm. The multipliers on `i` mean neighbouring LEDs are similar but not identical, creating wide "blobs" of brightness that drift around the strip. Through frosted glass it looks like a lava lamp.

### Price-to-colour mapping

The base colour smoothly transitions based on the current electricity price:

| Price         | Colour      |
|--------------|-------------|
| ≤ 26 c/kWh  | Green       |
| 26-33 c/kWh | Yellow      |
| 33-40 c/kWh | Orange      |
| 40-50 c/kWh | Red         |
| > 50 c/kWh  | Dark red    |

During price spikes (>50c), the brightness also ramps up from 30 to 200, so the orb gets visibly angry and bright — you'll notice it from across the room.

### API integration

The sketch calls the Amber API endpoint `GET /v1/sites/{siteId}/prices/current` every 5 minutes over HTTPS. The response includes `perKwh` (price in cents) and `renewables` (percentage of renewables in the grid). JSON parsing is done with simple string searching rather than a library, to save memory on the Arduino.

---

## Optional extras we built along the way

### LED matrix sun animation
A 7x7 pixel sun on the built-in matrix that grows from a tiny dot (0% renewables) to a full starburst (100% renewables). Eight stages, smoothly animated.

### LED matrix digit display
A custom 3x7 pixel font that displays the price as "X.XX" and renewables as "XX%" with a tiny pixel-art percent symbol. The three displays (sun, price, percentage) cycle every 4 seconds.

### Traffic light LEDs
Three groups of coloured LEDs (red, yellow, green) wired through 220Ω resistors to PWM pins. Shows renewables status at a glance with adjustable brightness via `analogWrite`.

---

## Running it 24/7

Plug the 9V adaptor into the barrel jack and the orb runs indefinitely. The Arduino draws a few watts total. The `millis()` timer handles overflow correctly so there are no issues after 49 days. If WiFi drops, it reconnects automatically. The API fetch runs every 5 minutes.

To turn it off, just unplug it.

---

## For Git users

Add `config.h` to your `.gitignore` and commit `config.example.h` as a template:

```
config.h
```

---

## What I learned

- Arduino sketches are just C++ files in a folder. Multiple `.h` and `.cpp` files in the same folder are automatically compiled together.
- `#define` in header files avoids the "multiple definition" linker errors you get with `const` variables included from multiple files.
- The UNO R4's built-in LED matrix is red-only (not RGB), 12 columns by 8 rows.
- WS2812B LEDs are individually addressable RGB — each LED can be any colour, controlled through a single data wire.
- Serial is for talking to your computer over USB (debugging). I2C and SPI are for talking to sensors and displays. WiFi is for talking to the internet.
- The Arduino's voltage regulator converts 9V from the barrel jack down to 5V. It can supply about 900mA to external devices.
- PWM (pulse width modulation) fakes analog output by flickering a digital pin very fast — great for dimming LEDs.
- Breadboard rows of 5 holes (a-e) are connected internally. The gap in the middle separates left from right. Power rails run the full length.
- Layered sine waves at slightly different frequencies create organic, lava-lamp-like animations.
- USB hubs and docks can interfere with Arduino uploads — plug directly into your computer if you have issues.

---

## What's next

- Add more LED coils inside the orb for a denser, more even glow
- Log price data to the SD card module for historical analysis
- Add an OLED display for crisp text readout of price and renewables
- Build a web dashboard that reads from the same API
- Use the Amber API's forecast data to predict price changes and shift the colour ahead of time
