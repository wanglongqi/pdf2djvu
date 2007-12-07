DJVULIBRE_BIN_PATH = /usr/bin

POPPLER_VERSION = $(shell ./tools/get-poppler-version)
EXT_CFLAGS = $(shell pkg-config --cflags poppler ddjvuapi) -DPOPPLER_VERSION=$(POPPLER_VERSION)
EXT_LDFLAGS = $(shell pkg-config --libs poppler ddjvuapi)

CXXFLAGS ?= -O3
override LDFLAGS += $(EXT_LDFLAGS)
override CXXFLAGS += $(EXT_CFLAGS) -DDJVULIBRE_BIN_PATH="\"$(strip $(DJVULIBRE_BIN_PATH))\""

.PHONY: all
all: pdf2djvu

pdf2djvu: pdf2djvu.cc compoppler.h

.PHONY: clean
clean:
	$(RM) pdf2djvu

.PHONY: test
test: pdf2djvu
	./pdf2djvu example.pdf > example.djvu

# vim:ts=4 sw=4 noet
