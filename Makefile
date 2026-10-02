.DEFAULT_GOAL := help
.PHONY: help all save-cli compile run-wine fmt compile-check res resources checkpoints-starsbox checkpoints-native checkpoints-compare tutorial tutorial-reject clean

DIST_DIR    ?= dist
CMAKE       ?= cmake
CLANG_FORMAT ?= clang-format
PYTHON      ?= python3
GO          ?= go
SAVE_CLI    := $(abspath $(DIST_DIR))/stars-save
MINGW_CC    ?= x86_64-w64-mingw32-gcc
MINGW_RC    ?= x86_64-w64-mingw32-windres
FILES       ?= $(wildcard *.c)
FORMAT_FILES ?= $(wildcard *.c *.h res/*.h tests/scaffold/*.c tests/scaffold/*.h tests/scaffold/tutorial/*.c tests/scaffold/tutorial/*.h)
STARSBOX    ?= tests/scaffold/starsbox
SEED        ?= 12345
NATIVE_BUILD := $(DIST_DIR)/regression-build
ORIGINAL_WORK ?= $(STARSBOX)/c_drive/REGTEST
NATIVE_WORK   ?= $(STARSBOX)/c_drive/native

help:
	@echo "Targets:"
	@echo "  save-cli             Build the standalone test save CLI"
	@echo "  compile              Build stars.exe with the MinGW CMake preset"
	@echo "  fmt                  Format C sources and headers (FORMAT_FILES=ai.c to limit)"
	@echo "  compile-check        Check C syntax (FILES=ai.c to limit) and resources"
	@echo "  res / resources      Compile res/stars.rc into $(DIST_DIR)/stars_res.o"
	@echo "  checkpoints-starsbox Generate fresh original checkpoints (takes a long time)"
	@echo "  checkpoints-native   Build with a fixed seed and generate fresh native checkpoints"
	@echo "  checkpoints-compare  Compare original and native checkpoints"
	@echo "  tutorial             Run the complete AutoHotkey v2 tutorial under Wine"
	@echo "  tutorial-reject      Verify early Generate is rejected"
	@echo "  clean                Remove $(DIST_DIR)/"

all: compile

save-cli:
	@mkdir -p "$(DIST_DIR)"
	cd tests/savecli && $(GO) build -o "$(SAVE_CLI)" .

compile:
	$(CMAKE) --preset mingw-debug
	$(CMAKE) --build --preset mingw-debug

run-wine:
	$(CMAKE) --preset mingw-debug
	$(CMAKE) --build --preset run-wine

fmt:
	$(CLANG_FORMAT) --style=file -i $(FORMAT_FILES)

compile-check:
	@set -e; for f in $(FILES); do \
		$(MINGW_CC) -std=gnu11 -fsigned-char -fsyntax-only -fmax-errors=0 -Wno-pointer-sign -I. "$$f"; \
	done
	cd res && $(MINGW_RC) stars.rc -O res -o /dev/null

res: resources

resources:
	@mkdir -p "$(DIST_DIR)"
	cd res && $(MINGW_RC) stars.rc -O coff -o "$(abspath $(DIST_DIR))/stars_res.o"

# Preparing checkpoints requires fresh work directories.
checkpoints-starsbox: save-cli
	rm -rf "$(ORIGINAL_WORK)"
	$(PYTHON) tests/scaffold/regression.py prepare --engine dosbox --seed $(SEED) --exe "$(STARSBOX)/c_drive/STARS/stars.exe" --work "$(ORIGINAL_WORK)"
	$(PYTHON) tests/scaffold/regression.py run --cli "$(SAVE_CLI)" --work "$(ORIGINAL_WORK)"

checkpoints-native: save-cli
	$(CMAKE) --preset mingw-debug -B "$(NATIVE_BUILD)" -DSTARS_TEST_SEED=$(SEED)
	$(CMAKE) --build "$(NATIVE_BUILD)"
	rm -rf "$(NATIVE_WORK)"
	$(PYTHON) tests/scaffold/regression.py prepare --engine native --seed $(SEED) --exe "$(NATIVE_BUILD)/bin/stars.exe" --work "$(NATIVE_WORK)"
	$(PYTHON) tests/scaffold/regression.py run --cli "$(SAVE_CLI)" --work "$(NATIVE_WORK)"

checkpoints-compare: save-cli
	$(PYTHON) tests/scaffold/regression.py compare --cli "$(SAVE_CLI)" "$(ORIGINAL_WORK)" "$(NATIVE_WORK)"

# STARS_TUTORIAL_SERIAL optionally overrides the runner's default serial.
tutorial:
	$(PYTHON) tests/scaffold/tutorial/run.py --download-ahk $(TUTORIAL_ARGS)

tutorial-reject:
	$(PYTHON) tests/scaffold/tutorial/run.py --download-ahk --scenario reject-generate $(TUTORIAL_ARGS)

clean:
	rm -rf "$(DIST_DIR)"
