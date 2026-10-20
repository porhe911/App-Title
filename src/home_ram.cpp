// SPDX-License-Identifier: GPL-3.0-or-later
// Exact 4.03 HomeUI Indexed RAM bundle profile. Only module metadata and our
// generated styles/replacements are shipped; the console's source stays local.
#include "home_style.hpp"
#include <cstdio>
#include <cstring>

namespace app_title {
namespace {
constexpr std::size_t plain_size=1386504,table_base=6876;
struct Module {unsigned id;std::size_t offset,size;const char* hash;};
constexpr Module modules[]={
    {25,67100,1205,"09a2036b256701606f04f3b524e2a66e379df0dc"},
    {102,148437,1016,"d9dd1aa657d5110c50915619015184104f317501"},
    {219,308526,567,"713c438f042772f45fdac128325454d60d59216b"},
    {527,727007,5276,"a74aec5cb8490615612e42b272b539c25ed4d23f"},
    {531,739593,1507,"a35b627618c3b5a8b0d23f4f5f0d1bc85a9c7599"},
    {552,771182,1948,"2aef30b24d50393bdd47650dfdfbec1d8e93efa1"}
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
    bool replace(const char* before,const char* after) {
        char* at=std::strstr(data,before);
        const auto from=std::strlen(before),to=std::strlen(after);
        if(!at||!from||std::strstr(at+from,before)||used-from+to>=capacity)return false;
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
    // RNPS transport/alignment bytes do not belong to the RAM source digest.
    // Accept the same exact complete source in a larger decrypted allocation;
    // never read less than its full size, or write any byte in the extra tail.
    constexpr std::size_t aligned_size=(plain_size+0x3fff)&~std::size_t(0x3fff);
    if(size<plain_size||size>aligned_size) {
        if(observed)observed->failure=1;
        return is_home(ram,size)?HomeStyleResult::unsupported:HomeStyleResult::ignored;
    }
    if(read32(ram+4)!=858||read32(ram+8)!=1795) {
        if(observed)observed->failure=2;
        return is_home(ram,size)?HomeStyleResult::unsupported:HomeStyleResult::ignored;
    }
    if(!digest_is(ram,plain_size,"1f57578fcf36d2c85281c142d76e39fc69c01fa8")) {
        if(observed)observed->failure=3;
        return is_home(ram,size)?HomeStyleResult::invalid:HomeStyleResult::ignored;
    }
    for(const auto& m:modules) {
        const auto* entry=ram+12+8*m.id;
        if(read32(entry)!=m.offset-table_base||read32(entry+4)!=m.size||
           ram[m.offset+m.size-1]!=0||std::memchr(ram+m.offset,0,m.size-1)||
           !digest_is(ram+m.offset,m.size,m.hash)) {
            if(observed)observed->failure=4;
            return HomeStyleResult::invalid;
        }
    }
    char row[1205],title[1016],labels[567],space[5276],geometry[1507],tile[1948];
    if(!style(labels,sizeof(labels),config)){if(observed)observed->failure=5;return HomeStyleResult::invalid;}
    if(config.outside) {
        Source a(row,ram+modules[0].offset,sizeof(row));
        Source b(title,ram+modules[1].offset,sizeof(title));
        Source c(space,ram+modules[3].offset,sizeof(space));
        Source d(geometry,ram+modules[4].offset,sizeof(geometry));
        Source e(tile,ram+modules[5].offset,sizeof(tile));
        if(!a.replace("container:{flexDirection:\"row\",width:1920,height:168}",
                      "container:{flexDirection:\"row\",width:1920,height:336}")||
           !a.replace("strandContainer:{width:1500,height:168}","strandContainer:{width:1500,height:336}")||
           !b.replace("var i=E.EXPERIENCE_SIZE","var i=214")||
           !c.replace("[T.default.strandContainer,z.strandContainer]","[z.strandContainer,T.default.strandContainer]")||
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
        std::memcpy(ram+modules[i].offset,staged[i],modules[i].size);
    return HomeStyleResult::applied;
}
}
