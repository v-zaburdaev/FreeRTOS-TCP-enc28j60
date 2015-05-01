# Configuration for Makefile
# Project name and in the same time names of the executables.
PROJ_NAME=hal

# sources' directories
SRCS         := $(wildcard src/*.c) $(wildcard lib/FreeRTOS/*.c)
SRCSCPP      := $(wildcard src/*.cpp)

# headers' directories
CINCS += -Iinclude -Ilib/stm32cubef4/include -Ilib/include/stm32cubef4/Legacy -Ilib/FreeRTOS/include

###
# TODO: Probably we don't need this section
#
# look for sources in given folders
# VPATH := src lib
###


# build directory configuration
BUILDDIR  := build
OBJSDIR   := $(BUILDDIR)/objs
DEPSDIR   := $(BUILDDIR)/deps

DEFINES   := -DSTM32F407xx

# You don't need to edit anything below this line
###################################################

# add list of needed library's objects - OBJSLIB
-include objslib.mk

OBJS    := $(SRCS:%.c=$(OBJSDIR)/%.o)
OBJSCPP := $(SRCSCPP:%.cpp=$(OBJSDIR)/%.o)
OBJSLIB := $(addprefix $(OBJSDIR)/, $(OBJSLIB))

# C compiler's settings
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy


# C compiler's options
CFLAGS := -Wall -std=c11 -Os
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += $(DEFINES)

# Cpp compiler's settings
CPP=arm-none-eabi-g++
CPPFLAGS := -Wall -std=c++11 -Os
CPPFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CPPFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CPPFLAGS += $(DEFINES)

# linker's scripts
LDSRCS    := ldscripts/libs.ld ldscripts/mem.ld ldscripts/sections.ld

# linker's settings
LDFLAGS := $(LDSRCS:%=-T%) -specs nosys.specs --specs=rdimon.specs -lc -lrdimon


# advanced settings
#
# LDLIBS  = -Llib -lstm32f4
# -mfpu=fpv4-sp-d16
# CFLAGS += -ffreestanding -nostdlib

###################################################

.PHONY: all proj depends debug release echo_variables link_needed_lib

.SUFFIXES:

debug: CFLAGS   += -g
debug: CPPFLAGS += -g -DDEBUG
debug: proj
all: proj
release: CFLAGS   += -Os
release: CPPFLAGS += -Os
release: proj

depends: link_needed_lib


#################################################
#        debug and dependancy generation        #
#################################################
echo_variables:
	@echo OBJS: $(OBJS)
	@echo OBJSCPP: $(OBJSCPP)
	@echo OBJSLIB: $(OBJSLIB)
	@echo DEPS: $(SRCS:src/%.c=$(DEPSDIR)/%.d)
	@echo DEPS: $(SRCSCPP:src/%.cpp=$(DEPSDIR)/%.d)

link_needed_lib: $(addprefix $(DEPSDIR)/, $(SRCS:%.c=%.d)) $(addprefix $(DEPSDIR)/, $(SRCSCPP:%.cpp=%.d))
	@for i in "$(DEPSDIR)/src/*.d"; do \
		sed 's/\\//g; s/ /\n/g;' $$i | \
		grep 'stm32f4xx_.*\.h' | \
		sed -e '/stm32f4xx_hal_conf.h/d'     \
		    -e '/stm32f4xx_hal_def.h/d'      \
		    -e '/stm32f4xx_hal_gpio_ex.h/d;' \
		    -e '/stm32f4xx_it.h/d;' \
			-e 's:/include::g; s/h$$/o/g' >> objslib.tmp ;\
	done
	@rm -f objslib.mk
	@echo 'OBJSLIB :=' >> objslib.mk
	@sort -u objslib.tmp >> objslib.mk
	@cat objslib.mk | tr '\n' ' '> objslib.mk2
	@mv objslib.mk2 objslib.mk
	@rm -f objslib.tmp
	
$(DEPSDIR)/%.d: %.c | $(BUILDDIR)
	@$(CC) $(CFLAGS) $(CINCS) -MM -o $@ $<

$(DEPSDIR)/%.d: %.cpp | $(BUILDDIR)
	@$(CPP) $(CPPFLAGS) $(CINCS) -MM -MF $@ $<

#################################################

# add list of needed library's objects - OBJSLIB



proj: $(PROJ_NAME).elf 

$(PROJ_NAME).elf: $(OBJS) $(OBJSLIB) $(OBJSCPP)
	@echo "  (LDCPP) -o $@ $^"
	@$(CPP) $(CPPFLAGS) $(LDFLAGS) $(CINCS) -o $@ $^ src/startup_stm32f407xx.s $(LDLIBS)
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

$(OBJS): | $(BUILDDIR)

$(OBJSDIR)/%.o : %.c 
	@echo "  (CC) $@"
	@$(CC) $(CFLAGS) $(CINCS) -MMD -MF $(DEPSDIR)/$(basename $<).d -c -o $@ $<

-include $(SRCS:%.c=$(DEPSDIR)/%.d)

$(OBJSDIR)/%.o : %.cpp
	@echo "  (CPP) $@"
	@$(CPP) $(CPPFLAGS) $(CINCS) -MMD -MF $(DEPSDIR)/$(basename $<).d -c -o $@ $<

-include $(SRCSCPP:%.cpp=$(DEPSDIR)/%.d)

# TODO: maybe dynamic creation of directories
$(BUILDDIR):
	mkdir -p $(OBJSDIR)/src
	mkdir -p $(OBJSDIR)/lib/stm32cubef4
	mkdir -p $(OBJSDIR)/lib/FreeRTOS
	mkdir -p $(DEPSDIR)/src
	mkdir -p $(DEPSDIR)/lib/stm32cubef4
	mkdir -p $(DEPSDIR)/lib/FreeRTOS
	


# Take care about non-existing headers.
# If a header doesn't exist, it assumes, that the header has been changed.
# see: GNU Make Manual "Rules without Commands or Prerequisites"
%.h:

clean:
	rm -f $(OBJSLIB)
	rm -f $(OBJSCPP)
	rm -rf $(BUILDDIR)
	rm -f objslib.mk
