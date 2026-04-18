.PHONY: build flash flash-ota monitor test clean

build:
	pio run -e esp32dev

flash:
	pio run -e esp32dev -t upload

flash-ota:
	pio run -e esp32ota -t upload

monitor:
	pio device monitor

test:
	pio test -e native

clean:
	pio run -t clean
