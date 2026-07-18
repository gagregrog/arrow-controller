# Arrow Controller

ESP32-based NFC controller that triggers quickplay requests on the Arrow server.

## Cloning

This repo uses submodules. Clone with:

```
git clone --recurse-submodules <url>
```

If you already cloned without submodules:

```
git submodule update --init
```

## Building

This is a PlatformIO project. Open in VS Code with the PlatformIO extension, or use the CLI:

```
pio run
pio run -t upload
```

## Hardware

- ESP32 DevKitC
- Adafruit PN532 NFC reader (I2C: SDA=GPIO21, SCL=GPIO22)
- NFC interrupt: GPIO16 (active-low, wired to PN532 IRQ)
- NFC reset: not wired (PN532 uses power-on reset)

### Physical Buttons

Four momentary buttons, each wired between the GPIO pin and GND (active low — internal pull-ups enabled):

| Button   | GPIO | Short press                                                 | Long press (≥1.5s)                                    |
| -------- | ---- | ----------------------------------------------------------- | ----------------------------------------------------- |
| Play     | 18   | Pause if playing, resume if paused, else start quickplay #0 | Shuffle all                                           |
| Stop     | 19   | Stop and switch the receiver to the TV input                | Stop, switch to TV input, then power off the receiver |
| Previous | 26   | Previous track                                              | Volume down (3 increments)                            |
| Next     | 23   | Next track                                                  | Volume up (3 increments)                              |

The play/pause-vs-quickplay decision is made server-side by `POST /play`. No external resistors needed — internal pull-ups enabled.

Short presses fire on release; long presses fire as soon as the 1.5s threshold is crossed (button still held).

### Status LEDs

WS2811 LED strip — 12 LEDs, data line on GPIO17.

**Persistent states** (override all other indicators):

| State             | Pattern          |
| ----------------- | ---------------- |
| WiFi lost         | Breathing orange |
| WiFi setup portal | Breathing blue   |
| Scan cooldown     | Breathing purple |

**Event flashes** (only shown when WiFi is connected):

| Event                   | Pattern           |
| ----------------------- | ----------------- |
| Badge scanned (known)   | Green, 4 flashes  |
| Badge scanned (unknown) | Pink, 2 flashes   |
| Previous / Next track   | Blue, 2 flashes   |
| Volume up / down        | Blue, 2 flashes   |
| Play / Shuffle          | Purple, 2 flashes |
| Stop                    | Red, 2 flashes    |
| Command failed          | Orange, 6 flashes |

Max brightness is set by `LED_MAX_BRIGHTNESS` in `Leds.cpp` (0–255, default 32).

### Scan cooldown

After a badge triggers playback, further scans are ignored for a fixed window (`SCAN_COOLDOWN_MS` in `main.cpp`, default 10s) so a quickplay can't be interrupted the moment it starts. Scanning any badge during the window does nothing except show breathing purple until the window ends. The window is anchored to the accepted scan and is not extended by scans made during it. (This is separate from the PN532's short same-tag debounce in `NFC.cpp`.)

### WiFi reconnect

If the WiFi link drops after boot, the controller keeps trying to restore it instead of sitting idle. The core's auto-reconnect is enabled as a first line of defense; on top of that, `wifiReconnectLoop()` triggers a `WiFi.reconnect()` immediately when a drop is detected and then retries every `WIFI_RECONNECT_INTERVAL_MS` (in `WiFiConn.cpp`, default 10s) until the link is back. While disconnected the LEDs show breathing orange (see "WiFi lost" above).

## Setup

### `.env` file

Create a `.env` file at the project root (gitignored). Required keys:

```
OTA_PASSWORD=yourpassword
IP=192.168.x.x        # ESP32 IP for OTA uploads
TZ_STRING=PST8PDT,M3.2.0,M11.1.0
```

On first boot, the controller creates a WiFi access point named **ArrowController**. Connect to it and enter your WiFi credentials. The controller will remember them on subsequent boots.

## OTA Updates

Use the `esp32ota` environment to flash over the network:

```
pio run -e esp32ota -t upload
```

Requires `OTA_PASSWORD` and `IP` in `.env`.

## Badge Registration

Badges are registered at runtime via the built-in web UI at `http://arrow-controller.local`.

- Scan an unregistered badge → a modal appears prompting you to register it as the next slot
- Registered badges are shown alongside their corresponding quickplay entry
- The "Link Music" modal is multi-select: tap artists/albums to add them to the selection tray, then Save to map them all to the slot. The selections play sequentially when the badge is scanned.
- Badges are stored persistently in NVS and survive reboots

The web UI fetches the current quickplay list from the Arrow server (`http://arrow.local:8000/quickplay`) to show which selections are mapped to each slot (the first item, plus a "+N more" count for multi-selection entries).

## Settings

A cog in the top-right corner opens a **Settings** modal that manages everything on the Pi from the browser:

- **Stereo** — check the live power-sensor status, and **Configure Sensor**: an editable form of every `stereo_sensor` value (enabled, I²C address, channel, gain, on/off thresholds). Each threshold has a **Tune** button that samples the sensor and fills in a suggested value; **Save** (enabled only when something changed) writes the whole block back to the Pi.
- **Quickplay** — clear the quickplay list (behind a confirmation).
- **System** — reboot or shut down the Raspberry Pi, each behind a confirmation dialog.

## REST API

All `/api/*` routes below the badge endpoints proxy to the Arrow server.

| Method | Path                  | Description                                                            |
| ------ | --------------------- | ---------------------------------------------------------------------- |
| GET    | `/api/badges`         | List registered badge UIDs                                             |
| POST   | `/api/badges`         | Register a new badge `{"uid":"AA:BB:CC:DD"}`                           |
| DELETE | `/api/badges/{index}` | Remove badge at index                                                  |
| GET    | `/api/quickplay`      | Proxied quickplay list from Arrow server                               |
| GET    | `/api/ir`             | Proxied IR function list from Arrow server — returns `[{name, class}]` |
| POST   | `/api/ir/{function}`  | Send a named IR command via Arrow server                               |
| GET    | `/api/stereo`         | Proxied stereo power status — `{"on":bool\|null,"voltage":float\|null,"sensor_enabled":bool}` |
| GET    | `/api/stereo/config`  | Proxied current sensor config for the settings form                   |
| PUT    | `/api/stereo/config`  | Proxied sensor config save (enabled/address/channel/gain/thresholds)  |
| POST   | `/api/stereo/sample`  | Proxied sensor sample burst for tuning — returns stats, writes nothing |
| POST   | `/api/system/reboot`  | Reboot the Raspberry Pi                                                |
| POST   | `/api/system/shutdown`| Shut down the Raspberry Pi                                             |

## WebSocket

Connect to `ws://arrow-controller.local/ws` to receive real-time badge events:

| Event                 | Payload                                        |
| --------------------- | ---------------------------------------------- |
| Known badge scanned   | `{"type":"badge_scan","index":N}`              |
| Unknown badge scanned | `{"type":"unknown_badge","uid":"AA:BB:CC:DD"}` |

## How It Works

1. Arms the PN532 for passive target detection; GPIO16 IRQ fires when a tag is present
2. Looks up the tag UID in NVS-backed badge store
3. POSTs to `http://arrow.local:8000/quickplay/{index}` when a known tag is detected; notifies WebSocket clients
4. Unknown tags are reported over WebSocket for registration via the web UI
5. A 3-second cooldown prevents duplicate triggers from the same tag
