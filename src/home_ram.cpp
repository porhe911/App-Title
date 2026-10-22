// SPDX-License-Identifier: GPL-3.0-or-later
// 4.03 HomeUI Indexed RAM profile, with startup and scoped source guards.
// Module addresses are resolved from the live table. Only metadata and our
// generated styles/replacements are shipped; the console's source stays local.
#include "home_style.hpp"
#include <cstdio>
#include <cstring>

namespace app_title {
namespace {
constexpr unsigned module_count=858,startup_size=1795;
constexpr std::size_t table_base=12+8*module_count;
struct Module {unsigned id;std::size_t size;const char* hash;};
constexpr Module modules[]={
    {25,1205,"09a2036b256701606f04f3b524e2a66e379df0dc"},
    {102,1016,"d9dd1aa657d5110c50915619015184104f317501"},
    {219,567,"713c438f042772f45fdac128325454d60d59216b"},
    {527,5276,"a74aec5cb8490615612e42b272b539c25ed4d23f"},
    {531,1507,"a35b627618c3b5a8b0d23f4f5f0d1bc85a9c7599"},
    {552,1948,"2aef30b24d50393bdd47650dfdfbec1d8e93efa1"}
};
std::uint32_t read32(const std::uint8_t* p) {
    return std::uint32_t(p[0])|(std::uint32_t(p[1])<<8)|
        (std::uint32_t(p[2])<<16)|(std::uint32_t(p[3])<<24);
}
bool digest_is(const void* bytes,std::size_t size,const char* expected) {
    std::uint8_t digest[20];sha1_digest(bytes,size,digest);
    constexpr char hex[]="0123456789abcdef";
    for(unsigned i=0;i<20;++i)
        if(expected[i*2]!=hex[digest[i]>>4]||expected[i*2+1]!=hex[digest[i]&15])return false;
    return true;
}
// Only Tile's build filename, five debug line numbers and redundant outer
// function parentheses may vary. Every executable byte, including imports,
// visibility and render code,
// must match. This normalization is for comparison; it never edits live source.
bool tile_digest(const std::uint8_t* input,std::size_t size) {
    char canonical[4096];
    if(!size||size>sizeof(canonical))return false;
    std::memcpy(canonical,input,size);
    char* first=std::strstr(canonical,",p=\"");
    if(!first||std::strstr(first+1,",p=\""))return false;
    first+=4;char* end=std::strchr(first,'\"');
    constexpr char suffix[]="/packages/home-ui/src/components/Tile/index.tsx";
    if(!end||static_cast<std::size_t>(end-first)<sizeof(suffix)-1||
       std::memcmp(end-(sizeof(suffix)-1),suffix,sizeof(suffix)-1))return false;
    for(char* p=first;p<end;++p)
        if(static_cast<unsigned char>(*p)<32||*p=='\\')return false;
    std::memmove(first,end,std::strlen(end)+1);
    constexpr char line[]="__source:{fileName:p,lineNumber:";
    char* from=canonical;unsigned count=0;
    while(char* at=std::strstr(from,line)) {
        at+=sizeof(line)-1;char* number_end=at;
        while(*number_end>='0'&&*number_end<='9')++number_end;
        if(number_end==at||*number_end!='}'||number_end-at>8)return false;
        *at='0';std::memmove(at+1,number_end,std::strlen(number_end)+1);
        from=at+1;++count;
    }
    constexpr char wrapper[]="__haul_application.l(552,";
    if(std::strncmp(canonical,wrapper,sizeof(wrapper)-1))return false;
    char* argument=canonical+sizeof(wrapper)-1;
    if(*argument=='(') {
        auto length=std::strlen(canonical);
        if(length<3||std::strcmp(canonical+length-3,"));"))return false;
        std::memmove(argument,argument+1,std::strlen(argument+1)+1);--length;
        std::memmove(canonical+length-3,canonical+length-2,3);
    }
    return count==5&&digest_is(canonical,std::strlen(canonical)+1,
        "16a279f7f88af66c7d7bd9e6bcb1e52d53a95a25");
}
bool is_home(const std::uint8_t* data,std::size_t size) {
    constexpr char marker[]="isHomeTitleIdDisplayEnabled";
    if(size<sizeof(marker)-1)return false;
    for(std::size_t i=0;i<=size-(sizeof(marker)-1);++i)
        if(data[i]==marker[0]&&!std::memcmp(data+i,marker,sizeof(marker)-1))return true;
    return false;
}
// All edits are staged before any live byte is changed. Each module retains
// its original table entry, capacity and terminal NUL. Space padding keeps
// every other module, source offset and RNPS header/trailer unchanged.
struct Source {
    char* data;std::size_t capacity,used;
    Source(char* out,const std::uint8_t* input,std::size_t size):data(out),capacity(size),used(size-1) {
        std::memcpy(data,input,size);
    }
    bool code_at(const char* position) const {
        char quote=0;bool line=false,block=false;
        for(const char* p=data;p<position;++p) {
            if(line){if(*p=='\n'||*p=='\r')line=false;continue;}
            if(block){if(*p=='*'&&p+1<position&&p[1]=='/'){block=false;++p;}continue;}
            if(quote){if(*p=='\\'&&p+1<position)++p;else if(*p==quote)quote=0;continue;}
            if(*p=='\''||*p=='\"'||*p=='`')quote=*p;
            else if(*p=='/'&&p+1<position&&p[1]=='/'){line=true;++p;}
            else if(*p=='/'&&p+1<position&&p[1]=='*'){block=true;++p;}
            // The known Space prefix has no division/regexp literals. Refuse
            // an ambiguous slash rather than mistake regexp text for code.
            else if(*p=='/')return false;
        }
        return !quote&&!line&&!block;
    }
    bool replace(const char* before,const char* after,bool require_code=false) {
        char* at=std::strstr(data,before);
        const auto from=std::strlen(before),to=std::strlen(after);
        if(!at||!from||std::strstr(at+from,before)||(require_code&&!code_at(at))||used-from+to>=capacity)return false;
        const auto offset=static_cast<std::size_t>(at-data);
        std::memmove(at+to,at+from,used-offset-from+1);
        std::memcpy(at,after,to);used=used-from+to;return true;
    }
    bool range(const char* first,const char* end,const char* replacement) {
        char* left=std::strstr(data,first);
        if(!left||std::strstr(left+std::strlen(first),first))return false;
        char* right=std::strstr(left,end);
        if(!right||std::strstr(right+std::strlen(end),end))return false;
        const auto from=static_cast<std::size_t>(right-left),to=std::strlen(replacement);
        if(used-from+to>=capacity)return false;
        const auto offset=static_cast<std::size_t>(left-data);
        std::memmove(left+to,right,used-offset-from+1);
        std::memcpy(left,replacement,to);used=used-from+to;return true;
    }
    void finish() {std::memset(data+used,' ',capacity-used-1);data[capacity-1]=0;}
};
bool style(char* out,std::size_t size,const Config& config) {
    const char* wrapper=config.outside?"width:106,height:202":"borderRadius:16";
    const int n=std::snprintf(out,size,
        "__haul_application.l(219,function(e,t,i){t.__esModule=!0;"
        "t.default=i(1).StyleSheet.create({imageWrapper:{%s},"
        "image:{width:106,height:106,borderRadius:16},"
        "titleId:{fontSize:i(5).FontSizePS.Size4XSmall,fontWeight:\"bold\",textAlign:\"center\","
        "position:\"absolute\",%s:0,left:0,right:0,color:\"#%06x\"},"
        "telemetryContainer:{position:\"relative\"},"
        "telemetryTile:{position:\"absolute\",transform:[{translateY:-1080}]}})});",
        wrapper,config.top?"top":"bottom",config.color);
    if(n<=0||static_cast<std::size_t>(n)>=size)return false;
    std::memset(out+n,' ',size-static_cast<std::size_t>(n)-1);out[size-1]=0;return true;
}
}
HomeStyleResult patch_home_ram(std::uint8_t* ram,std::size_t size,const Config& config,StyleInput* observed) {
    if(!ram||size<12||size>32*1024*1024||config.color>0xffffff||read32(ram)!=0xfb0bd1e5u)
        return HomeStyleResult::ignored;
    if(observed){observed->format=2;observed->payload_size=static_cast<std::uint32_t>(size);observed->modules=read32(ram+4);observed->startup_size=read32(ram+8);}
    if(read32(ram+4)!=module_count||read32(ram+8)!=startup_size) {
        if(observed)observed->failure=2;
        return is_home(ram,size)?HomeStyleResult::unsupported:HomeStyleResult::ignored;
    }
    if(size<table_base+startup_size) {
        if(observed)observed->failure=1;
        return is_home(ram,size)?HomeStyleResult::unsupported:HomeStyleResult::ignored;
    }
    // Qualify startup first. The live Space component is five bytes shorter
    // than the export, so edits must use each actual slot's length and context.
    if(!digest_is(ram+table_base,startup_size,"67a129582f323eee0448924a0d7523e03259287b")) {
        if(observed)observed->failure=3;
        return is_home(ram,size)?HomeStyleResult::invalid:HomeStyleResult::ignored;
    }
    std::size_t last=table_base+startup_size;
    for(unsigned id=0;id<module_count;++id) {
        const auto* entry=ram+12+8*id;
        const auto relative=read32(entry),n=read32(entry+4);
        const std::size_t at=table_base+static_cast<std::size_t>(relative);
        // Reject overlapping, unordered, unterminated and out-of-range source.
        // Empty unused slots are permitted; every target must still exist.
        if((!n&&relative)||(n&&(at<last||at>size||n>size-at||
           ram[at+n-1]!=0||std::memchr(ram+at,0,n-1)))) {
            if(observed){observed->failure=4;observed->failed_module=id;observed->module_size=n;}
            return HomeStyleResult::invalid;
        }
        if(n)last=at+n;
    }
    if(size-last>0x3fff) {
        if(observed)observed->failure=1;
        return HomeStyleResult::invalid;
    }
    std::size_t offsets[6]{},sizes[6]{};
    for(unsigned i=0;i<6;++i) {
        if(!config.outside&&i!=2)continue;
        const auto& m=modules[i];
        const auto* entry=ram+12+8*m.id;
        offsets[i]=table_base+static_cast<std::size_t>(read32(entry));
        sizes[i]=read32(entry+4);
        bool qualified=false;
        if(i==3) {
            // Space contains launch/focus handlers that are unrelated to this
            // edit. Do not replace or fingerprint those handlers. The sole
            // accepted change is one complete strand style-property context.
            constexpr char wrapper[]="__haul_application.l(527,";
            constexpr char function[]="function(e,t,n){";
            if(sizes[i]>sizeof(wrapper)+sizeof(function)&&sizes[i]<=8192&&
               !std::memcmp(ram+offsets[i],wrapper,sizeof(wrapper)-1)) {
                const char* argument=reinterpret_cast<const char*>(ram+offsets[i])+sizeof(wrapper)-1;
                if(*argument=='(')++argument;
                qualified=!std::memcmp(argument,function,sizeof(function)-1);
            }
        } else if(i==5)qualified=tile_digest(ram+offsets[i],sizes[i]);
        else qualified=sizes[i]==m.size&&digest_is(ram+offsets[i],m.size,m.hash);
        if(!qualified) {
            if(observed){observed->failure=4;observed->failed_module=m.id;observed->module_size=read32(entry+4);}
            return HomeStyleResult::invalid;
        }
    }
    char row[1205],title[1016],labels[567],space[8192],geometry[1507],tile[4096];
    if(!style(labels,sizeof(labels),config)){if(observed)observed->failure=5;return HomeStyleResult::invalid;}
    if(config.outside) {
        Source a(row,ram+offsets[0],sizeof(row));
        Source b(title,ram+offsets[1],sizeof(title));
        Source c(space,ram+offsets[3],sizes[3]);
        Source d(geometry,ram+offsets[4],sizeof(geometry));
        Source e(tile,ram+offsets[5],sizes[5]);
        if(!c.replace("style:[T.default.strandContainer,z.strandContainer],testID:\"strand-container\"",
                      "style:[z.strandContainer,T.default.strandContainer],testID:\"strand-container\"",true)) {
            if(observed){observed->failure=5;observed->failed_module=527;observed->module_size=sizes[3];}
            return HomeStyleResult::invalid;
        }
        if(!a.replace("container:{flexDirection:\"row\",width:1920,height:168}",
                      "container:{flexDirection:\"row\",width:1920,height:336}")||
           !a.replace("strandContainer:{width:1500,height:168}","strandContainer:{width:1500,height:336}")||
           !b.replace("var i=E.EXPERIENCE_SIZE","var i=214")||
           !d.replace("Object.defineProperty(t,\"__esModule\",{value:!0})","t.__esModule=!0")||
           !d.replace("var t=e/2","var t=(e+96)/2")||
           !d.replace("width:e,height:e}","width:e,height:e+96}")||
           !d.replace("{translateY:s}","{translateY:s+48}")||
           !e.range("y=l().default.createElement(i().Image,",",C=[d.default.imageWrapper,d.default.image]",
               "y=l().default.createElement(i().View,{style:d.default.imageWrapper},"
               "l().default.createElement(i().Image,{fallbackSource:h,source:N,"
               "style:[d.default.image,{marginTop:48}]},"
               "l().default.createElement(i().Animated.View,{style:[u,d.default.image]})),"
               "k&&l().default.createElement(i().Text,{style:d.default.titleId},S))")||
           !e.replace("C=[d.default.imageWrapper,d.default.image]","C=[d.default.imageWrapper]"))
            {if(observed)observed->failure=5;return HomeStyleResult::invalid;}
        a.finish();b.finish();c.finish();d.finish();e.finish();
    }
    const char* staged[]={row,title,labels,space,geometry,tile};
    for(unsigned i=0;i<6;++i)if(config.outside||i==2)
        std::memcpy(ram+offsets[i],staged[i],sizes[i]);
    return HomeStyleResult::applied;
}
}
