// SPDX-License-Identifier: GPL-3.0-or-later
#include "app_title.hpp"
#include "live_ram_export.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <filesystem>
int main() {
    using namespace app_title;
    assert(!std::strcmp(style_export_state(),"idle"));
    assert(request_style_export()&&!request_style_export());
    assert(!std::strcmp(style_export_state(),"queued"));
    run_style_export();assert(!std::strcmp(style_export_state(),"failed"));
    run_style_export();assert(!std::strcmp(style_export_state(),"failed"));
    assert(request_style_export());run_style_export();
    std::filesystem::create_directories("build-host/export");
    const char* request="build-host/export/request.bin";const char* result="build-host/export/result.bin";
    LiveExportClient client{};assert(!client.begin(0,request,result)&&client.begin(getpid(),request,result));
    LiveExportMessage reply{};assert(read_export_message(request,reply)&&reply.pid==getpid());
    assert(!client.begin(getpid(),request,result));reply.serial+=1;reply.result=1;reply.bytes=100;
    assert(atomic_file(result,&reply,sizeof(reply))&&client.poll(request,result)==0);
    reply.serial-=1;assert(atomic_file(result,&reply,sizeof(reply))&&client.poll(request,result)==1);
    assert(client.begin(getpid(),request,result));for(unsigned i=0;i<15;++i)assert(client.poll(request,result)==0);
    assert(client.poll(request,result)==-1&&!client.pending);
    std::filesystem::remove_all("build-host/export");
    puts("PASS: manual export queue, duplicate refusal, idle loop and retry after failed result");
}
