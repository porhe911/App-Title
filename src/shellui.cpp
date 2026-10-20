// SPDX-License-Identifier: GPL-3.0-or-later
// Native Title ID mechanism follows etaHEN's registry override and home reload.
// GPL-3.0-or-later reference: etaHEN / LightningMods, 2025.
#include "app_title.hpp"
#include "home_style.hpp"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <unistd.h>
struct MonoDomain;struct MonoThread;struct MonoAssembly;struct MonoImage;
struct MonoClass;struct MonoMethod;struct MonoObject;struct MonoString;
struct MonoMethodSignature;struct MonoType;
extern "C" {
int sceRegMgrGetInt(long,int*);
int ioctl(int,unsigned long,...);
MonoDomain* mono_get_root_domain();
MonoThread* mono_thread_attach(MonoDomain*);
void mono_thread_detach(MonoThread*);
MonoAssembly* mono_domain_assembly_open(MonoDomain*,const char*);
MonoImage* mono_assembly_get_image(MonoAssembly*);
MonoClass* mono_class_from_name(MonoImage*,const char*,const char*);
MonoMethod* mono_class_get_method_from_name(MonoClass*,const char*,int);
std::uint32_t mono_method_get_flags(MonoMethod*,std::uint32_t*);
MonoMethodSignature* mono_method_signature(MonoMethod*);
unsigned mono_signature_get_param_count(MonoMethodSignature*);
MonoType* mono_signature_get_return_type(MonoMethodSignature*);
MonoType* mono_signature_get_params(MonoMethodSignature*,void**);
int mono_type_get_type(MonoType*);
int mono_type_is_byref(MonoType*);
void* mono_compile_method(MonoMethod*);
MonoObject* mono_runtime_invoke(MonoMethod*,void*,void**,MonoObject**);
MonoString* mono_string_new(MonoDomain*,const char*);
std::uint32_t mono_gchandle_new(MonoObject*,int);
MonoObject* mono_gchandle_get_target(std::uint32_t);
void mono_gchandle_free(std::uint32_t);
}
namespace {
std::atomic_bool enabled{false},reload_pending{false};
std::atomic_flag reloading=ATOMIC_FLAG_INIT;
std::atomic<unsigned long long> registry_calls{0},title_queries{0},overrides{0},reloads{0},reload_errors{0};
std::atomic<unsigned> stage{1};
std::atomic<std::uint32_t> style_config{0xffffff},applied_config{0xffffff};
std::atomic<app_title::StyleState> style_state{app_title::StyleState::idle};
std::atomic_bool style_hook_ready{false};
std::atomic<unsigned long long> style_patches{0};
MonoDomain* root_domain=nullptr;MonoMethod* reload_method=nullptr;
#define TRAMP(name) extern "C" __attribute__((naked,noinline,used,aligned(16))) void name(){__asm__ volatile(".rept 256\n nop\n .endr\n ret\n");}
TRAMP(registry_trampoline)
TRAMP(update_trampoline)
TRAMP(ioctl_trampoline)
using Registry=int(*)(long,int*);
using Update=void(*)(MonoObject*);
using Ioctl=int(*)(int,unsigned long,void*);
struct RnpsArgs {void* buffer;int size;std::uint32_t error;};
static_assert(sizeof(RnpsArgs)==16&&offsetof(RnpsArgs,size)==8&&offsetof(RnpsArgs,error)==12,"RNPS ABI");
int registry_get(long key,int* out,Registry original) {
    registry_calls.fetch_add(1,std::memory_order_relaxed);
    if(key==app_title::kTitleIdKey) {
        title_queries.fetch_add(1,std::memory_order_relaxed);
        if(enabled.load(std::memory_order_acquire)) {
            if(out)*out=1;
            overrides.fetch_add(1,std::memory_order_relaxed);return 0;
        }
    }
    return original(key,out);
}
int registry_hook(long key,int* out){return registry_get(key,out,reinterpret_cast<Registry>(&registry_trampoline));}
int ioctl_get(int fd,unsigned long command,void* argument,Ioctl original) {
    const int result=original(fd,command,argument);
    if(result==0&&command==0xc0105203UL&&argument&&enabled.load(std::memory_order_acquire)&&style_hook_ready.load(std::memory_order_acquire)) {
        const auto* args=static_cast<const RnpsArgs*>(argument);
        if(args->error==0&&args->buffer&&args->size>0) {
            const auto packed=style_config.load(std::memory_order_acquire);
            app_title::Config config{};config.top=(packed&0x1000000u)!=0;config.outside=(packed&0x2000000u)!=0;config.color=packed&0xffffffu;
            const auto patched=app_title::patch_home_style(args->buffer,static_cast<std::size_t>(args->size),config);
            if(patched==app_title::HomeStyleResult::applied) {
                applied_config.store(packed,std::memory_order_relaxed);style_patches.fetch_add(1,std::memory_order_relaxed);
                style_state.store(app_title::StyleState::applied,std::memory_order_release);
            } else if(patched==app_title::HomeStyleResult::unsupported)style_state.store(app_title::StyleState::unsupported,std::memory_order_release);
            else if(patched==app_title::HomeStyleResult::invalid)style_state.store(app_title::StyleState::invalid,std::memory_order_release);
        }
    }
    return result;
}
int ioctl_hook(int fd,unsigned long command,void* argument){return ioctl_get(fd,command,argument,reinterpret_cast<Ioctl>(&ioctl_trampoline));}
void run_update(MonoObject* instance,Update original) {
    original(instance);
    if(!reload_pending.load(std::memory_order_acquire)||reloading.test_and_set(std::memory_order_acquire))return;
    // Consume before invocation: a scene reload may re-enter an Update callback.
    if(reload_pending.exchange(false,std::memory_order_acq_rel)) {
        MonoString* name=root_domain?mono_string_new(root_domain,"NPXS40002"):nullptr;
        const auto handle=name?mono_gchandle_new(reinterpret_cast<MonoObject*>(name),1):0;
        MonoObject* exception=nullptr;
        if(handle&&reload_method) {
            void* args[]={mono_gchandle_get_target(handle)};
            if(args[0]) {
                (void)mono_runtime_invoke(reload_method,nullptr,args,&exception);
                if(exception)reload_errors.fetch_add(1,std::memory_order_relaxed);
                else reloads.fetch_add(1,std::memory_order_relaxed);
            } else reload_errors.fetch_add(1,std::memory_order_relaxed);
        } else reload_errors.fetch_add(1,std::memory_order_relaxed);
        if(handle)mono_gchandle_free(handle);
    }
    reloading.clear(std::memory_order_release);
}
void update_hook(MonoObject* instance){run_update(instance,reinterpret_cast<Update>(&update_trampoline));}
void activation(bool valid,const app_title::Config& config,unsigned hooks) {
    static bool seen=false;static bool previous=false;static std::uint32_t previous_style=0;static unsigned previous_hooks=0;
    const auto packed=(config.color&0xffffffu)|(config.top?0x1000000u:0u)|(config.outside?0x2000000u:0u);
    style_config.store(packed,std::memory_order_release);
    style_hook_ready.store(valid&&hooks==app_title::kHooks,std::memory_order_release);
    enabled.store(valid&&config.enabled,std::memory_order_release);
    if(valid&&(!seen||previous!=config.enabled||previous_style!=packed||previous_hooks!=hooks)) {
        previous=config.enabled;previous_style=packed;previous_hooks=hooks;seen=true;stage.store(6,std::memory_order_relaxed);
        style_state.store(!config.enabled?app_title::StyleState::idle:hooks==app_title::kHooks?app_title::StyleState::waiting:app_title::StyleState::unavailable,std::memory_order_release);
        reload_pending.store(true,std::memory_order_release);
    }
}
MonoImage* image(const char* path) {
    auto* a=mono_domain_assembly_open(root_domain,path);return a?mono_assembly_get_image(a):nullptr;
}
bool signature(MonoMethod* method,bool takes_string) {
    auto* sig=mono_method_signature(method);
    if(!sig||mono_signature_get_param_count(sig)!=(takes_string?1u:0u))return false;
    auto* result=mono_signature_get_return_type(sig);
    if(!result||mono_type_get_type(result)!=0x01||mono_type_is_byref(result))return false;
    if(takes_string) {
        void* iterator=nullptr;auto* arg=mono_signature_get_params(sig,&iterator);
        if(!arg||mono_type_get_type(arg)!=0x0e||mono_type_is_byref(arg))return false;
    }
    return true;
}
bool prepare_request(app_title::Request& r) {
    root_domain=mono_get_root_domain();
    if(!root_domain){stage.store(100);return false;}
    // Only metadata/JIT preparation needs this native thread attached to Mono.
    struct Attachment {
        MonoThread* thread;
        ~Attachment(){if(thread)mono_thread_detach(thread);}
    } attachment{mono_thread_attach(root_domain)};
    if(!attachment.thread){stage.store(101);return false;}
    auto* react=image("/system_ex/common_ex/lib/ReactNative.Vsh.Common.dll");
    if(!react){stage.store(102);return false;}
    auto* manager=mono_class_from_name(react,"ReactNative.Vsh.Common","ReactApplicationSceneManager");
    reload_method=manager?mono_class_get_method_from_name(manager,"ReloadApp",1):nullptr;
    std::uint32_t impl=0;
    if(!reload_method||!(mono_method_get_flags(reload_method,&impl)&0x10u)||!signature(reload_method,true)){stage.store(103);return false;}
    auto* pui=image("/system_ex/common_ex/lib/Sce.PlayStation.PUI.dll");
    if(!pui){stage.store(104);return false;}
    auto* app=mono_class_from_name(pui,"Sce.PlayStation.PUI","Application");
    auto* update=app?mono_class_get_method_from_name(app,"Update",0):nullptr;
    if(!update||(mono_method_get_flags(update,&impl)&0x10u)||!signature(update,false)){stage.store(105);return false;}
    auto* address=mono_compile_method(update);
    if(!address){stage.store(106);return false;}
    r={};r.magic=app_title::kMagic;r.pid=getpid();r.count=app_title::kHooks;
    r.hooks[0]={reinterpret_cast<std::uintptr_t>(&sceRegMgrGetInt),reinterpret_cast<std::uintptr_t>(&registry_hook),reinterpret_cast<std::uintptr_t>(&registry_trampoline)};
    r.hooks[1]={reinterpret_cast<std::uintptr_t>(address),reinterpret_cast<std::uintptr_t>(&update_hook),reinterpret_cast<std::uintptr_t>(&update_trampoline)};
    r.hooks[2]={reinterpret_cast<std::uintptr_t>(&ioctl),reinterpret_cast<std::uintptr_t>(&ioctl_hook),reinterpret_cast<std::uintptr_t>(&ioctl_trampoline)};
    stage.store(5);return true;
}
app_title::RuntimeStatus snapshot() {
    app_title::RuntimeStatus s{};s.pid=getpid();s.enabled=enabled.load()?1:0;
    s.registry_calls=registry_calls.load();s.title_queries=title_queries.load();s.overrides=overrides.load();
    s.reloads=reloads.load();s.reload_errors=reload_errors.load();s.stage=stage.load();
    s.style_state=static_cast<std::uint32_t>(style_state.load(std::memory_order_acquire));
    const auto applied=applied_config.load(std::memory_order_relaxed);s.style_color=applied&0xffffffu;s.style_top=(applied>>24)&3;
    s.style_patches=style_patches.load();return s;
}
void write_diagnostics(const app_title::RuntimeStatus& s,const char* path) {
    char text[512];const int n=std::snprintf(text,sizeof(text),
        "App Title v%s native-titleid pid=%d enabled=%u stage=%u\nkey=0x7802c916 home=NPXS40002\nregistry_calls=%llu title_queries=%llu overrides=%llu reloads=%llu reload_errors=%llu\nstyle_state=%u patches=%llu applied_position=%s applied_color=%06x applied_placement=%s\n",
        app_title::kVersion,s.pid,s.enabled,s.stage,static_cast<unsigned long long>(s.registry_calls),static_cast<unsigned long long>(s.title_queries),static_cast<unsigned long long>(s.overrides),static_cast<unsigned long long>(s.reloads),static_cast<unsigned long long>(s.reload_errors),s.style_state,static_cast<unsigned long long>(s.style_patches),(s.style_top&1)?"top":"bottom",s.style_color,(s.style_top&2)?"outside":"inside");
    if(n>0&&static_cast<std::size_t>(n)<sizeof(text))(void)app_title::atomic_file(path,text,static_cast<std::size_t>(n));
}
struct RuntimePublisher {
    app_title::RuntimeStatus previous{};bool have=false,diagnostic_written=false,style_diagnostic_written=false;unsigned elapsed=10;
    void update(const app_title::RuntimeStatus& s,const char* status_path=app_title::kStatus,const char* diagnostic_path="/data/AppTitle/AppTitle_shellui.log") {
        ++elapsed;
        const bool urgent=!have||s.enabled!=previous.enabled||s.stage!=previous.stage||s.style_state!=previous.style_state||s.style_patches!=previous.style_patches;
        const bool changed=s.title_queries!=previous.title_queries||s.overrides!=previous.overrides||s.reloads!=previous.reloads||s.reload_errors!=previous.reload_errors;
        if(urgent||(changed&&elapsed>=10)) {
            if(app_title::atomic_file(status_path,&s,sizeof(s))){previous=s;have=true;}
            elapsed=0;
        }
        if(!diagnostic_written&&(s.stage>=100||s.reloads||s.reload_errors)) {
            write_diagnostics(s,diagnostic_path);diagnostic_written=true;
        }
        if(!style_diagnostic_written&&s.style_state>=static_cast<unsigned>(app_title::StyleState::applied)) {
            write_diagnostics(s,diagnostic_path);style_diagnostic_written=true;
        }
    }
};
}
extern "C" __attribute__((noreturn,visibility("default"))) void* elf_main(void*) {
    RuntimePublisher publisher{};publisher.update(snapshot());
    app_title::Request request{};const bool ready=prepare_request(request);
    if(ready&&!app_title::atomic_file(app_title::kRequest,&request,sizeof(request)))stage.store(107);
    publisher.update(snapshot());
    // The native worker remains resident; its temporary Mono attachment is gone.
    for(;;) {
        int pid=0,on=0,top=0;unsigned color=0xffffff,hooks=0;char extra=0;bool valid=false;
        if(ready)if(FILE* f=fopen(app_title::kActive,"r")) {
            valid=fscanf(f,"%d %d %d %x %u %c",&pid,&on,&top,&color,&hooks,&extra)==5&&pid==getpid()&&(on==0||on==1)&&top>=0&&top<=3&&color<=0xffffff&&(hooks==app_title::kRequiredHooks||hooks==app_title::kHooks);fclose(f);
        }
        app_title::Config config{};config.enabled=on==1;config.top=(top&1)!=0;config.outside=(top&2)!=0;config.color=color;
        activation(valid,config,hooks);publisher.update(snapshot());usleep(500000);
    }
}
