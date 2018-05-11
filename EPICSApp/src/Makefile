TOP = ../..
include $(TOP)/configure/CONFIG

# extract visual studio major version number from path
# unfortunately cannot use %.% so using %.0 but may not match minor version number changes e.g. 10.1

VS_VERS=$(filter %.0,$(subst \, ,$(VisualStudioVersion)))
VS_MAJOR=$(word 1,$(subst ., ,$(VS_VERS)))

INSTALL_DIR=$(TOP)/install/$(EPICS_HOST_ARCH)
# -DEXTRA_LIBRARIES="$(OPENSSL)/lib/$(EPICS_HOST_ARCH)/libssl.a;$(ZLIB)/lib/$(EPICS_HOST_ARCH)/libzlib.a"
ifeq ($(OS_CLASS),WIN32)
EXTRA_LIBRARIES = $(ZLIB)/lib/$(EPICS_HOST_ARCH)/libzlib.a
EXTRA_LIBRARIES += $(foreach elib,$(EPICS_BASE_IOC_LIBS),$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH)/$(elib).lib)
else
EXTRA_LIBRARIES = /usr/lib64/libssl.so /usr/lib64/libcrypto.so;/usr/lib64/libsasl2.so $(ZLIB)/lib/$(EPICS_HOST_ARCH)/libzlib.a /usr/lib64/libdl.so
EXTRA_LIBRARIES += $(foreach elib,$(EPICS_BASE_IOC_LIBS),$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH)/lib$(elib).a)
endif
CMAKE_BUILD_FLAGS=
CMAKE_CONFIG_FLAGS = -DEPICS_MODULES_PATH="$(EV4_BASE)" -DCMAKE_LIBRARY_PATH="$(LIBRDKAFKA)/lib/$(EPICS_HOST_ARCH);$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH)" -DCMAKE_INCLUDE_PATH="$(FLATBUFFERS)/include;$(LIBRDKAFKA)/include;$(EPICS_BASE)/include;$(FMT);$(RAPIDJSON)/include" -DCMAKE_PROGRAM_PATH="$(FLATBUFFERS)/bin/$(EPICS_HOST_ARCH)" -Dpath_include_streaming_data_types="$(STREAMINGDATATYPES)"

blank :=
space := $(blank) $(blank)
CMAKE_CONFIG_FLAGS += -DFETK_EXTRA_LIBRARIES="$(subst $(space),;,$(EXTRA_LIBRARIES))"
ifneq ($(findstring static,$(EPICS_HOST_ARCH)),)
CMAKE_CONFIG_FLAGS += -DFETK_BUILD_TYPE=STATIC -DRDKAFKA_BUILD_TYPE:STRING=STATIC
endif

ifneq ($(findstring linux,$(EPICS_HOST_ARCH)),)
CMAKE_GENERATOR=Unix Makefiles
else
ifneq ($(findstring windows,$(EPICS_HOST_ARCH)),)
VS_ARCH=$(VS_MAJOR) Win64
PLATFORM=x64
else
VS_ARCH=$(VS_MAJOR)
PLATFORM=Win32
endif
CMAKE_GENERATOR=Visual Studio $(VS_ARCH)
endif
ifneq ($(findstring debug,$(EPICS_HOST_ARCH)),)
CMAKE_CONFIG_FLAGS += -DCMAKE_BUILD_TYPE:STRING=Debug
CMAKE_CONFIG=Debug
else
CMAKE_CONFIG_FLAGS += -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo
CMAKE_CONFIG=RelWithDebInfo
endif

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

ifdef T_A
install:
	cmake $(TOP) -G "$(CMAKE_GENERATOR)" -DCMAKE_INSTALL_PREFIX:PATH="$(INSTALL_DIR)" $(CMAKE_CONFIG_FLAGS)
	-$(MKDIR) $(TOP)/bin
	-$(MKDIR) $(TOP)/bin/$(EPICS_HOST_ARCH)
	cmake --build . --config $(CMAKE_CONFIG) $(CMAKE_BUILD_FLAGS)
ifneq ($(wildcard $(CMAKE_CONFIG)/forward-epics-to-kafka$(EXE)),)
	$(CP) -f $(CMAKE_CONFIG)/forward-epics-to-kafka$(EXE) $(TOP)/bin/$(EPICS_HOST_ARCH)/
endif
ifneq ($(wildcard forward-epics-to-kafka$(EXE)),)
	$(CP) -f forward-epics-to-kafka$(EXE) $(TOP)/bin/$(EPICS_HOST_ARCH)/
endif
endif
# using    --target install    with --build above seems to no longer work?
#	-$(CP) -f $(INSTALL_DIR)/bin/* $(TOP)/bin/$(EPICS_HOST_ARCH)/

uninstall:
	$(RMDIR) $(INSTALL_DIR)
