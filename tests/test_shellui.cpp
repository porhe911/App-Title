// SPDX-License-Identifier: GPL-3.0-or-later
// Execute the actual native backend with mocked Mono and registry APIs.
#include "../src/shellui.cpp"
#include "ram_fixture.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
struct MonoDomain{};struct MonoThread{};struct MonoAssembly{int kind;};struct MonoImage{int kind;};
struct MonoClass{int kind;};struct MonoMethod{int kind;};struct MonoObject{};struct MonoString{std::string text;};
struct MonoMethodSignature{int kind;};struct MonoType{int kind;};
namespace {
MonoDomain domain;MonoThread thread;MonoAssembly react_assembly{1},pui_assembly{2};MonoImage react_image{1},pui_image{2};
MonoClass manager_class{1},application_class{2};MonoMethod reload_info{1},update_info{2};MonoObject exception_value;
MonoString home_name;bool pinned=false,on_ui_thread=false,raise_exception=false,reenter=false;
int fail_stage=0;unsigned original_queries=0,original_updates=0,invocations=0;
bool worker_attached=false;
unsigned attaches=0,detaches=0;
bool bad_reload_signature=false,bad_update_signature=false,byref_signature=false;
MonoMethodSignature reload_signature{1},update_signature{2};MonoType void_type{1},string_type{0x0e},int_type{0x08};
std::vector<std::string> events;
int original_registry(long,int* out){++original_queries;if(out)*out=42;return -77;}
void original_update(MonoObject*){assert(on_ui_thread);++original_updates;events.emplace_back("original");errno=EALREADY;}
void compiled_update(MonoObject*){}
void frame(){assert(!worker_attached);on_ui_thread=true;run_update(nullptr,original_update);assert(errno==EALREADY);on_ui_thread=false;}
}
extern "C" {
int sceRegMgrGetInt(long key,int* out){return original_registry(key,out);}
MonoDomain* mono_get_root_domain(){return fail_stage==100?nullptr:&domain;}
MonoThread* mono_thread_attach(MonoDomain* p){assert(p==&domain&&!worker_attached);if(fail_stage==101)return nullptr;worker_attached=true;++attaches;return &thread;}
void mono_thread_detach(MonoThread* t){assert(t==&thread&&worker_attached&&!on_ui_thread);worker_attached=false;++detaches;}
MonoAssembly* mono_domain_assembly_open(MonoDomain* p,const char* path) {
    assert(p==&domain&&worker_attached);
    if(!std::strcmp(path,"/system_ex/common_ex/lib/ReactNative.Vsh.Common.dll"))return fail_stage==102?nullptr:&react_assembly;
    assert(!std::strcmp(path,"/system_ex/common_ex/lib/Sce.PlayStation.PUI.dll"));return fail_stage==104?nullptr:&pui_assembly;
}
MonoImage* mono_assembly_get_image(MonoAssembly* a){assert(worker_attached);return a->kind==1?&react_image:&pui_image;}
MonoClass* mono_class_from_name(MonoImage* img,const char* ns,const char* name) {
    assert(worker_attached);
    if(img==&react_image){assert(!std::strcmp(ns,"ReactNative.Vsh.Common")&&!std::strcmp(name,"ReactApplicationSceneManager"));return &manager_class;}
    assert(img==&pui_image&&!std::strcmp(ns,"Sce.PlayStation.PUI")&&!std::strcmp(name,"Application"));return &application_class;
}
MonoMethod* mono_class_get_method_from_name(MonoClass* c,const char* name,int count) {
    assert(worker_attached);
    if(c==&manager_class){assert(!std::strcmp(name,"ReloadApp")&&count==1);return fail_stage==103?nullptr:&reload_info;}
    assert(c==&application_class&&!std::strcmp(name,"Update")&&count==0);return fail_stage==105?nullptr:&update_info;
}
std::uint32_t mono_method_get_flags(MonoMethod* m,std::uint32_t* impl){assert(worker_attached);*impl=0;return m==&reload_info?0x10:0;}
MonoMethodSignature* mono_method_signature(MonoMethod* m){assert(worker_attached);return m==&reload_info?&reload_signature:&update_signature;}
unsigned mono_signature_get_param_count(MonoMethodSignature* s){return s->kind==1?1:0;}
MonoType* mono_signature_get_return_type(MonoMethodSignature* s){return s->kind==2&&bad_update_signature?&int_type:&void_type;}
MonoType* mono_signature_get_params(MonoMethodSignature* s,void**){assert(s==&reload_signature);return bad_reload_signature?&int_type:&string_type;}
int mono_type_get_type(MonoType* t){return t->kind;}
int mono_type_is_byref(MonoType* t){return t==&string_type&&byref_signature?1:0;}
void* mono_compile_method(MonoMethod* m){assert(worker_attached&&m==&update_info);return fail_stage==106?nullptr:reinterpret_cast<void*>(&compiled_update);}
MonoString* mono_string_new(MonoDomain* d,const char* text){assert(on_ui_thread&&d==&domain);home_name.text=text;return &home_name;}
std::uint32_t mono_gchandle_new(MonoObject* obj,int pin){assert(obj==reinterpret_cast<MonoObject*>(&home_name)&&pin==1&&!pinned);pinned=true;return 1;}
MonoObject* mono_gchandle_get_target(std::uint32_t id){assert(pinned&&id==1);return reinterpret_cast<MonoObject*>(&home_name);}
void mono_gchandle_free(std::uint32_t id){assert(pinned&&id==1);pinned=false;errno=ENOMEM;}
MonoObject* mono_runtime_invoke(MonoMethod* m,void* instance,void** args,MonoObject** error) {
    assert(on_ui_thread&&m==&reload_info&&!instance&&pinned&&args[0]==&home_name&&home_name.text=="NPXS40002");
    ++invocations;events.emplace_back("reload");
    int value=0;const int rc=registry_get(app_title::kTitleIdKey,&value,original_registry);
    if(enabled.load())assert(rc==0&&value==1);else assert(rc==-77&&value==42);
    if(reenter){reenter=false;run_update(nullptr,original_update);}
    *error=raise_exception?&exception_value:nullptr;return nullptr;
}
}
int main(int argc,char** argv) {
    using namespace app_title;namespace fs=std::filesystem;
    for(int value:{100,101,102,103,104,105,106}) {
        fail_stage=value;Request r{};assert(!prepare_request(r)&&stage.load()==static_cast<unsigned>(value)&&r.count==0);
        assert(!worker_attached&&attaches==detaches);
    }
    fail_stage=0;
    for(unsigned mode=0;mode<3;++mode) {
        bad_reload_signature=mode==0;bad_update_signature=mode==1;byref_signature=mode==2;
        Request bad{};assert(!prepare_request(bad)&&bad.count==0);
        assert(!worker_attached&&attaches==detaches);
    }
    bad_reload_signature=bad_update_signature=byref_signature=false;
    Request request{};assert(prepare_request(request)&&request.count==kHooks&&stage.load()==5);
    assert(!worker_attached&&attaches==detaches&&attaches==9);
    assert(request.hooks[0].target==reinterpret_cast<std::uintptr_t>(&sceRegMgrGetInt));
    assert(request.hooks[1].target==reinterpret_cast<std::uintptr_t>(&compiled_update));
    assert(request.hooks[2].target==reinterpret_cast<std::uintptr_t>(&ioctl));
    Config config{};
    enabled.store(true);int out=0;
    assert(registry_get(kTitleIdKey,&out,original_registry)==0&&out==1&&original_queries==0);
    assert(registry_get(kTitleIdKey,nullptr,original_registry)==0);
    assert(registry_get(kTitleIdKey+1,&out,original_registry)==-77&&out==42&&original_queries==1);
    enabled.store(false);assert(registry_get(kTitleIdKey,&out,original_registry)==-77&&out==42);
    activation(true,config,kHooks);assert(reload_pending.load()&&invocations==0);
    reenter=true;frame();assert(invocations==1&&reloads.load()==1&&!pinned&&!reload_pending.load());
    assert(events[0]=="original"&&events[1]=="reload");
    for(unsigned i=0;i<1000;++i){activation(true,config,kHooks);frame();}
    assert(invocations==1); // no repeated home reload on the polling timer
    config.enabled=false;activation(true,config,kHooks);frame();assert(invocations==2&&reloads.load()==2&&!enabled.load());
    config.enabled=true;activation(true,config,kHooks);raise_exception=true;frame();raise_exception=false;
    assert(reload_errors.load()==1&&!pinned);frame();assert(invocations==3);
    config.top=true;config.color=0x12abef;
    activation(true,config,kHooks);assert(reload_pending.load());frame();assert(invocations==4);
    for(unsigned i=0;i<1000;++i){activation(true,config,kHooks);frame();}assert(invocations==4);
    assert(style_config.load()==0x312abef&&style_state.load()==StyleState::waiting);
    config.outside=false;activation(true,config,kHooks);assert(reload_pending.load());frame();assert(invocations==5&&style_config.load()==0x112abef);
    for(unsigned i=0;i<1000;++i){activation(true,config,kHooks);frame();}assert(invocations==5);
    config.outside=true;activation(true,config,kHooks);frame();assert(invocations==6&&style_config.load()==0x312abef);
    unsigned ioctl_calls=0;auto passthrough=[](int,unsigned long,void*){errno=EBADF;return -19;};
    assert(ioctl_get(10,0xc0105203,reinterpret_cast<void*>(1),passthrough)==-19);
    assert(errno==EBADF);
    assert(ioctl_get(10,0xc0105202,reinterpret_cast<void*>(1),[](int,unsigned long,void*){return 0;})==0);
    assert(ioctl_get(10,0xc0105203,nullptr,[](int,unsigned long,void*){return 0;})==0);
    if(argc>1) {
        std::ifstream fixture(argv[1],std::ios::binary);assert(fixture.good());
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(fixture)),{});
        const auto before=bytes;RnpsArgs args{bytes.data(),static_cast<int>(bytes.size()),1};
        assert(ioctl_get(10,0xc0105203,&args,[](int,unsigned long,void*){return 0;})==0&&bytes==before);
        args.error=0;
        assert(ioctl_get(10,0xc0105203,&args,[](int fd,unsigned long command,void*){assert(fd==10&&command==0xc0105203);return 0;})==0);
        ++ioctl_calls;assert(bytes!=before&&style_patches.load()==1&&style_state.load()==StyleState::applied&&applied_config.load()==0x312abef);
        if(bytes.size()==1389744) {
            auto live=before;
            const std::uint32_t padded=1386512;std::memcpy(live.data()+0x24,&padded,4);
            RnpsArgs transport{live.data(),static_cast<int>(live.size()),0};
            assert(ioctl_get(10,0xc0105203,&transport,[](int,unsigned long,void*){errno=EAGAIN;return 0;})==0&&errno==EAGAIN);
            assert(style_patches.load()==2&&style_state.load()==StyleState::applied&&live!=before);
            const auto state=snapshot();
            assert(state.style_input.format==2&&state.style_input.buffer_size==live.size()&&state.style_input.payload_size==padded&&state.style_input.modules==858&&state.style_input.failure==0);
            live=app_title_test::ram_variant(before);transport={live.data(),static_cast<int>(live.size()),0};
            assert(ioctl_get(10,0xc0105203,&transport,[](int,unsigned long,void*){return 0;})==0);
            assert(style_patches.load()==3&&style_state.load()==StyleState::applied&&snapshot().style_input.payload_size==1385934);
            live=app_title_test::ram_variant(before,10,false,1);transport={live.data(),static_cast<int>(live.size()),0};
            assert(ioctl_get(10,0xc0105203,&transport,[](int,unsigned long,void*){return 0;})==0);
            assert(style_patches.load()==4&&style_state.load()==StyleState::applied&&snapshot().style_input.failure==0);
            live=app_title_test::ram_variant(before);transport.buffer=live.data();
            const auto at=app_title_test::ram_u32(live,2848+12+8*219)+6876+2848;live[at+100]^=1;
            assert(ioctl_get(10,0xc0105203,&transport,[](int,unsigned long,void*){return 0;})==0);
            assert(style_state.load()==StyleState::invalid&&snapshot().style_input.failed_module==219&&snapshot().style_input.module_size==567);
            const auto marker=std::uint32_t(859);live=before;std::memcpy(live.data()+2848+4,&marker,4);
            transport={live.data(),static_cast<int>(live.size()),0};
            assert(ioctl_get(10,0xc0105203,&transport,[](int,unsigned long,void*){return 0;})==0);
            assert(style_state.load()==StyleState::unsupported&&snapshot().style_input.modules==859&&snapshot().style_input.failure==2);
        }
        bytes=before;activation(true,config,kRequiredHooks);frame();
        assert(style_state.load()==StyleState::unavailable);
        assert(ioctl_get(10,0xc0105203,&args,[](int,unsigned long,void*){return 0;})==0&&bytes==before);
        activation(true,config,kHooks);frame();config.enabled=false;activation(true,config,kHooks);frame();
        assert(ioctl_get(10,0xc0105203,&args,[](int,unsigned long,void*){return 0;})==0&&bytes==before);
        assert(style_state.load()==StyleState::idle);
    }
    (void)ioctl_calls;
    fs::create_directories("build-host/native");RuntimePublisher publisher{};auto state=snapshot();
    publisher.update(state,"build-host/native/status.bin","build-host/native/diagnostic.log");
    const auto binary_stamp=fs::last_write_time("build-host/native/status.bin"),text_stamp=fs::last_write_time("build-host/native/diagnostic.log");
    for(unsigned i=0;i<10000;++i){++state.registry_calls;publisher.update(state,"build-host/native/status.bin","build-host/native/diagnostic.log");}
    assert(fs::last_write_time("build-host/native/status.bin")==binary_stamp&&fs::last_write_time("build-host/native/diagnostic.log")==text_stamp);
    ++state.overrides;publisher.update(state,"build-host/native/status.bin","build-host/native/diagnostic.log");
    std::ifstream file("build-host/native/status.bin",std::ios::binary);RuntimeStatus loaded{};file.read(reinterpret_cast<char*>(&loaded),sizeof(loaded));assert(loaded.overrides==state.overrides);
    assert(fs::last_write_time("build-host/native/diagnostic.log")==text_stamp);
    fs::remove("build-host/native/status.bin");fs::remove("build-host/native/diagnostic.log");fs::remove("build-host/native");
    puts("PASS: balanced Mono attachment on all bootstrap paths, native worker detached, UI-thread reloads and errno preserved, exact registry key, GC pinning, reentry/exception guards, colour/position changes, style routing and bounded logging");
}
