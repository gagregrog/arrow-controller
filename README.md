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

| Button         | GPIO |
| -------------- | ---- |
| Play           | 23   |
| Stop           | 5    |
| Previous       | 19   |
| Next           | 26   |
| Restart track  | 18   |
| Restart Mopidy | 13   |

No external resistors needed — internal pull-ups enabled.

## Setup

On first boot, the controller creates a WiFi access point named **ArrowController**. Connect to it and enter your WiFi credentials. The controller will remember them on subsequent boots.

## Tag Map

NFC tag-to-badge mappings live in `tags.csv` (gitignored — never committed). Copy `tags.csv.example` to get started:

```
cp tags.csv.example tags.csv
```

Each line is a tag UID; badge ID is assigned by line position starting at 1:

```
DE:AD:BE:EF:12:34:56
AB:CD:EF:01:23:45
```

The UID is printed to serial on every first scan of an unknown tag. `include/TagMap.h` is auto-generated from `tags.csv` at build time — do not edit it directly.

## How It Works

1. Arms the PN532 for passive target detection; GPIO16 IRQ fires when a tag is present
2. Looks up the tag UID in the tag map
3. POSTs to `http://arrow.local/quickplay/{id}` when a known tag is detected
4. A 3-second cooldown prevents duplicate triggers from the same tag
