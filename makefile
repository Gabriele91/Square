#Input
MKDIR_P       ?=mkdir -p
COMPILER      ?=g++
TESTS         ?=true
TOP           ?=$(shell pwd)
HAVE_TERM     :=$(shell echo $$TERM)
#undef to none (linux)
ifndef HAVE_TERM
	HAVE_TERM = none
endif
#dump to none (macOS)
ifeq ($(HAVE_TERM),dumb)
	HAVE_TERM = none
endif
#program name
S_DIR  = $(TOP)/source/
S_INC  = $(TOP)/include/
S_TEST_DIR  = $(TOP)/tests/
S_TEST_INC  = $(TOP)/tests/

DEBUG_DIR    = Debug/obj
RELEASE_DIR  = Release/obj
DEBUG_PROG   = Debug/Square
RELEASE_PROG = Release/Square

#global include
DIPS_INCLUDE = $(TOP)/dependencies/linux/include/

# Subdirs
SUB_DIRS := $(wildcard $(S_DIR)/**/.)\
            $(wildcard $(S_DIR)/**/**/.)
SUB_DIRS := $(subst $(S_DIR)/,,$(SUB_DIRS))

####################################################
# C FLAGS
C_FLAGS = -fPIC -D_FORCE_INLINES
# CPP FLAGS
CC_FLAGS = -std=c++14 -I $(DIPS_INCLUDE) -I $(S_INC)
# RELEASE_FLAGS
RELEASE_FLAGS = -O3
# DEBUG_FLAGS
DEBUG_FLAGS = -g -D_DEBUG -Wall 
# Linker
LDFLAGS += -lz -lm -lutil 

# Linux flags
ifeq ($(shell uname -s),Linux)
# threads
C_FLAGS += -lpthread -pthread 
# Xorg
LDFLAGS += -lX11 -lXxf86vm -lGLEW -lGLU -lGL -lxcb
endif

# MacOS flags
ifeq ($(shell uname -s),Darwin)
# XQuartz
CC_FLAGS += -I /opt/X11/include/
LDFLAGS  += -L /opt/X11/lib/ -lX11 -lXxf86vm -lGLU -lGL -lxcb
endif

