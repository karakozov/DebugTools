
PHONY = clean
TARGET_NAME = amb_info

include $(GIPCYDIR)/make/macros.mk

DIRS := $(GIPCYINC)
#DIRS += $(BARDYDIR)/BRDINC
#DIRS += $(BARDYDIR)/BRDINC/ctrladmpro
#DIRS += $(BARDYDIR)/IcrInc
#DIRS += $(BARDYDIR)/BRDLIBS

INC := $(addprefix  -I, $(DIRS))

CFLAGS += $(INC)

SRC := $(wildcard *.cpp)
SOURCE += $(SRC)
OBJ_FILES := $(SOURCE:.cpp=.o)

all: $(TARGET_NAME)

include $(GIPCYDIR)/make/rules.mk
include $(GIPCYDIR)/make/clean.mk
include $(GIPCYDIR)/make/app.mk
