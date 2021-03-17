####################################################################
# Makefile                                                         
# 
# OS variable must either be 'posix' or 'win'. E.g. 'make OS=posix'.
# Error is thrown if OS variable is not equal with any of these.
#
####################################################################

.SUFFIXES:				# ignore builtin rules
.PHONY: all debug release clean

####################################################################
# Definitions                                                      #
####################################################################

ifeq (1,VERBOSE)
	Q:=@
endif

# uniq is a function which removes duplicate elements from a list
uniq = $(strip $(if $1,$(firstword $1) \
       $(call uniq,$(filter-out $(firstword $1),$1))))

PROJECTNAME = provisioner
DBG  ?= 1
DBGP ?= 1
PLVL ?= 5
JSON_SUP ?= 1
XML_SUP ?= 0

OBJ_DIR = build_sdk/obj
EXE_DIR = build_sdk
LST_DIR = build_sdk/lst

CWD="$(shell pwd)/"

ifneq ($(VERBOSE), 1)
  Q:=@
endif

####################################################################
# Definitions of toolchain.                                        #
# You might need to do changes to match your system setup          #
####################################################################

RMDIRS     := rm -rf
RMFILES    := rm -rf
ALLFILES   := /*.*
NULLDEVICE := /dev/null
SHELLNAMES := $(ComSpec)$(COMSPEC)

# Try autodetecting the environment: Windows
ifneq ($(SHELLNAMES),)
  QUOTE :="
  ifeq (,$(filter $(OS),posix win))
    OS:=win
  endif
  ifneq ($(COMSPEC),)
    ifeq ($(findstring cygdrive,$(shell set)),)
      # We were not on a cygwin platform
      NULLDEVICE := NUL
    endif
  else
    # Assume we are making on a Windows platform
    # This is a convenient place to override TOOLDIR, DO NOT add trailing
    # whitespace chars, they do matter !
    SHELL      := $(SHELLNAMES)
    RMDIRS     := rd /s /q
    RMFILES    := del /s /q
    ALLFILES   := \*.*
    NULLDEVICE := NUL
  endif
# Other than Windows
else
  ifeq (,$(filter $(OS),posix win))
    OS:=posix
  endif
endif

# Create directories and do a clean which is compatible with parallell make
$(shell mkdir -p $(OBJ_DIR)>$(NULLDEVICE) 2>&1)
$(shell mkdir -p $(EXE_DIR)>$(NULLDEVICE) 2>&1)
$(shell mkdir -p $(LST_DIR)>$(NULLDEVICE) 2>&1)
ifeq (clean,$(findstring clean, $(MAKECMDGOALS)))
  ifneq ($(filter $(MAKECMDGOALS),all debug release),)
    $(shell $(RMFILES) $(OBJ_DIR)$(ALLFILES)>$(NULLDEVICE) 2>&1)
    $(shell $(RMFILES) $(EXE_DIR)$(ALLFILES)>$(NULLDEVICE) 2>&1)
    $(shell $(RMFILES) $(LST_DIR)$(ALLFILES)>$(NULLDEVICE) 2>&1)
  endif
endif

CC      = gcc
LD      = ld
AR      = ar


####################################################################
# Flags                                                            #
####################################################################

INCLUDEPATHS += \
-I./inc/async \
-I/usr/local/include/libxml2 \
-I/usr/include/json-c \
-I./inc \
-I./inc/bg_stack \
-I./common/uart \
-I./utest

# -MMD : Don't generate dependencies on system header files.
# -MP  : Add phony targets, useful when a h-file is removed from a project.
# -MF  : Specify a file to write the dependencies to.
DEPFLAGS = \
-MMD \
-MP \
-MF $(@:.o=.d)

override ASMFLAGS += \
-std=c99
# Add -Wa,-ahld=$(LST_DIR)/$(@F:.o=.lst) to CFLAGS to produce assembly list files
override CFLAGS += \
-fno-short-enums\
-Wall \
-c \
-fmessage-length=0 \
-std=c99 \
$(DEPFLAGS)

# Linux platform: if _DEFAULT_SOURCE is defined, the default is to have _POSIX_SOURCE set to one
# and _POSIX_C_SOURCE set to 200809L, as well as enabling miscellaneous functions from BSD and SVID.
# See usr/include/fetures.h for more information.
# 
# _BSD_SOURCE (deprecated since glibc 2.20)
# Defining this macro with any value causes header files to expose BSD-derived definitions.
# In glibc versions up to and including 2.18, defining this macro also causes BSD definitions to be
# preferred in some situations where standards conflict, unless one or more of _SVID_SOURCE,
# _POSIX_SOURCE, _POSIX_C_SOURCE, _XOPEN_SOURCE, _XOPEN_SOURCE_EXTENDED, or _GNU_SOURCE is defined,
# in which case BSD definitions are disfavored. Since glibc 2.19, _BSD_SOURCE no longer causes BSD
# definitions to be preferred in case of conflicts. Since glibc 2.20, this macro is deprecated. 
# It now has the same effect as defining _DEFAULT_SOURCE, but generates a compile-time warning
# (unless _DEFAULT_SOURCE is also defined). Use _DEFAULT_SOURCE instead.
# To allow code that requires _BSD_SOURCE in glibc 2.19 and earlier and _DEFAULT_SOURCE in glibc
# 2.20 and later to compile without warnings, define both _BSD_SOURCE and _DEFAULT_SOURCE.
#
# OSX platform: _DEFAULT_SOURCE is not used, instead _DARWIN_C_SOURCE is defined by default.
# ifeq ($(OS),posix)
override CFLAGS += \
-D_DEFAULT_SOURCE \
-D_BSD_SOURCE \
-DLIBXML_STATIC \
-DSRC_ROOT_DIR='$(CWD)' \
-D__FILENAME__='"$(subst $(CWD),,$(abspath $<))"' \
-DXML_SUP=$(XML_SUP) \
-DJSON_SUP=$(JSON_SUP)
# endif

# NOTE: The -Wl,--gc-sections flag may interfere with debugging using gdb.
override LDFLAGS +=

####################################################################
# Files                                                            #
####################################################################

C_SRC +=  \
$(wildcard common/*.c) \
$(wildcard utest/*.c) \
$(wildcard src/async/*.c) \
$(wildcard src/states/*.c) \
src/common/conf_generic_parser.c \
src/common/conf_out.c \
src/common/err_code.c \
src/common/logging.c \
src/common/scanner.c \
src/common/socket_handler.c \
src/common/utils.c \
main.c

# this file should be the last added
ifeq ($(OS),posix)
C_SRC += ./common/uart/uart_posix.c
else ifeq ($(OS),win)
C_SRC += ./common/uart/uart_win.c
endif

ifeq ($(JSON_SUP), 1)
C_SRC += src/common/json_parser.c
endif
ifeq ($(XML_SUP), 1)
C_SRC += src/common/xml_parser.c
endif

s_SRC += 

S_SRC += 

LIBS =


####################################################################
# Rules                                                            #
####################################################################

C_FILES = $(notdir $(C_SRC) )
S_FILES = $(notdir $(S_SRC) $(s_SRC) )
#make list of source paths, uniq removes duplicate paths
C_PATHS = $(call uniq, $(dir $(C_SRC) ) )
S_PATHS = $(call uniq, $(dir $(S_SRC) $(s_SRC) ) )

C_OBJS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.o))
S_OBJS = $(if $(S_SRC), $(addprefix $(OBJ_DIR)/, $(S_FILES:.S=.o)))
s_OBJS = $(if $(s_SRC), $(addprefix $(OBJ_DIR)/, $(S_FILES:.s=.o)))
C_DEPS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.d))
OBJS = $(C_OBJS) $(S_OBJS) $(s_OBJS)

vpath %.c $(C_PATHS)
vpath %.s $(S_PATHS)
vpath %.S $(S_PATHS)

# Default build is debug build
all:      debug

debug:    CFLAGS += -O0 -g3 -DDEBUG=$(DBG) -DDEBUG_PRINT=$(DBGP) -DPRINT_LEVEL=$(PLVL)
release:  CFLAGS += -O0 -g3 -DDEBUG=0 -DDEBUG_PRINT=0 -DPRINT_LEVEL=4
# release:  CFLAGS += -Os -DDEBUG=0 -DDEBUG_PRINT=0 -DPRINT_LEVEL=4

debug:    $(EXE_DIR)/$(PROJECTNAME)
release:  $(EXE_DIR)/$(PROJECTNAME)


# Create objects from C SRC files
$(OBJ_DIR)/%.o: %.c
	@echo "Building file: $<"
	$(Q)$(CC) $(CFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Assemble .s/.S files
$(OBJ_DIR)/%.o: %.s
	@echo "Assembling $<"
	$(Q)$(CC) $(ASMFLAGS) $(INCLUDEPATHS) -c-o $@ $<

$(OBJ_DIR)/%.o: %.S
	@echo "Assembling $<"
	$(Q)$(CC) $(ASMFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Link - /usr/local/lib/libxml2.a
$(EXE_DIR)/$(PROJECTNAME): $(OBJS) $(LIBS)
	@echo "Linking target: $@"
	$(Q)$(CC) $(LDFLAGS) $^ -o $@ -lrt -lm -lpthread -ljson-c /usr/local/lib/libxml2.a

help:
	@echo "OS=$(OS)"
	@echo "CWD=$(CWD)"
	@echo "DBG = 0/1 to enable/disable debug mode"
	@echo "DBGP = 0/1 to enable/disable debug print"
	@echo "PLVL = [0, 10] to set printf level, 3=err, 4=msg, 5=log"

clean:
ifeq ($(filter $(MAKECMDGOALS),all debug release),)
	$(Q)$(RMDIRS) $(OBJ_DIR) $(LST_DIR) $(EXE_DIR) compile_commands.json
endif

# include auto-generated dependency files (explicit rules)
ifneq (clean,$(findstring clean, $(MAKECMDGOALS)))
-include $(C_DEPS)
endif
