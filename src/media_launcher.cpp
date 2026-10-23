/* App Title for PS5 - Copyright (C) 2026 porhe911
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Media/deeplink registration reference: drakmor/ShadowMountPlus,
 * commit 98c524890176c7b474fcb03387bec88c4a559861 (GPLv3).
 * App Title implementation, revised 2026-10-18. */
#include "media_launcher.hpp"
#include "app_title.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <ps5/kernel.h>
#include <sys/stat.h>
#include <unistd.h>

extern "C" {
int sceKernelLoadStartModule(const char*,std::size_t,const void*,unsigned,void*,int*);
extern const unsigned char apptitle_settings_param[];
extern const std::size_t apptitle_settings_param_size;
extern const unsigned char apptitle_settings_icon[];
extern const std::size_t apptitle_settings_icon_size;
}

namespace app_title::ps5 {
namespace {
constexpr char kTitleId[] = "APID00001";
#ifdef APP_TITLE_HOST_TEST
constexpr char kDir[] = "./build-host/media/APID00001";
constexpr char kSysDir[] = "./build-host/media/APID00001/sce_sys";
constexpr char kParam[] = "./build-host/media/APID00001/sce_sys/param.json";
constexpr char kIcon[] = "./build-host/media/APID00001/sce_sys/icon0.png";
constexpr char kInstallRoot[] = "./build-host/media/";
#else
constexpr char kDir[] = "/user/app/APID00001";
constexpr char kSysDir[] = "/user/app/APID00001/sce_sys";
constexpr char kParam[] = "/user/app/APID00001/sce_sys/param.json";
constexpr char kIcon[] = "/user/app/APID00001/sce_sys/icon0.png";
constexpr char kInstallRoot[] = "/user/app/";
#endif
constexpr std::uint64_t kAppInstallerAuth = 0x4800000000000006ULL;

bool directory(const char* path) {
    struct stat st{};
    if (lstat(path, &st) == 0) return S_ISDIR(st.st_mode);
    return errno == ENOENT && mkdir(path, 0755) == 0;
}

bool asset(const char* path, const unsigned char* data, std::size_t size) {
    char temp[160]{};
    std::snprintf(temp, sizeof(temp), "%s.tmp", path);
    const int fd = open(temp, O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, 0644);
    if (fd < 0) return false;
    std::size_t n = 0;
    while (n < size) {
        const auto written = write(fd, data+n, size-n);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) break;
        n += static_cast<std::size_t>(written);
    }
    bool ok = n == size && fsync(fd) == 0;
    if (close(fd) != 0) ok = false;
    if (ok) ok = rename(temp, path) == 0;
    if (!ok) unlink(temp);
    return ok;
}

bool matches_file(const char* path,const unsigned char* expected,std::size_t size) {
    struct stat st{};
    if(lstat(path,&st)!=0 || !S_ISREG(st.st_mode) || st.st_size!=static_cast<off_t>(size))return false;
    const int fd=open(path,O_RDONLY|O_NOFOLLOW);if(fd<0)return false;
    unsigned char buffer[1024];std::size_t at=0;bool ok=true;
    while(at<size) {
        const auto count=size-at<sizeof(buffer)?size-at:sizeof(buffer);
        const auto n=read(fd,buffer,count);
        if(n<0&&errno==EINTR)continue;
        if(n<=0 || std::memcmp(buffer,expected+at,static_cast<std::size_t>(n))){ok=false;break;}
        at+=static_cast<std::size_t>(n);
    }
    if(close(fd)!=0)ok=false;
    return ok&&at==size;
}

bool only_entries(const char* path,const char* const* allowed,std::size_t count) {
    DIR* d=opendir(path);if(!d)return false;
    bool ok=true;
    while(auto* e=readdir(d)) {
        if(!std::strcmp(e->d_name,".")||!std::strcmp(e->d_name,".."))continue;
        bool known=false;
        for(std::size_t i=0;i<count;++i)if(!std::strcmp(e->d_name,allowed[i]))known=true;
        if(!known){ok=false;break;}
    }
    closedir(d);return ok;
}

bool can_stage_owned_title() {
    struct stat st{};
    if (lstat(kParam, &st) != 0) {
        // A missing parameter file does not authorize overwriting another app.
        if(errno!=ENOENT)return false;
        if(lstat(kDir,&st)!=0)return errno==ENOENT;
        if(!S_ISDIR(st.st_mode))return false;
        const char* root_entries[]={"sce_sys"};
        if(!only_entries(kDir,root_entries,1))return false;
        if(lstat(kSysDir,&st)!=0)return errno==ENOENT;
        if(!S_ISDIR(st.st_mode))return false;
        const char* system_entries[]={"icon0.png","icon0.png.tmp","param.json.tmp"};
        if(!only_entries(kSysDir,system_entries,3))return false;
        if(lstat(kIcon,&st)!=0)return errno==ENOENT;
        return matches_file(kIcon,apptitle_settings_icon,apptitle_settings_icon_size);
    }
    if (!S_ISREG(st.st_mode) || st.st_size != static_cast<off_t>(apptitle_settings_param_size))
        return false;
    return matches_file(kParam,apptitle_settings_param,apptitle_settings_param_size);
}

template<class Fn> Fn symbol(std::uint32_t handle,const char* name) {
    log_fmt("media.resolve.begin name=%s handle=0x%x",name,handle);
    const auto at=static_cast<std::uint64_t>(kernel_dynlib_dlsym(getpid(),handle,name));
    const bool valid=at>=0x10000 && at<0x800000000000ULL;
    log_fmt("media.resolve.end name=%s found=%d",name,valid?1:0);
    return valid?reinterpret_cast<Fn>(static_cast<std::uintptr_t>(at)):nullptr;
}

struct ServiceSession {
    const char* name;
    int (*terminate)()=nullptr;
    bool owned=false;
    bool close() {
        if(!owned)return true;
        owned=false;
        const int rc=terminate();
        log_fmt("media.%s_terminate.end rc=0x%x",name,rc);
        return rc==0;
    }
    ~ServiceSession(){close();}
};

bool module(const char* name,const char* path,unsigned id,std::uint32_t& handle) {
    handle=0;
    log_fmt("media.module.lookup.begin name=%s",name);
    const int lookup=kernel_dynlib_handle(getpid(),name,&handle);
    log_fmt("media.module.lookup.end name=%s rc=%d handle=0x%x",name,lookup,handle);
    if(lookup==0&&handle!=0)return true;
    // Use the native system loader. Do not create a second SDK dlopen object
    // and copy the SPRX's complete symbol/string tables at this stage.
    std::uint32_t sysmodule=0;
    const int sys_lookup=kernel_dynlib_handle(getpid(),"libSceSysmodule.sprx",&sysmodule);
    log_fmt("media.sysmodule.lookup rc=%d handle=0x%x",sys_lookup,sysmodule);
    using Load=int(*)(unsigned);
    auto load=sys_lookup==0&&sysmodule?symbol<Load>(sysmodule,"sceSysmoduleLoadModuleInternal"):nullptr;
    if(load) {
        log_fmt("media.module.internal_load.begin name=%s id=0x%x",name,id);
        const int rc=load(id);
        log_fmt("media.module.internal_load.end name=%s rc=0x%x",name,rc);
        handle=0;
        if(kernel_dynlib_handle(getpid(),name,&handle)==0&&handle)return true;
    }
    int start_result=0;
    log_fmt("media.module.native_load.begin name=%s",name);
    const int native=sceKernelLoadStartModule(path,0,nullptr,0,nullptr,&start_result);
    log_fmt("media.module.native_load.end name=%s handle=0x%x result=0x%x",name,native,start_result);
    handle=0;
    if(kernel_dynlib_handle(getpid(),name,&handle)==0&&handle)return true;
    return false;
}
} // namespace

