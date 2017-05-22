
PHONY = clean
TARGET_NAME = amb_info

include $(BARDYDIR)/make/macros.mk

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

include $(BARDYDIR)/make/rules.mk
include $(BARDYDIR)/make/clean.mk
include $(BARDYDIR)/make/app.mk
