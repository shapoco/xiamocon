.PHONY: all build clean distclean

APP_NAME = ocarina
REPO_DIR = $(shell git rev-parse --show-toplevel)
BUILD_DIR = build
BIN_DIR = bin/$(BOARD)

#BOARD := pico
BOARD := pico2
#BOARD := pico_w

BIN_NAME = $(APP_NAME).uf2
ELF_NAME = $(APP_NAME).elf
BIN = $(BIN_DIR)/$(BIN_NAME)
ELF = $(BIN_DIR)/$(ELF_NAME)

APP_INC_DIR = include
APP_SRC_DIR = src
APP_HPP_LIST = \
	$(wildcard $(APP_INC_DIR)/*.hpp) \
	$(wildcard $(APP_INC_DIR)/*.h)
APP_CPP_LIST = \
	$(wildcard $(APP_SRC_DIR)/*.cpp) \
	$(wildcard $(APP_SRC_DIR)/*.c)

PFM_DECL_INC_DIR = $(REPO_DIR)/cpp/platform/decl/include
PFM_DECL_SRC_DIR = $(REPO_DIR)/cpp/platform/decl/src
PFM_IMPL_INC_DIR = $(REPO_DIR)/cpp/platform/impl/rp2350_pico_sdk/include
PFM_IMPL_SRC_DIR = $(REPO_DIR)/cpp/platform/impl/rp2350_pico_sdk/src
PFM_HPP_LIST = \
	$(wildcard $(PFM_DECL_INC_DIR)/*.hpp) \
	$(wildcard $(PFM_DECL_INC_DIR)/*.h) \
	$(wildcard $(PFM_IMPL_INC_DIR)/*.hpp) \
	$(wildcard $(PFM_IMPL_INC_DIR)/*.h)
PFM_CPP_LIST = \
	$(wildcard $(PFM_DECL_SRC_DIR)/*.cpp) \
	$(wildcard $(PFM_DECL_SRC_DIR)/*.c) \
	$(wildcard $(PFM_IMPL_SRC_DIR)/*.cpp) \
	$(wildcard $(PFM_IMPL_SRC_DIR)/*.c)

LIB_INC_DIR = $(REPO_DIR)/cpp/library/include
LIB_SRC_DIR = $(REPO_DIR)/cpp/library/src
LIB_HPP_LIST = \
	$(wildcard $(LIB_INC_DIR)/*.hpp) \
	$(wildcard $(LIB_INC_DIR)/*.h) \
	$(wildcard $(LIB_INC_DIR)/*/*.hpp) \
	$(wildcard $(LIB_INC_DIR)/*/*.h)
LIB_CPP_LIST = \
	$(wildcard $(LIB_SRC_DIR)/*.cpp) \
	$(wildcard $(LIB_SRC_DIR)/*.c) \
	$(wildcard $(LIB_SRC_DIR)/*/*.cpp) \
	$(wildcard $(LIB_SRC_DIR)/*/*.c)

DEPENDENCY_LIST = \
	$(APP_HPP_LIST) \
	$(APP_CPP_LIST) \
	$(PFM_HPP_LIST) \
	$(PFM_CPP_LIST) \
	$(LIB_HPP_LIST) \
	$(LIB_CPP_LIST) \
	Makefile

all: build
build: $(BIN)

$(BIN): $(DEPENDENCY_LIST) CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) \
		&& cmake -DPICO_BOARD=$(BOARD) .. \
		&& make -j
	mkdir -p $(BIN_DIR)
	cp $(BUILD_DIR)/$(BIN_NAME) $(BIN)
	cp $(BUILD_DIR)/$(ELF_NAME) $(ELF)
	@echo "------------------------------"
	@grep "todo" -r $(APP_SRC_DIR) $(APP_INC_DIR) || true
	@echo "------------------------------"
	@echo -n "UF2 File: "
	@du -h $(BIN)
	@echo -n "ELF File: "
	@du -h $(ELF)

$(ELF): $(BIN)


PICO_DRIVE := e

flash: $(BIN)
	sudo mkdir -p /mnt/$(PICO_DRIVE)
	sudo mount -t drvfs $(PICO_DRIVE): /mnt/$(PICO_DRIVE)
	cp $(BIN) /mnt/$(PICO_DRIVE)/.

objclean:
	rm -f \
		$(BUILD_DIR)/*.uf2 \
		$(BUILD_DIR)/*.elf

clean: objclean
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(BIN_DIR)
