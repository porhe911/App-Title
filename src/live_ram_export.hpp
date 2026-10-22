// SPDX-License-Identifier: GPL-3.0-or-later
// Explicit-request export of the actual pre-edit RAM source. No hook-thread IO.
#pragma once
#include "app_title.hpp"
#include <atomic>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
namespace app_title {
constexpr char kLiveExportRequest[]="/system_tmp/app_title_export_request.bin";
constexpr char kLiveExportResult[]="/system_tmp/app_title_export_result.bin";
constexpr char kLiveExportFile[]="/data/AppTitle/HomeUI_live.bin";
constexpr std::size_t kLiveExportLimit=4*1024*1024;
struct LiveExportMessage {
    std::uint64_t magic=0x3145505854545041ULL;
    std::int32_t pid=0;std::uint32_t serial=0,result=0,bytes=0;
};
static_assert(sizeof(LiveExportMessage)==24);
inline bool read_export_message(const char* path,LiveExportMessage& out) {
    FILE* f=std::fopen(path,"rb");if(!f)return false;
    LiveExportMessage message{};const bool ok=std::fread(&message,1,sizeof(message),f)==sizeof(message)&&std::fgetc(f)==EOF;
    std::fclose(f);if(!ok||message.magic!=LiveExportMessage{}.magic)return false;out=message;return true;
}
struct LiveExportClient {
    LiveExportMessage request{};unsigned ticks=0;bool pending=false;
    bool begin(int pid,const char* input=kLiveExportRequest,const char* output=kLiveExportResult) {
        if(pending||pid<=0)return false;
        request.pid=pid;if(!++request.serial)++request.serial;request.result=request.bytes=0;ticks=0;
        unlink(output);pending=atomic_file(input,&request,sizeof(request));return pending;
    }
    int poll(const char* input=kLiveExportRequest,const char* output=kLiveExportResult) {
        if(!pending)return -1;
        LiveExportMessage reply{};
        if(read_export_message(output,reply)&&reply.pid==request.pid&&reply.serial==request.serial&&reply.result) {
            pending=false;unlink(output);
            return reply.result==1&&reply.bytes>0&&reply.bytes<=kLiveExportLimit?1:-1;
        }
        if(++ticks>=16){pending=false;unlink(input);return -1;}return 0;
    }
};
class LiveRamExport {
    std::atomic_flag busy=ATOMIC_FLAG_INIT;
    std::atomic_bool armed{false};std::atomic<std::size_t> complete{0};
    unsigned char* buffer=nullptr;LiveExportMessage request{};unsigned ticks=0;bool pending=false;
    bool release() {
        armed.store(false,std::memory_order_release);
        if(busy.test_and_set(std::memory_order_acquire))return false;
        std::free(buffer);buffer=nullptr;complete.store(0);busy.clear(std::memory_order_release);return true;
    }
public:
    void capture(const void* source,std::size_t size) {
        if(!source||!size||size>kLiveExportLimit||!armed.load(std::memory_order_acquire)||busy.test_and_set(std::memory_order_acquire))return;
        if(buffer&&armed.load(std::memory_order_acquire)) {
            std::memcpy(buffer,source,size);complete.store(size,std::memory_order_release);armed.store(false,std::memory_order_release);
        }
        busy.clear(std::memory_order_release);
    }
    // Return true once when an explicit request needs a managed home refresh.
    bool poll(bool can_capture,const char* input=kLiveExportRequest,const char* output=kLiveExportResult,const char* destination=kLiveExportFile) {
        if(pending) {
            const auto size=complete.load(std::memory_order_acquire);
            if(size||++ticks>=24) {
                if(busy.test_and_set(std::memory_order_acquire))return false;
                armed.store(false,std::memory_order_release);
                const auto captured=complete.load(std::memory_order_acquire);
                const bool saved=captured&&atomic_file(destination,buffer,captured);
                std::free(buffer);buffer=nullptr;complete.store(0);busy.clear(std::memory_order_release);
                request.result=saved?1:2;request.bytes=saved?static_cast<std::uint32_t>(captured):0;
                (void)atomic_file(output,&request,sizeof(request));pending=false;
            }
            return false;
        }
        LiveExportMessage next{};
        if(!read_export_message(input,next)||next.pid!=getpid()||!next.serial||next.result||next.bytes)return false;
        unlink(input);request=next;
        if(!can_capture||!release()||!(buffer=static_cast<unsigned char*>(std::malloc(kLiveExportLimit)))) {
            request.result=2;(void)atomic_file(output,&request,sizeof(request));return false;
        }
        ticks=0;pending=true;armed.store(true,std::memory_order_release);return true;
    }
};
}
