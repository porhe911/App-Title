// SPDX-License-Identifier: GPL-3.0-or-later
#include "app_title.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>

namespace app_title {
bool prepare_data_directory(const char* path) {
    struct stat st{};
    if(lstat(path,&st)==0){if(S_ISDIR(st.st_mode))return true;errno=ENOTDIR;return false;}
    return errno==ENOENT&&mkdir(path,0755)==0;
}
namespace {
pthread_mutex_t log_mutex=PTHREAD_MUTEX_INITIALIZER;
bool startup_log=true;const char* log_path=kLog;
void append_log(const char* text,bool error) {
    pthread_mutex_lock(&log_mutex);
    if(!startup_log&&!error){pthread_mutex_unlock(&log_mutex);return;}
    const int fd=open(log_path,O_WRONLY|O_CREAT|O_APPEND|O_NOFOLLOW,0644);
    if(fd>=0) {
        char line[600];const int n=std::snprintf(line,sizeof(line),"%s\n",text);
        struct stat st{};
        if(n>0&&static_cast<std::size_t>(n)<sizeof(line)&&fstat(fd,&st)==0&&st.st_size>=0&&
           static_cast<std::uint64_t>(st.st_size)+static_cast<unsigned>(n)<=kLogLimit) {
            std::size_t done=0;
            while(done<static_cast<std::size_t>(n)) {
                const auto count=write(fd,line+done,static_cast<std::size_t>(n)-done);
                if(count<0&&errno==EINTR)continue;
                if(count<=0)break;
                done+=static_cast<std::size_t>(count);
            }
        }
        close(fd);
    }
    pthread_mutex_unlock(&log_mutex);
}
}
static bool hex_color(const char* s,std::uint32_t& color) {
    if(std::strlen(s)!=6)return false;
    color=0;
    for(int i=0;i<6;++i) {
        const char c=s[i]; int n=-1;
        if(c>='0'&&c<='9')n=c-'0';
        if(c>='a'&&c<='f')n=c-'a'+10;
        if(c>='A'&&c<='F')n=c-'A'+10;
        if(n<0)return false;
        color=(color<<4)|static_cast<unsigned>(n);
    }
    return true;
}
bool parse_settings(const char* body,Config& result) {
    if(!body)return false;
    std::size_t size=0;
    while(body[size] && size<=100)++size;
    if(size>100)return false;
    char buf[128];std::memcpy(buf,body,size+1);
    Config c{}; unsigned found=0;
    char* cursor=buf;
    while(*cursor) {
        while(*cursor=='&'||*cursor=='\n'||*cursor=='\r')++cursor;
        if(!*cursor)break;
        char* token=cursor;
        while(*cursor && *cursor!='&' && *cursor!='\n' && *cursor!='\r')++cursor;
        if(*cursor)*cursor++=0;
        char* eq=token;
        while(*eq && *eq!='=')++eq;
        if(!*eq)return false;
        *eq++=0;
        unsigned bit=0;
        if(!std::strcmp(token,"enabled")) {
            bit=1;if(std::strcmp(eq,"0") && std::strcmp(eq,"1"))return false;c.enabled=eq[0]=='1';
        } else if(!std::strcmp(token,"position")) {
            bit=2;if(std::strcmp(eq,"top") && std::strcmp(eq,"bottom"))return false;c.top=!std::strcmp(eq,"top");
        } else if(!std::strcmp(token,"color")) {
            bit=4;if(!hex_color(eq,c.color))return false;
        } else if(!std::strcmp(token,"placement")) {
            bit=8;if(std::strcmp(eq,"inside")&&std::strcmp(eq,"outside"))return false;c.outside=!std::strcmp(eq,"outside");
        } else return false;
        if(found&bit)return false;
        found|=bit;
    }
    if(found!=7&&found!=15)return false;
    result=c;return true;
}
bool load_config(Config& c,const char* path) {
    const int fd=open(path,O_RDONLY|O_NOFOLLOW);
    if(fd<0)return false;
    char buf[128]{};std::size_t used=0;bool eof=false,ok=true;
    while(used<sizeof(buf)-1) {
        const auto n=read(fd,buf+used,sizeof(buf)-1-used);
        if(n<0&&errno==EINTR)continue;
        if(n<0){ok=false;break;}
        if(n==0){eof=true;break;}
        used+=static_cast<std::size_t>(n);
    }
    if(close(fd)!=0)ok=false;
    return ok && eof && used>0 && !std::memchr(buf,0,used) && parse_settings(buf,c);
}
bool load_runtime_status(int pid,RuntimeStatus& status,const char* path) {
    const int fd=open(path,O_RDONLY|O_NOFOLLOW);
    if(fd<0)return false;
    unsigned char bytes[sizeof(RuntimeStatus)+1]{};std::size_t used=0;
    while(used<sizeof(bytes)) {
        const auto n=read(fd,bytes+used,sizeof(bytes)-used);
        if(n<0&&errno==EINTR)continue;
        if(n<0){close(fd);return false;}
        if(n==0)break;
        used+=static_cast<std::size_t>(n);
    }
    close(fd);
    if(used!=sizeof(RuntimeStatus))return false;
    RuntimeStatus next{};std::memcpy(&next,bytes,sizeof(next));
    if(next.magic!=kStatusMagic||next.pid!=pid||pid<=0||next.enabled>1||next.style_state>5||next.style_color>0xffffff||next.style_top>3||next.style_input.format>2||next.style_input.failure>5||next.style_input.buffer_size>32*1024*1024||next.style_input.payload_size>32*1024*1024)return false;
    status=next;return true;
}
bool save_config(const Config& c,const char* path) {
    char buf[100];const int n=std::snprintf(buf,sizeof(buf),"enabled=%d\nposition=%s\ncolor=%06x\nplacement=%s\n",c.enabled?1:0,c.top?"top":"bottom",c.color,c.outside?"outside":"inside");
    return n>0 && atomic_file(path,buf,static_cast<std::size_t>(n));
}
void reset_log(const char* path) {
    pthread_mutex_lock(&log_mutex);log_path=path;startup_log=true;
    const int fd=open(path,O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW,0644);if(fd>=0)close(fd);
    pthread_mutex_unlock(&log_mutex);
}
void finish_startup_log(){pthread_mutex_lock(&log_mutex);startup_log=false;pthread_mutex_unlock(&log_mutex);}
void log_line(const char* text){append_log(text,false);}
void log_fmt(const char* fmt,...) {
    char text[512];va_list ap;va_start(ap,fmt);std::vsnprintf(text,sizeof(text),fmt,ap);va_end(ap);log_line(text);
}
void log_error(const char* fmt,...) {
    char text[512];va_list ap;va_start(ap,fmt);std::vsnprintf(text,sizeof(text),fmt,ap);va_end(ap);append_log(text,true);
}
void log_home_result(const char* fmt,...) {
    char line[640];va_list ap;va_start(ap,fmt);const int count=std::vsnprintf(line,sizeof(line),fmt,ap);va_end(ap);
    if(count<=0||static_cast<std::size_t>(count)>=sizeof(line)-1||std::strncmp(line,"HomeUI result ",14))return;
    line[count]='\n';const auto length=static_cast<std::size_t>(count)+1;
    pthread_mutex_lock(&log_mutex);
    char previous[kLogLimit+1]{},next[kLogLimit];std::size_t used=0;
    const int fd=open(log_path,O_RDONLY|O_NOFOLLOW);
    if(fd>=0) {
        while(used<kLogLimit){const auto n=read(fd,previous+used,kLogLimit-used);if(n<0&&errno==EINTR)continue;if(n<=0)break;used+=static_cast<std::size_t>(n);}
        close(fd);
    }
    std::size_t written=0;
    for(std::size_t p=0;p<used;) {
        std::size_t end=p;while(end<used&&previous[end]!='\n')++end;if(end<used)++end;
        if((end-p<14||std::memcmp(previous+p,"HomeUI result ",14))&&written+end-p+length<=kLogLimit) {
            std::memcpy(next+written,previous+p,end-p);written+=end-p;
        }
        p=end;
    }
    std::memcpy(next+written,line,length);written+=length;
    (void)atomic_file(log_path,next,written);
    pthread_mutex_unlock(&log_mutex);
}
bool record_launch(const char* stage,int result,const char* path) {
    char text[256];
    const int n=std::snprintf(text,sizeof(text),"App Title v%s launch\npid=%d stage=%s result=%d\n",kVersion,static_cast<int>(getpid()),stage,result);
    if(n<=0||static_cast<std::size_t>(n)>=sizeof(text))return false;
    const int fd=open(path,O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW,0644);
    if(fd<0)return false;
    std::size_t used=0;
    while(used<static_cast<std::size_t>(n)) {
        const auto count=write(fd,text+used,static_cast<std::size_t>(n)-used);
        if(count<0&&errno==EINTR)continue;
        if(count<=0)break;
        used+=static_cast<std::size_t>(count);
    }
    const bool closed=close(fd)==0;
    return used==static_cast<std::size_t>(n)&&closed;
}
bool reserve_session(int pid,const char* path) {
    if(pid<=0)return false;
    const int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);
    if(fd<0)return false;
    char text[64];const int n=std::snprintf(text,sizeof(text),"%s %d\n",kVersion,pid);
    const bool ok=n>0&&write(fd,text,static_cast<std::size_t>(n))==n;
    const bool closed=close(fd)==0;
    // Keep a failed reservation too: an uncertain launch must not be repeated.
    return ok&&closed;
}
}
