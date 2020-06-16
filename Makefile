#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

#IDF_PATH=$(realpath ../esp-idf)
PROJECT_NAME := WiFiHeater
EXTRA_COMPONENT_DIRS := $(realpath ..) $(realpath ../esp-idf-overrides)

# Call CPPUTEST makefile if any of these targets are used
CPPUTEST_TARGETS := test all_no_tests

ifeq ($(MAKECMDGOALS), $(filter-out $(CPPUTEST_TARGETS),$(MAKECMDGOALS)))
  ## ESP-IDF build
  include $(IDF_PATH)/make/project.mk

else
  ## CPPUTEST build
  include Tests/cpputest.mk
endif
