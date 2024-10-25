#
# Voice Signal Preprocess
# Copyright (C) 2001-2020 NationalChip Co., Ltd
# ALL RIGHTS RESERVED!
#
# Makefile: lvp/common source list
#
#=================================================================================#

ifeq ($(CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION), y)
-include lvp/vui/kws/Makefile
#vui_objs += $(kws_objs)
endif

ifeq ($(CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION), y)
-include lvp/vui/vpr/Makefile
vui_objs += $(vpr_objs)
endif

ifeq ($(CONFIG_LVP_ENABLE_NN_DENOISE), y)
-include lvp/vui/denoise/Makefile
vui_objs += $(denoise_objs)
endif

ifeq ($(CONFIG_LVP_ENABLE_SED), y)
-include lvp/vui/sed/Makefile
vui_objs += $(sed_objs)
endif

__vui_config_start_line=$(shell sed -n '/VUI\ Settings/=' include/autoconf.h)
__vui_config_end_line=$(shell sed -n '/LVP\ Application\ Settings/=' include/autoconf.h)
build: lib/libvui.a
lib/libvui.a:$(vui_objs)
	@echo $(__vui_config_start_line) $(__vui_config_end_line)
ifneq ($(__vui_config_start_line), )
	@echo "#ifndef __VUI_CONF_H__" > include/vui/vuiconf.h
	@echo "#define __VUI_CONF_H__" >> include/vui/vuiconf.h
	@sed -n '$(__vui_config_start_line),$(__vui_config_end_line)p' include/autoconf.h >> include/vui/vuiconf.h
	@echo "#endif // __VUI_CONF_H__" >> include/vui/vuiconf.h
endif
	@mkdir -p lib/
	@rm -f $@
	@echo [$(AR) archiving $@]
	@$(AR) $(ARFLAGS) $@ $^
	@echo "-make whitout vui source code-" > ./lib/vui_readme.txt
	@echo "cp libvui.a lib" >> ./lib/vui_readme.txt
	@echo "cp vuiconf.h include/vui" >> ./lib/vui_readme.txt
	@echo "cp .config .config" >> ./lib/vui_readme.txt
	@echo "make novuiconfig # do not edit this: Sram kept for NPU (KBytes)" >> ./lib/vui_readme.txt
	@echo "mv lvp/vui/ ." >> ./lib/vui_readme.txt
	@echo "make clean;make" >> ./lib/vui_readme.txt
	@echo "mv vui/ lvp/" >> ./lib/vui_readme.txt
	@zip -q -j ./lib/vui.zip ./lib/vui_readme.txt lib/libvui.a include/vui/vuiconf.h .config
	@rm ./lib/vui_readme.txt


