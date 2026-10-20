PS5_PAYLOAD_SDK ?= /opt/ps5-payload-sdk
CC := $(PS5_PAYLOAD_SDK)/bin/prospero-clang
CXX := $(PS5_PAYLOAD_SDK)/bin/prospero-clang++
FLAGS := -O2 -fPIC -ffunction-sections -fdata-sections -MMD -MP -Wall -Wextra -Iinclude -Isrc -Ivendor/loader -Ivendor/shsrv -Ivendor/hde -Ivendor/stb
CXXFLAGS := $(FLAGS) -std=c++17 -fno-exceptions -fno-rtti
# Only the shared component uses RegMgr/Mono. DT_NEEDED on the controller is
# processed by the SDK before main (and before any log or HTTP listener).
LDLIBS := -lkernel_sys -lSceLibcInternal -lSceNet
SHELLUI_LDLIBS := -lSceRegMgr -lmonosgen-2.0 $(LDLIBS)
CSRC := vendor/shsrv/pt.c vendor/loader/stable_elfldr_bridge.c src/style_probe.c
CPPSRC := src/main.cpp src/core.cpp src/paths.cpp src/hooks.cpp src/relocate.cpp src/settings.cpp src/style_export.cpp src/media_launcher.cpp vendor/hde/hde64.cpp vendor/loader/stable_shellui_injector.cpp vendor/loader/remote_call.cpp build/shellui_blob.cpp build/html.cpp build/param.cpp build/icon.cpp
OBJ := $(addprefix build/obj/,$(CSRC:.c=.o) $(CPPSRC:.cpp=.o))

.PHONY: all clean verify-elf host-test media-test hooks-test shellui-test runtime-test style-test export-test home-style-test
all: dist/App_Title_v0.2.8.elf verify-elf
build/App_Title_ShellUI.elf: src/shellui.cpp src/paths.cpp src/home_style.cpp src/home_style_1360.inc src/home_style.hpp include/app_title.hpp Makefile
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -fvisibility=hidden -shared -Wl,-Bsymbolic -Wl,-e,elf_main -Wl,-z,now -Wl,--hash-style=sysv -Wl,--gc-sections -Wl,-soname,libAppTitle.so src/shellui.cpp src/paths.cpp src/home_style.cpp $(SHELLUI_LDLIBS) -o $@
build/shellui_blob.cpp: build/App_Title_ShellUI.elf tools/embed.py
	python3 tools/embed.py $< $@ apptitle_shellui_elf
build/html.cpp: assets/index.html tools/embed.py
	python3 tools/embed.py $< $@ apptitle_settings_html
build/param.cpp: assets/param.json tools/embed.py
	python3 tools/embed.py $< $@ apptitle_settings_param
build/icon.cpp: assets/icon0.png tools/embed.py
	python3 tools/embed.py $< $@ apptitle_settings_icon
build/obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@
dist/App_Title_v0.2.8.elf: $(OBJ) Makefile
	@mkdir -p dist
	$(CXX) -Wl,--gc-sections $(OBJ) $(LDLIBS) -o $@
verify-elf: dist/App_Title_v0.2.8.elf build/App_Title_ShellUI.elf
	python3 tools/verify_elf.py
host-test:
	@mkdir -p build-host
	g++ -std=c++17 -O2 -Wall -Wextra -Iinclude -Ivendor/hde tests/test_core.cpp src/core.cpp src/paths.cpp src/relocate.cpp vendor/hde/hde64.cpp -o build-host/test_core
	./build-host/test_core
media-test: build/param.cpp build/icon.cpp
	@mkdir -p build-host
	g++ -std=c++17 -O2 -Wall -Wextra -DAPP_TITLE_HOST_TEST -Itests/mock -Iinclude -Isrc tests/test_media.cpp src/media_launcher.cpp build/param.cpp build/icon.cpp -o build-host/test_media
	./build-host/test_media
hooks-test:
	@mkdir -p build-host
	g++ -std=c++17 -O2 -Wall -Wextra -Itests/mock -Iinclude -Ivendor/hde -Ivendor/shsrv tests/test_hooks.cpp src/hooks.cpp src/relocate.cpp vendor/hde/hde64.cpp -o build-host/test_hooks
	./build-host/test_hooks
shellui-test:
	@mkdir -p build-host
	g++ -std=c++17 -O2 -Wall -Wextra -DAPP_TITLE_HOST_TEST -Iinclude tests/test_shellui.cpp src/paths.cpp src/home_style.cpp -o build-host/test_shellui
	./build-host/test_shellui $(if $(HOME_UI),"$(HOME_UI)")
runtime-test:
	@mkdir -p build-host
	g++ -std=c++17 -O2 -Wall -Wextra -DAPP_TITLE_HOST_TEST -Iinclude tests/test_runtime.cpp src/core.cpp src/paths.cpp -pthread -o build-host/test_runtime
	./build-host/test_runtime
style-test:
	@mkdir -p build-host/style
	gcc -O2 -Wall -Wextra -Werror -D_DEFAULT_SOURCE -std=c11 -Isrc tests/test_style_probe.c src/style_probe.c -o build-host/test_style_probe
	./build-host/test_style_probe
export-test:
	@mkdir -p build-host
	g++ -O2 -Wall -Wextra -Werror -std=c++17 -DAPP_TITLE_HOST_TEST -Iinclude -Isrc tests/test_export.cpp src/style_export.cpp -o build-host/test_export
	./build-host/test_export
home-style-test:
	@mkdir -p build-host
	g++ -O2 -Wall -Wextra -Werror -std=c++17 -Iinclude -Isrc tests/test_home_style.cpp src/home_style.cpp -o build-host/test_home_style
	./build-host/test_home_style $(if $(HOME_UI),"$(HOME_UI)")
clean:
	rm -rf build build-host dist

-include $(OBJ:.o=.d)
