// SPDX-License-Identifier: GPL-3.0-or-later
#include "app_title.hpp"
#include "style_probe.h"
#include "live_ram_export.hpp"
#include <atomic>
#include <cerrno>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#ifndef APP_TITLE_HOST_TEST
extern "C" {
#include <ps5/kernel.h>
}
#endif
namespace app_title {
namespace {
// The HTTP thread only queues. Device I/O runs on the controller thread,
// sequentially with installer/ptrace credential changes and hook setup.
std::atomic<unsigned> state{0};
std::atomic_bool live_path{false};LiveExportClient live_client{};
struct Args {void* data;int size;std::uint32_t error;};
static_assert(sizeof(Args)==16&&offsetof(Args,size)==8&&offsetof(Args,error)==12);
#ifndef APP_TITLE_HOST_TEST
decrypt_result decrypt(void* data,int size) {
    decrypt_result r{-1,0,0x800f1225U};
    const int fd=open("/dev/rnps",O_RDWR);
    if(fd<0){r.saved_errno=errno;return r;}
    Args args{data,size,0x800f1225U};
    r.ioctl_rc=ioctl(fd,0xc0105203UL,&args);r.saved_errno=r.ioctl_rc<0?errno:0;
    r.device_error=args.error;close(fd);return r;
}
#endif
}
const char* style_export_state() {
    constexpr const char* names[]={"idle","queued","running","saved","failed"};
    return names[state.load(std::memory_order_acquire)];
}
const char* style_export_path(){return live_path.load(std::memory_order_acquire)?kLiveExportFile:"/data/AppTitle/HomeUI_decrypted.bin";}
bool request_style_export() {
    auto previous=state.load(std::memory_order_acquire);
    do {if(previous==1||previous==2)return false;}
    while(!state.compare_exchange_weak(previous,1,std::memory_order_acq_rel));
    return true;
}
void run_style_export() {
    if(live_client.pending) {
        const int result=live_client.poll();
        if(result)state.store(result>0?3:4,std::memory_order_release);
        return;
    }
    unsigned queued=1;if(!state.compare_exchange_strong(queued,2,std::memory_order_acq_rel))return;
#ifdef APP_TITLE_HOST_TEST
    state.store(4,std::memory_order_release); // No PS5 device access in HTTP tests.
#else
    int pid=0;if(FILE* f=fopen(kActive,"r")){(void)fscanf(f,"%d",&pid);fclose(f);}
    RuntimeStatus status{};
    if(load_runtime_status(pid,status)&&status.enabled&&status.style_input.format==2) {
        live_path.store(true,std::memory_order_release);
        if(!live_client.begin(pid))state.store(4,std::memory_order_release);
        return;
    }
    live_path.store(false,std::memory_order_release);
    const char* input="/system_ex/rnps/apps/NPXS40002/application.ps.bundle";
    if(access(input,F_OK)<0&&errno==ENOENT)input="/mnt/rnps/apps/NPXS40002/application.ps.bundle";
    const int rc=style_probe(input,"/data/AppTitle/HomeUI_decrypted.bin",
        "/data/AppTitle/AppTitle_Style_Probe.log",kernel_get_fw_version(),decrypt);
    state.store(rc==0?3:4,std::memory_order_release);
    if(rc!=0)log_error("Manual interface export failed; see /data/AppTitle/AppTitle_Style_Probe.log");
#endif
}
}
