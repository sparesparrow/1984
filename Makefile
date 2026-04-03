SHELL := /bin/bash
.PHONY: help setup build-dev build-ship package deploy clean logs

# -----------------------------------------------------------------------
# Configuration — override via environment or command line
# -----------------------------------------------------------------------
UE_ROOT         ?= $(HOME)/UnrealEngine
BUILD_CONFIG    ?= Development
COOK_FLAVOR     ?= ASTC
OUTPUT_DIR      ?= $(CURDIR)/Build/Android
UPROJECT        := $(CURDIR)/1984.uproject

# -----------------------------------------------------------------------
help: ## Show available targets
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-18s\033[0m %s\n", $$1, $$2}'

# -----------------------------------------------------------------------
setup: ## Install Android SDK + NDK (requires curl, Java 11+)
	@bash scripts/setup-android-sdk.sh

# -----------------------------------------------------------------------
build-dev: ## Cook + package Development APK for Meta Quest
	@bash scripts/build-android.sh --config Development --flavor $(COOK_FLAVOR) --output $(OUTPUT_DIR)

build-ship: ## Cook + package Shipping APK for Meta Quest (store release)
	@bash scripts/build-android.sh --config Shipping --flavor $(COOK_FLAVOR) --output $(OUTPUT_DIR)

package: build-dev ## Alias for build-dev

# -----------------------------------------------------------------------
deploy: ## Deploy most recent APK to connected Quest (ADB)
	@bash scripts/deploy-to-quest.sh

deploy-build: build-dev ## Build Development APK then deploy to Quest
	@bash scripts/deploy-to-quest.sh

# -----------------------------------------------------------------------
logs: ## Stream UE4 logcat from connected Quest
	@command -v adb &>/dev/null || (echo "ERROR: adb not found — run make setup" && exit 1)
	adb logcat -s UE4:V AndroidRuntime:E

# -----------------------------------------------------------------------
clean: ## Remove build outputs (Build/, Saved/, Intermediate/)
	@echo "Cleaning build artifacts..."
	@rm -rf Build/ Saved/ Intermediate/ DerivedDataCache/
	@echo "Clean complete."
