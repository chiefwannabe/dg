SHELL := /bin/bash
CMAKE ?= cmake
BUILD_DIR := build
WEB_BUILD_DIR := web-build

.PHONY: all configure build run clean web serve android

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

run: build
	./$(BUILD_DIR)/dungeon-hunter

clean:
	rm -rf $(BUILD_DIR) $(WEB_BUILD_DIR)

web:
	./scripts/build-web.sh

serve: web
	./scripts/serve-web.sh

android:
	./scripts/build-android.sh
