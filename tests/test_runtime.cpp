// SPDX-License-Identifier: GPL-3.0-or-later
#include "app_title.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/stat.h>
namespace {
std::string contents(const char* p){std::ifstream f(p);return {(std::istreambuf_iterator<char>(f)),{}};}
}
int main() {
    using namespace app_title;namespace fs=std::filesystem;
    fs::create_directories("build-host/runtime");
    const char* data="build-host/runtime/data";
    assert(prepare_data_directory(data)&&prepare_data_directory(data));
    reset_log("build-host/runtime/data/AppTitle.log");log_line("new directory ready");
    assert(contents("build-host/runtime/data/AppTitle.log")=="new directory ready\n");
    fs::remove_all(data);fs::create_symlink(".",data);assert(!prepare_data_directory(data));fs::remove(data);
    // Reproduce the observed PID sequence. No later PID is allowed to start.
    SessionGuard guard{};assert(!guard.begin(-1)&&guard.begin(58));
    assert(!guard.begin(58)&&!guard.observe(-1)&&!guard.observe(58));
    assert(guard.observe(104)&&guard.blocked&&!guard.begin(104));
    assert(!guard.observe(106)&&guard.blocked&&!guard.begin(106));
    assert(!guard.observe(58)&&!guard.begin(58));
    const char* session="build-host/runtime/session";fs::remove(session);
    assert(reserve_session(58,session));const auto reserved=contents(session);
    assert(reserved.find(std::string(kVersion)+" 58")!=std::string::npos);
    assert(!reserve_session(104,session)&&contents(session)==reserved);
    struct stat st{};assert(stat(session,&st)==0&&(st.st_mode&0777)==0600);
    // Simulate the ephemeral reservation disappearing on a full reboot.
    fs::remove(session);SessionGuard fresh{};assert(fresh.begin(104)&&reserve_session(104,session));
    fs::remove(session);
    const char* log="build-host/runtime/startup.log";
    reset_log(log);log_line("startup begins");log_line("hooks ready");finish_startup_log();
    const auto before=contents(log);const auto stamp=fs::last_write_time(log);
    for(unsigned i=0;i<10000;++i)log_line("Worker alive");
    assert(contents(log)==before&&fs::last_write_time(log)==stamp);
    log_error("ShellUI changed old_pid=%d new_pid=%d; stopped",58,104);
    assert(contents(log).find("new_pid=104")!=std::string::npos);
    for(unsigned i=0;i<1000;++i)log_error("error %u: bounded log retains its start",i);
    assert(fs::file_size(log)<=kLogLimit&&contents(log).find("startup begins")==0);
    const auto capped=contents(log);log_line("no normal logging after activation");assert(contents(log)==capped);
    reset_log(log);log_line("startup evidence");finish_startup_log();
    for(unsigned i=0;i<1000;++i)log_home_result("HomeUI result requested=outside/bottom/ffffff result=%u",i);
    const auto latest=contents(log);
    assert(latest.find("startup evidence\n")==0&&latest.find("result=999\n")!=std::string::npos);
    assert(latest.find("HomeUI result ")==latest.rfind("HomeUI result ")&&latest.size()<256);
    reset_log(log);log_line("next activation");assert(contents(log)=="next activation\n");finish_startup_log();
    const char* launch="build-host/runtime/launch.log";
    assert(record_launch("main.enter",0,launch));
    assert(contents(launch).find(std::string("App Title v")+kVersion+" launch\n")==0);
    assert(record_launch("lock.acquire_failed",11,launch));
    assert(contents(launch).find("stage=lock.acquire_failed result=11")!=std::string::npos);
    assert(contents(launch).find("main.enter")==std::string::npos&&fs::file_size(launch)<256);
    assert(contents(log)=="next activation\n"); // A rejected launch never erases the worker journal.
    fs::remove(launch);fs::create_symlink("startup.log",launch);
    assert(!record_launch("must.not.follow.symlink",0,launch)&&contents(log)=="next activation\n");
    fs::remove(launch);
    fs::remove(log);fs::remove("build-host/runtime");
    puts("PASS: session/restart guard, zero periodic logs, hard size cap, reset and independent bounded launch snapshot (including lock failure/symlink refusal)");
}