####################################################
# No win32
FILTER := $(wildcard $(S_DIR)/**/Win32/*.cpp)

# No macOS
FILTER += $(wildcard $(S_DIR)/**/MacOS/*.cpp)\
          $(wildcard $(S_DIR)/**/Cocoa/*.cpp)

# Libary Source
ALL_SOURCE_FILES := $(wildcard $(S_DIR)/*.cpp)\
					$(wildcard $(S_DIR)/**/*.cpp)\
					$(wildcard $(S_DIR)/**/**/*.cpp)	

####################################################
# Object files
SOURCE_FILES = $(filter-out $(FILTER), $(ALL_SOURCE_FILES))
SOURCE_DEBUG_OBJS = $(subst $(S_DIR),$(DEBUG_DIR),$(subst .cpp,.o,$(SOURCE_FILES)))
SOURCE_RELEASE_OBJS = $(subst $(S_DIR),$(RELEASE_DIR),$(subst .cpp,.o,$(SOURCE_FILES)))

####################################################
# Test Source
ifeq ($(TESTS),true)	
# outdir
O_TEST_DIR := tests
# flags
CC_FLAGS += -I $(S_TEST_INC)
# source
TEST_SOURCE_FILES = $(wildcard $(S_TEST_DIR)/**.cpp)
TEST_SOURCE_DEBUG_OBJS = $(subst $(S_TEST_DIR),$(DEBUG_DIR)/$(O_TEST_DIR),$(subst .cpp,.o,$(TEST_SOURCE_FILES)))
TEST_SOURCE_RELEASE_OBJS = $(subst $(S_TEST_DIR),$(RELEASE_DIR)/$(O_TEST_DIR),$(subst .cpp,.o,$(TEST_SOURCE_FILES)))
# fake sub dir
SUB_DIRS += $(O_TEST_DIR)
else 
#void
TEST_SOURCE_FILES =
TEST_SOURCE_DEBUG_OBJS = 
TEST_SOURCE_RELEASE_OBJS = 
O_TEST_DIR := 
endif	
####################################################

# Output dirs
O_DEBUG_DIR    = $(TOP)/$(DEBUG_DIR)
O_RELEASE_DIR  = $(TOP)/$(RELEASE_DIR)
O_DEBUG_PROG   = $(TOP)/$(DEBUG_PROG)
O_RELEASE_PROG = $(TOP)/$(RELEASE_PROG)



##
# Support function for colored output
# Args:
#     - $(1) = Color Type
#     - $(2) = String to print
ifneq ($(HAVE_TERM),none)
define colorecho
	@tput setaf $(1)
	@echo $(2)
	@tput sgr0
endef
else
define colorecho
	@echo $(2)
endef
endif

# Color Types
COLOR_BLACK = 0
COLOR_RED = 1
COLOR_GREEN = 2
COLOR_YELLOW = 3
COLOR_BLUE = 4
COLOR_MAGENTA = 5
COLOR_CYAN = 6
COLOR_WHITE = 7

all: directories show_debug_flags debug release

directories: debug_make_dirs release_make_dirs

rebuild: clean directories debug release

rebuild_debug: clean_debug debug

rebuild_release: clean_release release

debug: directories show_debug_flags $(SOURCE_DEBUG_OBJS) $(TEST_SOURCE_DEBUG_OBJS)
	$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(SOURCE_DEBUG_OBJS) $(TEST_SOURCE_DEBUG_OBJS) $(LDFLAGS) -o $(O_DEBUG_PROG)
	
release: directories show_release_flags $(SOURCE_RELEASE_OBJS) $(TEST_SOURCE_RELEASE_OBJS)
	$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(SOURCE_RELEASE_OBJS) $(TEST_SOURCE_RELEASE_OBJS) $(LDFLAGS) -o $(O_RELEASE_PROG)

# makedir
debug_make_dirs:
	@for dir in $(SUB_DIRS); do \
		${MKDIR_P} $(DEBUG_DIR)/$$dir; \
	done

# makedir
release_make_dirs:
	@for dir in $(SUB_DIRS); do \
		${MKDIR_P} $(RELEASE_DIR)/$$dir; \
	done

show_debug_flags:
	$(call colorecho,$(COLOR_YELLOW),"[ Debug flags: $(C_FLAGS) $(CC_FLAGS) $(DEBUG_FLAGS) ]")

show_release_flags:
	$(call colorecho,$(COLOR_YELLOW),"[ Release flags: $(C_FLAGS) $(CC_FLAGS) $(RELEASE_FLAGS) ]")

##################################################################################################################
# DEBUG
$(SOURCE_DEBUG_OBJS):
	$(call colorecho,$(COLOR_GREEN),"[ Make debug object: $(subst $(DEBUG_DIR),,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@)) ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(DEBUG_FLAGS) -c $(subst $(DEBUG_DIR),$(S_DIR),$(@:.o=.cpp)) -o $@

$(TEST_SOURCE_DEBUG_OBJS):
	$(call colorecho,$(COLOR_GREEN),"[ Make debug object: $(subst $(DEBUG_DIR),,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@)) ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(DEBUG_FLAGS) -c $(subst $(DEBUG_DIR)/$(O_TEST_DIR),$(S_TEST_DIR),$(@:.o=.cpp)) -o $@

##################################################################################################################
# RELEASE
$(SOURCE_RELEASE_OBJS):
	$(call colorecho,$(COLOR_GREEN),"[ Make release object: $(subst $(RELEASE_DIR),,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@)) ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(RELEASE_FLAGS) -c $(subst $(RELEASE_DIR),$(S_DIR),$(@:.o=.cpp)) -o $@

$(TEST_SOURCE_RELEASE_OBJS):
	$(call colorecho,$(COLOR_GREEN),"[ Make release object: $(subst $(RELEASE_DIR),,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@)) ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(RELEASE_FLAGS) -c $(subst $(RELEASE_DIR)/$(O_TEST_DIR),$(S_TEST_DIR),$(@:.o=.cpp)) -o $@

# Clean
clean: clean_debug clean_release

clean_debug:
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete debug obj files ]")
	@rm -f -R $(O_DEBUG_DIR)
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete debug executable files ]")
	@rm -f -R $(O_DEBUG_PROG)
	
clean_release:
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete release obj files ]")
	@rm -f -R $(O_RELEASE_DIR)
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete release executable files ]")
	@rm -f -R $(O_RELEASE_PROG)
