// App Title for PS5. Copyright (C) 2026 porhe911
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>

namespace app_title {
constexpr char kVersion[] = "0.2.13";
#ifdef APP_TITLE_HOST_TEST
constexpr char kDirectory[] = "./build-host/AppTitle";
constexpr char kConfig[] = "./build-host/config.ini";
constexpr char kCache[] = "./build-host/cache";
constexpr char kLog[] = "./build-host/AppTitle/AppTitle.log";
constexpr char kLaunchLog[] = "./build-host/AppTitle/AppTitle_launch.log";
constexpr char kSession[] = "./build-host/app_title_session";
#else
constexpr char kDirectory[] = "/data/AppTitle";
constexpr char kConfig[] = "/data/AppTitle/config.ini";
constexpr char kCache[] = "/data/AppTitle/cache";
constexpr char kLog[] = "/data/AppTitle/AppTitle.log";
constexpr char kLaunchLog[] = "/data/AppTitle/AppTitle_launch.log";
constexpr char kSession[] = "/system_tmp/app_title_session";
#endif
constexpr char kRequest[] = "/system_tmp/app_title_request.bin";
constexpr char kActive[] = "/system_tmp/app_title_active";
constexpr char kStatus[] = "/system_tmp/app_title_status.bin";
constexpr unsigned kPort = 39031;
constexpr std::uint64_t kMagic = 0x3354495444505041ULL;
constexpr std::uint64_t kStatusMagic = 0x3753544954505041ULL;
constexpr unsigned kRequiredHooks = 2;
constexpr unsigned kHooks = 3;
constexpr long kTitleIdKey = 2013448470L;
constexpr unsigned kLogLimit = 8*1024;
struct SessionGuard {
    int attempted_pid=-1;bool blocked=false;
    bool observe(int pid) {
        if(attempted_pid>0&&pid>0&&pid!=attempted_pid&&!blocked){blocked=true;return true;}
        return false;
    }
    bool begin(int pid) {
        if(pid<=0||attempted_pid>0||blocked)return false;
        attempted_pid=pid;return true;
    }
};
struct Config { bool enabled = true; bool top = false; std::uint32_t color = 0xffffff; bool outside = true; };
struct Hook { std::uint64_t target, replacement, trampoline; };
struct Request { std::uint64_t magic; std::int32_t pid; std::uint32_t count; Hook hooks[kHooks]; };
static_assert(sizeof(Request)==88,"Native hook request ABI");
enum class StyleState : std::uint32_t { idle=0,waiting=1,applied=2,unsupported=3,invalid=4,unavailable=5 };
// One bounded input description for the startup result, never bundle contents.
// format: 0=unknown, 1=Hermes, 2=RAM; failure: 0=none, 1=length,
// 2=profile header, 3=digest, 4=module metadata, 5=generated replacement.
struct StyleInput {
    std::uint32_t format=0,buffer_size=0,payload_size=0,modules=0,startup_size=0,failure=0;
    std::uint32_t failed_module=0,module_size=0;
};
struct RuntimeStatus {
    std::uint64_t magic=kStatusMagic;
    std::int32_t pid=0;std::uint32_t enabled=0;
    std::uint64_t registry_calls=0,title_queries=0,overrides=0,reloads=0,reload_errors=0;
    std::uint32_t stage=0,style_state=0;
    // style_top is a position bitfield: bit 0 = top, bit 1 = outside.
    std::uint32_t style_color=0xffffff,style_top=0;
    std::uint64_t style_patches=0;
    StyleInput style_input{};
};
static_assert(sizeof(RuntimeStatus)==112,"Shared status ABI");
struct Stats {
    unsigned icons=0, failed=0, hooks=0;int shell_pid=-1;
    const char* phase="Starting";std::uint64_t queries=0,overrides=0,reloads=0,reload_errors=0;
    std::uint32_t style_state=0,style_color=0xffffff,style_top=0,style_outside=0;
    std::uint64_t style_patches=0;
    StyleInput style_input{};
};

bool valid_id(const char* text, std::size_t size);
bool redirect_path(const char* original, char* output, std::size_t size);
bool load_config(Config& config, const char* path=kConfig);
bool save_config(const Config& config, const char* path=kConfig);
bool parse_settings(const char* body, Config& config);
bool atomic_file(const char* path, const void* data, std::size_t size);
bool load_runtime_status(int pid, RuntimeStatus& status, const char* path=kStatus);
void log_line(const char* text);
void log_fmt(const char* format, ...);
void reset_log(const char* path=kLog);
void finish_startup_log();
void log_error(const char* format, ...);
// One short snapshot per launch/stage, never a periodic log. Independent of
// the resident worker's main journal so rejected relaunches remain visible.
bool record_launch(const char* stage,int result,const char* path=kLaunchLog);
bool prepare_data_directory(const char* path=kDirectory);
bool request_style_export();
void run_style_export();
const char* style_export_state();
bool reserve_session(int pid,const char* path=kSession);
bool render_icon(const char* source, const char* output, const char* id,
                 const Config& config, const char* font);
void scan_icons(const Config& config, Stats& stats);
bool start_settings(Config& config, Stats& stats);
bool take_settings_change(Config& config);
void publish_status(const Stats& stats);
bool install_hooks(int pid, const Request& request, unsigned& installed);
int find_shellui();
}

