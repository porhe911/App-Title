// SPDX-License-Identifier: GPL-3.0-or-later
#include "app_title.hpp"
#include "media_launcher.hpp"
#include "stable_shellui_injector.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
extern "C" {
#include <ps5/kernel.h>
int sceKernelSetProcessName(const char*);
int sceKernelSendNotificationRequest(int,const void*,std::size_t,int);
extern const unsigned char apptitle_shellui_elf[];
extern const std::size_t apptitle_shellui_elf_size;
}
namespace app_title {
int find_shellui() {
    int mib[]={1,14,8,0};std::size_t size=0;
    if(syscall(202,mib,4,nullptr,&size,nullptr,0)!=0||size==0||size>32*1024*1024)return -1;
    size+=size/4+4096;auto* bytes=static_cast<unsigned char*>(std::malloc(size));if(!bytes)return -1;
    std::size_t used=size;int result=-1;
    if(syscall(202,mib,4,bytes,&used,nullptr,0)==0&&used<=size)for(std::size_t off=0;off+4<=used;) {
        int length=0;std::memcpy(&length,bytes+off,4);if(length<=0||static_cast<std::size_t>(length)>used-off)break;
        if(length>=458&&!std::memcmp(bytes+off+447,"SceShellUI",11)) {
            int pid=0;std::memcpy(&pid,bytes+off+72,4);if(pid>0&&pid!=getpid()){result=pid;break;}
        }
        off+=static_cast<std::size_t>(length);
    }
    std::free(bytes);return result;
}
static void active(int pid,const Config& config,unsigned hooks) {
    char text[96];const int n=std::snprintf(text,sizeof(text),"%d %d %d %06x %u\n",pid,config.enabled?1:0,(config.top?1:0)|(config.outside?2:0),config.color,hooks);
    if(n>0&&static_cast<std::size_t>(n)<sizeof(text))atomic_file(kActive,text,static_cast<std::size_t>(n));
}
static void runtime_phase(const Config& config,Stats& stats) {
    RuntimeStatus s{};
    const bool ready=load_runtime_status(stats.shell_pid,s);
    if(ready){stats.queries=s.title_queries;stats.overrides=s.overrides;stats.reloads=s.reloads;stats.reload_errors=s.reload_errors;
        stats.style_state=s.style_state;stats.style_color=s.style_color;stats.style_top=s.style_top&1;stats.style_outside=(s.style_top>>1)&1;stats.style_patches=s.style_patches;stats.style_input=s.style_input;}
    if(ready&&s.reload_errors)stats.phase="Home reload failed; see AppTitle_shellui.log";
    else if(!config.enabled)stats.phase="Disabled";
    else if(ready&&s.style_state==static_cast<unsigned>(StyleState::applied))stats.phase="Title ID style applied";
    else if(ready&&s.style_state==static_cast<unsigned>(StyleState::unsupported))stats.phase="HomeUI style unsupported; ID rendering not confirmed";
    else if(ready&&s.style_state==static_cast<unsigned>(StyleState::invalid))stats.phase="HomeUI validation failed; ID rendering not confirmed";
    else if(ready&&s.style_state==static_cast<unsigned>(StyleState::unavailable))stats.phase="Style callback unavailable; ID rendering not confirmed";
    else if(ready&&s.overrides&&s.reloads)stats.phase="Native title ID override active; refresh requested";
    else stats.phase="Native hooks ready; waiting for home refresh";
}
static void notify(const char* message) {
    struct Request {char pad[45];char msg[3075];} r{};
    std::snprintf(r.msg,sizeof(r.msg),"%s",message);sceKernelSendNotificationRequest(0,&r,sizeof(r),0);
}

}
int main() {
    using namespace app_title;
    if(!prepare_data_directory()){notify("App Title 0.2.14: cannot create /data/AppTitle.");return 1;}
    record_launch("main.enter",0);
    // A held advisory lock prevents duplicate autoload workers, including after a shell restart.
    const int lock=open("/system_tmp/app_title.lock",O_CREAT|O_RDWR|O_NOFOLLOW,0600);
    if(lock<0){const int error=errno;record_launch("lock.open_failed",error);notify("App Title 0.2.14: cannot open its lock. See AppTitle_launch.log.");return 1;}
    if(flock(lock,LOCK_EX|LOCK_NB)!=0) {
        const int error=errno;record_launch("lock.acquire_failed",error);close(lock);
        notify("App Title 0.2.14: lock unavailable; another instance may be running. See AppTitle_launch.log.");return 1;
    }
    reset_log();
    log_fmt("App Title v%s EXPERIMENTAL",kVersion);
    log_line("stage=main.enter lock=acquired");
    record_launch("lock.ready",0);
    (void)sceKernelSetProcessName("App Title");
    (void)syscall(SYS_thr_set_name,-1,"App Title");
    log_fmt("fw=0x%08x",kernel_get_fw_version());
    log_line("stage=config.begin");
    Config config{};
    const bool config_loaded=load_config(config);
    log_fmt("stage=config.read_end loaded=%d enabled=%d position=%s color=%06x placement=%s",config_loaded?1:0,config.enabled?1:0,config.top?"top":"bottom",config.color,config.outside?"outside":"inside");
    if(!config_loaded) {
        log_line("stage=config.defaults_write.begin");
        log_fmt("stage=config.defaults_write.end saved=%d",save_config(config)?1:0);
    }
    Stats stats{};
    record_launch("media.begin",0);
    log_line("stage=media.begin");
    if(!app_title::ps5::ensure_media_launcher(kernel_get_fw_version())) {
        record_launch("media.cleanup_failed",-1);log_line("Media service cleanup or credential restoration failed; stopping");
        notify("App Title 0.2.14: Media setup failed. See AppTitle.log.");return 1;
    }
    log_line("stage=media.end");
    record_launch("settings.begin",0);
    log_line("stage=settings.begin");
    const bool settings_ready=start_settings(config,stats);
    log_fmt("stage=settings.end ready=%d",settings_ready?1:0);
    record_launch(settings_ready?"settings.ready":"settings.failed",settings_ready?0:-1);
    if(!settings_ready)notify("App Title 0.2.14: settings server unavailable. See AppTitle.log.");
    else notify("App Title 0.2.14: settings ready; initializing Title IDs.");
    log_line("backend=native-titleid key=0x7802c916 PNG interception disabled");
    int previous=-1,stable=0;bool online=false,style_reported=false;unsigned style_wait=0;SessionGuard session{};
    log_line("stage=worker.loop");
    for(;;) {
        if(take_settings_change(config)) {
            style_reported=false;style_wait=0;
            if(session.blocked){stats.phase="ShellUI restarted; App Title stopped until reboot";publish_status(stats);continue;}
            if(online){active(stats.shell_pid,config,stats.hooks);runtime_phase(config,stats);}
            else stats.phase="Waiting for compatible ShellUI";
        }
        const int pid=find_shellui();
        if(pid>0 && pid!=previous) {
            previous=pid;stable=0;online=false;stats.hooks=0;stats.shell_pid=pid;
            stats.queries=stats.overrides=stats.reloads=stats.reload_errors=0;
            stats.style_state=0;stats.style_patches=0;stats.style_color=0xffffff;stats.style_top=0;stats.style_outside=0;stats.style_input={};
            // Transient sysctl failures do not discard a live session. A new
            // positive PID starts the readiness gate again.
            if(session.observe(pid)) {
                Config off{};off.enabled=false;active(-1,off,0);
                log_error("ShellUI changed after activation old_pid=%d new_pid=%d; stopped, reinjection blocked until reboot",session.attempted_pid,pid);
                notify("App Title stopped after ShellUI restarted. Reboot before another test.");
            }
            stats.phase=session.blocked?"ShellUI restarted; App Title stopped until reboot":"Waiting for ShellUI";
            if(!session.blocked)log_fmt("ShellUI observed pid=%d; waiting 10 seconds",pid);
        } else if(pid>0&&stable<10)++stable;
        if(pid>0&&stable>=10&&!online&&session.begin(pid)) {
            int marked_pid=0,marked_on=0;
            if(FILE* f=fopen(kActive,"r")){fscanf(f,"%d %d",&marked_pid,&marked_on);fclose(f);}
            // A marker alone is not proof that hooks survived. No reinjection
            // into an already modified process during this boot is attempted.
            if(marked_pid>0||!reserve_session(pid)) {
                stats.phase="Resident session detected; reboot before relaunch";log_line(stats.phase);
                record_launch("session.blocked",marked_pid);
            } else {
                unlink(kRequest);unlink(kStatus);Config off{};off.enabled=false;active(-1,off,0);
                stats.phase="Loading ShellUI component";publish_status(stats);
                log_fmt("stage=inject.begin pid=%d",pid);
                record_launch("inject.begin",pid);
                const auto r=app_title::ps5::inject_shellui_stable(pid,apptitle_shellui_elf,apptitle_shellui_elf_size);
                log_fmt("Inject pid=%d attached=%d loaded=%d imports=%u unresolved=%u first=%s thread=%d detached=%d auth_restored=%d",pid,r.attached,r.elf_loaded,r.imports_resolved,r.imports_unresolved,r.first_unresolved?r.first_unresolved:"",r.pthread_create_ok,r.detached,r.auth_restored);
                if(r.pthread_create_ok&&r.detached&&r.auth_restored) {
                    Request req{};bool received=false;
                    for(unsigned tries=0;tries<100&&!received;++tries) {
                        if(FILE* f=fopen(kRequest,"rb")){received=fread(&req,1,sizeof(req),f)==sizeof(req)&&fgetc(f)==EOF;fclose(f);}
                        if(!received)usleep(100000);
                    }
                    if(received&&install_hooks(pid,req,stats.hooks)) {
                        online=true;active(pid,config,stats.hooks);runtime_phase(config,stats);
                        notify("App Title: hooks ready; refreshing home.");
                    } else {
                        stats.phase="Hook unavailable; see AppTitle.log";
                        RuntimeStatus diagnostic{};
                        if(load_runtime_status(pid,diagnostic))log_fmt("Native bootstrap stage=%u reload_errors=%llu",diagnostic.stage,static_cast<unsigned long long>(diagnostic.reload_errors));
                    }
                } else stats.phase="Injection unavailable; see AppTitle.log";
            }
            log_fmt("Startup complete pid=%d online=%d hooks=%u; periodic logging disabled",pid,online?1:0,stats.hooks);
            record_launch(online?"native.ready":"native.unavailable",static_cast<int>(stats.hooks));
            finish_startup_log();
        }
        if(online) {
            runtime_phase(config,stats);
            // One replaceable result at startup or after an explicit save.
            // No text writes occur again until settings change.
            if(!style_reported)++style_wait;
            if(!style_reported&&(style_wait>=30||(style_wait>=8&&(!config.enabled||stats.style_state>=static_cast<unsigned>(StyleState::applied))))) {
                log_home_result("HomeUI result renderer=rnps-reserved-space style_state=%u patches=%llu title_queries=%llu overrides=%llu reloads=%llu reload_errors=%llu requested=%s/%s/%06x applied=%s/%s/%06x input_format=%u buffer=%u payload=%u modules=%u startup=%u failure=%u failed_module=%u module_size=%u",stats.style_state,static_cast<unsigned long long>(stats.style_patches),static_cast<unsigned long long>(stats.queries),static_cast<unsigned long long>(stats.overrides),static_cast<unsigned long long>(stats.reloads),static_cast<unsigned long long>(stats.reload_errors),config.outside?"outside":"inside",config.top?"top":"bottom",config.color,stats.style_outside?"outside":"inside",stats.style_top?"top":"bottom",stats.style_color,stats.style_input.format,stats.style_input.buffer_size,stats.style_input.payload_size,stats.style_input.modules,stats.style_input.startup_size,stats.style_input.failure,stats.style_input.failed_module,stats.style_input.module_size);
                style_reported=true;
            }
        }
        run_style_export();
        publish_status(stats);usleep(1000000);
    }
}

