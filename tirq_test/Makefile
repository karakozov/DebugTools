
PHONY = clean
TARGET_NAME = tirq_test

include $(BARDYDIR)/make/macros.mk

DIRS := $(GIPCYINC) $(BARDYDIR)/BRDINC $(BARDYDIR)/BRDINC/ctrladmpro $(BARDYDIR)/IcrInc \
$(BARDYDIR)/BRDLIBS
INC := $(addprefix  -I, $(DIRS))

CFLAGS += $(INC)

SRC := $(wildcard *.cpp)
SOURCE += $(SRC)
OBJ_FILES := $(SOURCE:.cpp=.o)

all: $(TARGET_NAME)

include $(BARDYDIR)/make/rules.mk
include $(BARDYDIR)/make/clean.mk
include $(BARDYDIR)/make/app.mk
