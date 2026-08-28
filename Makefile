SHELL := /bin/bash
CMAKE ?= cmake
BUILD_DIR := build
WEB_BUILD_DIR := web-build

.PHONY: all configure build run clean web serve serve-web appimage android

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

run: build
	./$(BUILD_DIR)/dungeon-hunter

clean:
	rm -rf $(BUILD_DIR) $(WEB_BUILD_DIR) Dungeon-Hunter-x86_64.AppImage

web:
	./scripts/build-web.sh

serve: web
	./scripts/serve-web.sh

serve-web: serve

appimage:
	./scripts/build-appimage.sh

android:
	@echo "Android build target is deferred during the foundation repair phase."
