CC = xtensa-lx106-elf-gcc
CFLAGS = -I./src -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

BUILD_DIR = ./build

all: $(BUILD_DIR)/esp-beacon.bin $(BUILD_DIR)/esp-beacon-ap.bin

$(BUILD_DIR)/esp-beacon.bin: $(BUILD_DIR)/esp-beacon build
	esptool.py elf2image --version=2 -o $@ $<

$(BUILD_DIR)/esp-beacon-ap.bin: $(BUILD_DIR)/esp-beacon-ap build
	esptool.py elf2image --version=2 -o $@ $<

$(BUILD_DIR)/esp-beacon: $(BUILD_DIR)/esp-beacon.o build
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

$(BUILD_DIR)/esp-beacon-ap: $(BUILD_DIR)/esp-beacon-ap.o build
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

$(BUILD_DIR)/esp-beacon.o: src/esp-beacon.c build
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/esp-beacon-ap.o: src/esp-beacon.c build
	$(CC) $(CFLAGS) -DBEACON_AP -c $< -o $@

build:
	mkdir -p build

flash: $(BUILD_DIR)/esp-beacon.bin
	esptool.py write_flash 0 $<

flash-ap: $(BUILD_DIR)/esp-beacon-ap.bin
	esptool.py write_flash 0 $<

clean:
	rm -rf build


.PHONY: all flash clean
