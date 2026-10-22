// SPDX-License-Identifier: GPL-3.0-or-later
#include "app_title.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
extern "C" {
extern const unsigned char apptitle_settings_html[];
extern const std::size_t apptitle_settings_html_size;
}
namespace app_title {
namespace {
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
Config current{},pending{};Stats status{};bool changed=false;
char token[33]{};
bool send_all(int fd,const void* data,std::size_t size) {
    std::size_t sent=0;
    while(sent<size) {
        const auto n=send(fd,static_cast<const char*>(data)+sent,size-sent,MSG_NOSIGNAL);
        if(n<0&&errno==EINTR)continue;if(n<=0)return false;sent+=static_cast<std::size_t>(n);
    }
    return true;
}
void response(int fd,int code,const char* type,const void* data,std::size_t size) {
    char header[512];const int n=std::snprintf(header,sizeof(header),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nContent-Security-Policy: default-src 'self'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; img-src 'self' data:; frame-ancestors 'none'\r\n\r\n",
        code,code==200?"OK":"Error",type,size);
    if(n>0&&static_cast<std::size_t>(n)<sizeof(header)&&send_all(fd,header,static_cast<std::size_t>(n)))send_all(fd,data,size);
}
void error(int fd,int code,const char* msg){response(fd,code,"text/plain; charset=utf-8",msg,std::strlen(msg));}
bool equals_case(const char* a,const char* b) {
    while(*a&&*b){char c=*a++,d=*b++;if(c>='A'&&c<='Z')c+=32;if(d>='A'&&d<='Z')d+=32;if(c!=d)return false;}return *a==*b;
}
void serve(int fd) {
    char input[4097]{};std::size_t used=0;char* split=nullptr;
    while(used<4096) {
        const auto n=recv(fd,input+used,4096-used,0);if(n<=0)return;
        used+=static_cast<std::size_t>(n);input[used]=0;split=std::strstr(input,"\r\n\r\n");if(split)break;
    }
    if(!split){error(fd,413,"Request too large");return;}
    const auto head_size=static_cast<std::size_t>(split-input)+4;
    char headers[4097];std::memcpy(headers,input,head_size);headers[head_size]=0;
    char* end=std::strstr(headers,"\r\n");if(!end){error(fd,400,"Bad request");return;}*end=0;
    bool get_home=!std::strcmp(headers,"GET / HTTP/1.1");
    bool get_state=!std::strcmp(headers,"GET /api/state HTTP/1.1");
    const bool post_export=!std::strcmp(headers,"POST /api/export HTTP/1.1");
    bool post=post_export||!std::strcmp(headers,"POST /api/settings HTTP/1.1");
    bool host=false,host_seen=false,auth=false,origin_ok=true,length_seen=false,chunked=false;
    std::size_t length=0;
    for(char* line=end+2;*line && std::strcmp(line,"\r\n");) {
        char* next=std::strstr(line,"\r\n");if(!next)break;*next=0;
        if(!*line)break;
        char* colon=std::strchr(line,':');if(!colon){error(fd,400,"Bad header");return;}*colon++=0;while(*colon==' '||*colon=='\t')++colon;
        if(equals_case(line,"Host")) {if(host_seen){error(fd,400,"Duplicate host");return;}host_seen=true;host=!std::strcmp(colon,"127.0.0.1:39031");}
        if(equals_case(line,"Origin"))origin_ok=origin_ok&&!std::strcmp(colon,"http://127.0.0.1:39031");
        if(equals_case(line,"X-App-Title-Token"))auth=!std::strcmp(colon,token);
        if(equals_case(line,"Transfer-Encoding"))chunked=true;
        if(equals_case(line,"Content-Length")) {
            if(length_seen||!*colon){error(fd,400,"Bad content length");return;}length_seen=true;
            for(const char* c=colon;*c;++c)if(*c<'0'||*c>'9'){error(fd,400,"Bad content length");return;}
            char* tail=nullptr;const auto value=std::strtoul(colon,&tail,10);
            if(!tail||*tail||value>128){error(fd,413,"Body too large");return;}length=value;
        }
        line=next+2;
    }
    if(!host||!origin_ok||chunked){error(fd,403,"Local page only");return;}
    if(get_home){response(fd,200,"text/html; charset=utf-8",apptitle_settings_html,apptitle_settings_html_size);return;}
    if(get_state) {
        pthread_mutex_lock(&mutex);const auto c=current;const auto s=status;pthread_mutex_unlock(&mutex);
        char data[1536];const int n=std::snprintf(data,sizeof(data),"{\"version\":\"%s\",\"backend\":\"native-titleid\",\"style_supported\":%s,\"style_state\":%u,\"style_patches\":%llu,\"applied_placement\":\"%s\",\"applied_position\":\"%s\",\"applied_color\":\"%06x\",\"style_failure\":%u,\"failed_module\":%u,\"module_size\":%u,\"export_state\":\"%s\",\"export_path\":\"%s\",\"enabled\":%d,\"placement\":\"%s\",\"position\":\"%s\",\"color\":\"%06x\",\"hooks\":%u,\"pid\":%d,\"queries\":%llu,\"overrides\":%llu,\"reloads\":%llu,\"reload_errors\":%llu,\"phase\":\"%s\",\"token\":\"%s\"}",kVersion,s.style_state==static_cast<unsigned>(StyleState::applied)?"true":"false",s.style_state,static_cast<unsigned long long>(s.style_patches),s.style_outside?"outside":"inside",s.style_top?"top":"bottom",s.style_color,s.style_input.failure,s.style_input.failed_module,s.style_input.module_size,style_export_state(),style_export_path(),c.enabled?1:0,c.outside?"outside":"inside",c.top?"top":"bottom",c.color,s.hooks,s.shell_pid,static_cast<unsigned long long>(s.queries),static_cast<unsigned long long>(s.overrides),static_cast<unsigned long long>(s.reloads),static_cast<unsigned long long>(s.reload_errors),s.phase,token);
        if(n>0&&static_cast<std::size_t>(n)<sizeof(data))response(fd,200,"application/json",data,static_cast<std::size_t>(n));return;
    }
    if(!post){error(fd,404,"Not found");return;}
    if(!auth||!length_seen||!length){error(fd,403,"Invalid session");return;}
    while(used<head_size+length) {
        const auto n=recv(fd,input+used,head_size+length-used,0);if(n<=0)return;used+=static_cast<std::size_t>(n);
    }
    input[head_size+length]=0;
    if(std::memchr(input+head_size,0,length)){error(fd,400,"Invalid body");return;}
    if(post_export) {
        if(std::strcmp(input+head_size,"export=1")){error(fd,400,"Invalid export request");return;}
        if(!request_style_export()){error(fd,409,"Export already queued or running");return;}
        response(fd,200,"application/json","{\"queued\":true}",15);return;
    }
    Config c{};
    if(!parse_settings(input+head_size,c)){error(fd,400,"Invalid settings");return;}
    if(!save_config(c)){error(fd,500,"Cannot save settings");return;}
    pthread_mutex_lock(&mutex);pending=current=c;changed=true;pthread_mutex_unlock(&mutex);
    response(fd,200,"application/json","{\"saved\":true}",14);
}
void* listener(void* value) {
    const int server=static_cast<int>(reinterpret_cast<std::intptr_t>(value));
    log_line("Settings HTTP worker entered");
    for(;;) {
        const int client=accept(server,nullptr,nullptr);if(client<0){usleep(100000);continue;}
        timeval timeout{2,0};setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));setsockopt(client,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
#ifdef SO_NOSIGPIPE
        const int no_sigpipe=1;setsockopt(client,SOL_SOCKET,SO_NOSIGPIPE,&no_sigpipe,sizeof(no_sigpipe));
#endif
        serve(client);close(client);
    }
}
bool probe_listener() {
    const int fd=socket(AF_INET,SOCK_STREAM,0);if(fd<0)return false;
    timeval timeout{1,0};
    setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    sockaddr_in addr{};
#if defined(__FreeBSD__) || defined(__ORBIS__) || defined(__PROSPERO__)
    addr.sin_len=sizeof(addr);
#endif
    addr.sin_family=AF_INET;addr.sin_port=htons(kPort);addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    bool ok=connect(fd,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))==0;
    constexpr char request[]="GET /api/state HTTP/1.1\r\nHost: 127.0.0.1:39031\r\nConnection: close\r\n\r\n";
    if(ok)ok=send_all(fd,request,sizeof(request)-1);
    constexpr char expected[]="HTTP/1.1 200 ";char prefix[sizeof(expected)-1]{};
    std::size_t used=0;
    while(ok&&used<sizeof(prefix)) {
        const auto n=recv(fd,prefix+used,sizeof(prefix)-used,0);
        if(n<0&&errno==EINTR)continue;
        if(n<=0){ok=false;break;}
        used+=static_cast<std::size_t>(n);
    }
    close(fd);return ok&&used==sizeof(prefix)&&!std::memcmp(prefix,expected,sizeof(prefix));
}
}
bool start_settings(Config& c,Stats& s) {
    log_line("settings.entropy.begin");
    unsigned char entropy[16]{};bool ok=false;
    const char* devices[]={"/dev/urandom","/dev/random"};
    for(const char* path:devices) {
        const int random=open(path,O_RDONLY|O_NONBLOCK);
        if(random<0)continue;
        std::size_t used=0;
        while(used<sizeof(entropy)) {
            const auto n=read(random,entropy+used,sizeof(entropy)-used);
            if(n<0&&errno==EINTR)continue;
            if(n<=0)break;
            used+=static_cast<std::size_t>(n);
        }
        close(random);
        if(used==sizeof(entropy)){ok=true;break;}
    }
    if(!ok){log_line("settings.entropy.end ready=0; settings unavailable");return false;}
    log_line("settings.entropy.end ready=1");
    for(unsigned i=0;i<sizeof(entropy);++i)std::snprintf(token+i*2,3,"%02x",entropy[i]);
    log_line("settings.mutex.begin");
    pthread_mutex_lock(&mutex);current=c;status=s;pthread_mutex_unlock(&mutex);
    log_line("settings.mutex.end");
    const int server=socket(AF_INET,SOCK_STREAM,0);
    if(server<0){log_fmt("Settings socket failed errno=%d",errno);return false;}
    int yes=1;setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    sockaddr_in addr{};
#if defined(__FreeBSD__) || defined(__ORBIS__) || defined(__PROSPERO__)
    addr.sin_len=sizeof(addr);
#endif
    addr.sin_family=AF_INET;addr.sin_port=htons(kPort);addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    const int bind_rc=bind(server,reinterpret_cast<sockaddr*>(&addr),sizeof(addr));
    if(bind_rc!=0){const int error=errno;close(server);log_fmt("Settings bind failed errno=%d",error);return false;}
    if(listen(server,4)!=0){const int error=errno;close(server);log_fmt("Settings listen failed errno=%d",error);return false;}
    pthread_t thread{};
    const int rc=pthread_create(&thread,nullptr,listener,reinterpret_cast<void*>(static_cast<std::intptr_t>(server)));
    if(rc!=0){close(server);log_fmt("Settings pthread_create failed rc=%d",rc);return false;}
    pthread_detach(thread);
    log_line("Settings listening on loopback port 39031");
    const bool ready=probe_listener();
    log_fmt("Settings loopback HTTP self-check ready=%d",ready?1:0);
    return ready;
}
bool take_settings_change(Config& c) {
    pthread_mutex_lock(&mutex);const bool yes=changed;if(yes){c=pending;changed=false;}pthread_mutex_unlock(&mutex);return yes;
}
void publish_status(const Stats& s){pthread_mutex_lock(&mutex);status=s;pthread_mutex_unlock(&mutex);}
}
