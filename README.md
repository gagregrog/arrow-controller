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
- NFC interrupt: GPIO4
- NFC reset: GPIO5

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

1. Scans for NFC tags continuously
2. Looks up the tag UID in the tag map
3. POSTs to `http://arrow.local/quickplay/{id}` when a known tag is detected