bool ensure_media_launcher(std::uint32_t sdk_version) {
    // The upstream direct installer is unavailable on 12+. No ShellCore bridge.
    if ((sdk_version >> 24) >= 0x12) {
        log_line( "Media launcher skipped: direct installer unsupported on FW12+");
        return true;
    }
    log_line("media.assets.begin");
    if(!can_stage_owned_title()) {
        log_line("media.assets.refused: title directory is incomplete or not owned by App Title");
        return true;
    }
    const bool staged=directory(kDir)&&directory(kSysDir)&&
        asset(kIcon,apptitle_settings_icon,apptitle_settings_icon_size)&&
        asset(kParam,apptitle_settings_param,apptitle_settings_param_size);
    log_fmt("media.assets.end staged=%d errno=%d",staged?1:0,staged?0:errno);
    if(!staged)return true;

    ServiceSession user_session{"user"};
    std::uint32_t user=0;
    if(module("libSceUserService.sprx","/system/common/lib/libSceUserService.sprx",0x80000011U,user)) {
        using UserInit=int(*)(const void*);
        const auto user_init=symbol<UserInit>(user,"sceUserServiceInitialize");
        user_session.terminate=symbol<int(*)()>(user,"sceUserServiceTerminate");
        if(user_init&&user_session.terminate) {
            log_line("media.user_initialize.begin");
            const int user_rc=user_init(nullptr);
            user_session.owned=user_rc==0;
            log_fmt("media.user_initialize.end rc=0x%x",user_rc);
        }
    }
    std::uint32_t installer=0;
    if(!module("libSceAppInstUtil.sprx","/system/common/lib/libSceAppInstUtil.sprx",0x80000014U,installer)) {
        log_line("media.installer.unavailable; continuing without Media registration");
        return user_session.close();
    }
    using Init = int (*)();
    using Exists = int (*)(const char*, int*);
    using Install = int (*)(const char*, const char*, void*);
    const auto init=symbol<Init>(installer,"sceAppInstUtilInitialize");
    const auto terminate=symbol<Init>(installer,"sceAppInstUtilTerminate");
    const auto exists=symbol<Exists>(installer,"sceAppInstUtilAppExists");
    const auto install=symbol<Install>(installer,"sceAppInstUtilAppInstallTitleDir");
    if (!init || !terminate || !exists || !install) {
        log_line( "Media launcher unavailable: AppInstUtil symbols missing");
        return user_session.close();
    }
    // UserService precedes AppInstUtil as in SM+. AppInstUtil initialization
    // keeps the installer identity window used by the Common FPS baseline.
    // The matching Terminate must complete inside that same identity window;
    // no installer session is needed by the resident settings/ID workers.
    ServiceSession installer_session{"installer",terminate};
    int init_rc=-1;
    log_line("media.credentials.snapshot.begin");
    const auto original = kernel_get_ucred_authid(getpid());
    log_fmt("media.credentials.snapshot.end valid=%d",original!=0&&original!=static_cast<std::uint64_t>(-1)?1:0);
    if (original == 0 || original == static_cast<std::uint64_t>(-1)) {
        log_line( "Media launcher skipped: cannot snapshot credentials");
        return user_session.close();
    }
    int check_rc = -1, install_rc = -1, present = 0;
    log_line("media.credentials.enter.begin");
    if (kernel_set_ucred_authid(getpid(), kAppInstallerAuth) == 0 &&
        kernel_get_ucred_authid(getpid()) == kAppInstallerAuth) {
        log_line("media.credentials.enter.end verified=1");
        log_line("media.initialize.begin");
        init_rc=init();
        installer_session.owned=init_rc==0;
        log_fmt("media.initialize.end rc=0x%x",init_rc);
        if (init_rc == 0) {
            log_line("media.exists.begin");
            check_rc = exists(kTitleId, &present);
            log_fmt("media.exists.end rc=0x%x present=%d",check_rc,present);
            if (check_rc == 0 && !present) {
                log_line("media.install.begin");
                install_rc = install(kTitleId,kInstallRoot,nullptr);
                log_fmt("media.install.end rc=0x%x",install_rc);
                if (install_rc == 0 || static_cast<std::uint32_t>(install_rc) == 0x80990002U) {
                    log_line("media.verify.begin");
                    check_rc = exists(kTitleId, &present);
                    log_fmt("media.verify.end rc=0x%x present=%d",check_rc,present);
                }
            }
        }
    } else {
        log_line("media.credentials.enter.end verified=0");
    }
    // Close only sessions this call successfully initialized. In particular,
    // never terminate a service whose Initialize reported already initialized.
    const bool installer_released=installer_session.close();
    // Credential changes never overlap sampler work or the HTTP thread.
    log_line("media.credentials.restore.begin");
    const bool restored = kernel_set_ucred_authid(getpid(), original) == 0 &&
        kernel_get_ucred_authid(getpid()) == original;
    const bool user_released=user_session.close();
    char record[256]{};
    std::snprintf(record, sizeof(record),
        "Media launcher title=%s init=0x%x check=0x%x staged=%d install=0x%x present=%d auth_restored=%d",
        kTitleId, init_rc, check_rc, staged ? 1 : 0, install_rc, present, restored ? 1 : 0);
    log_line(record);
    log_fmt("media.result present=%d auth_restored=%d sessions_closed=%d",present,restored?1:0,installer_released&&user_released?1:0);
    return restored&&installer_released&&user_released;
}
} // namespace app_title::ps5
