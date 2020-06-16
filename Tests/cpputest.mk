#Set this to @ to keep the makefile quiet
ifndef SILENCE
	SILENCE = @
endif

#--- Inputs ----#
COMPONENT_NAME = $(PROJECT_NAME)_test
CPPUTEST_HOME = Tests/cpputest-3.8
CPPUTEST_LIB_DIR = build/Tests
CPPUTEST_OBJS_DIR = build/Tests

CPPUTEST_USE_EXTENSIONS = Y
CPP_PLATFORM = Gcc
CPPUTEST_LDFLAGS = -lpthread
CPPUTEST_CPPFLAGS = -DUNITTESTS  # Needed to trigger FreeRTOS mock headers
CPPUTEST_WARNINGFLAGS = -w


# Looks up our custom local components
LOCAL_COMPONENTS := $(patsubst %/.,%,$(wildcard components/*/.)) main

# Filter out components which are untestable
UNTESTABLE_COMPONENTS := components/Drivers components/Socket components/StatusDbg components/Crypto main components/AzureEsp32Pal components/FGT 
LOCAL_COMPONENTS := $(filter-out $(UNTESTABLE_COMPONENTS),$(LOCAL_COMPONENTS))

# This line is overriding the default new macros.  This is helpful
# when using std library includes like <list> and other containers
# so that memory leak detection does not conflict with stl.
#CPPUTEST_MEMLEAK_DETECTOR_NEW_MACRO_FILE = -include ApplicationLib/ExamplesNewOverrides.h

# This will work as long as no includes outside of our components are used.
# FIXME: find solution for when such includes are needed (Drivers etc)
SRC_DIRS = $(LOCAL_COMPONENTS)

# # Include source files to be tested
# SRC_FILES = \
# 	../src/CircularBuffer.cpp\
# 	../src/ExampleTask.cpp\
# 	../src/ZentriDriver.cpp\
# 	../src/ZentriResponseHeader.cpp\


# TEST_SRC_DIRS = \
# 	.\

TEST_SRC_FILES = $(foreach lc,$(LOCAL_COMPONENTS),$(wildcard $(lc)/Tests/*Test.cpp)) Tests/mainRunAllTests.cpp

# FIXME: separate this out in each component?
MOCKS_SRC_DIRS = Tests/mock

INCLUDE_DIRS = $(foreach lc,$(LOCAL_COMPONENTS),$(lc)/include) $(CPPUTEST_HOME)/include Tests/

# This would add IDF includes (but causes unresolved error with cygwin/types.h)
#IDF_COMPONENTS := $(patsubst %/.,%,$(wildcard $(IDF_PATH)/components/*/.))
#IDF_INCLUDES := $(foreach ic,$(IDF_COMPONENTS),$(ic)/include)
#INCLUDE_DIRS += $(IDF_INCLUDES) /usr/include

include $(CPPUTEST_HOME)/build/MakefileWorker.mk
