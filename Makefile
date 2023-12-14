#doc :
#	@mkdir -p $(DOC_DIR)
#	@doxygen Doxyfile

include $(MAPPER_HOME)/scripts/config.mk
-include $(MAPPER_HOME)/include/config/auto.conf
-include $(MAPPER_HOME)/include/config/auto.conf.cmd
SHARE = 1
NAME = mapperPass
CXXSRCS += $(shell find ./src -name "*.cpp")
CFLAGS_BUILD += $(call remove_quote,$(CONFIG_CC_OPT))
CFLAGS_BUILD += $(if $(CONFIG_CC_DEBUG),-Og -ggdb3,)
CFLAGS += $(shell llvm-config-12 --cxxflags) $(CFLAGS_BUILD)
LDFLAGS += $(shell llvm-config-12 --ldflags) $(CFLAGS_BUILD)
include $(MAPPER_HOME)/scripts/build.mk
