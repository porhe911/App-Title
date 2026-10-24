// SPDX-License-Identifier: GPL-3.0-or-later
#include "home_style.hpp"
#include "ram_fixture.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
namespace {
std::uint32_t get32(const unsigned char* p){std::uint32_t v;std::memcpy(&v,p,4);return v;}
void put32(unsigned char* p,std::uint32_t v){std::memcpy(p,&v,4);}
}
int main(int argc,char** argv) {
    using namespace app_title;
    if(argc<2){puts("INFO: supply the local 4.03 HomeUI fixture for RAM bundle checks");return 0;}
    std::ifstream input(argv[1],std::ios::binary);assert(input.good());
    const std::vector<unsigned char> original((std::istreambuf_iterator<char>(input)),{});
    assert(original.size()==1389744);
    const auto offset=get32(original.data()+0x1c),plain=get32(original.data()+0x24);
    assert(offset==2848&&plain==1386504&&get32(original.data()+offset)==0xfb0bd1e5u);
    constexpr unsigned ids[]={25,102,219,527,531,552};
    constexpr std::size_t starts[]={67100,148437,308526,727007,739593,771182};
    constexpr std::size_t lengths[]={1205,1016,567,5276,1507,1948};
    Config config{};assert(config.enabled&&!config.top&&config.outside&&config.color==0xffffff);
    const auto live_variant=app_title_test::ram_variant(original);
    assert(live_variant.size()==1389168&&get32(live_variant.data()+0x24)==1385934);
    assert(get32(live_variant.data()+offset+4)==858&&get32(live_variant.data()+offset+8)==1795);
    {std::ofstream out("build-host/ram_variant_input.bin",std::ios::binary);out.write(reinterpret_cast<const char*>(live_variant.data()),live_variant.size());assert(out.good());}
    for(unsigned mode=0;mode<12;++mode) {
        config.outside=mode<4||(mode>=8&&mode<10);config.top=(mode&1)!=0;
        config.color=mode>=8?0:mode%4<2?0xffffff:0x12abef;
        auto bytes=original;
        StyleInput observed{};
        assert(patch_home_style(bytes.data(),bytes.size(),config,&observed)==HomeStyleResult::applied);
        assert(observed.format==2&&observed.buffer_size==original.size()&&observed.payload_size==plain&&observed.modules==858&&observed.startup_size==1795&&observed.failure==0);
        for(std::size_t j=0;j<bytes.size();++j)if(bytes[j]!=original[j]) {
            bool allowed=false;
            if(j>=offset)for(unsigned k=0;k<6;++k)
                if(config.outside||k==2)allowed|=j-offset>=starts[k]&&j-offset<starts[k]+lengths[k];
            assert(allowed);
        }
        // Every table offset/length, startup code, terminal NUL and unrelated
        // module remains at its original address, including all launch logic.
        assert(!std::memcmp(bytes.data(),original.data(),offset+67100));
        for(unsigned i=0;i<858;++i) {
            const auto* entry=bytes.data()+offset+12+8*i;
            const auto at=6876+get32(entry),n=get32(entry+4);
            if(!n)continue;
            assert(at<=plain&&n<=plain-at&&bytes[offset+at+n-1]==0);
            assert(!std::memchr(bytes.data()+offset+at,0,n-1));
        }
        for(unsigned k=0;k<6;++k) {
            const auto* entry=bytes.data()+offset+12+8*ids[k];
            assert(get32(entry)==starts[k]-6876&&get32(entry+4)==lengths[k]);
        }
        const auto copy=bytes;
        assert(patch_home_style(bytes.data(),bytes.size(),config)!=HomeStyleResult::applied&&bytes==copy);
        const std::string path="build-host/ram_style_"+std::to_string(mode)+".bin";
        std::ofstream out(path,std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());assert(out.good());
        // Reproduce the rejected transport shape independently of the source:
        // the payload is identical, but the live buffer includes alignment.
        // Exercise the raw decrypted view as well as the RNPS envelope.
        constexpr std::size_t aligned=(1386504+0x3fff)&~std::size_t(0x3fff);
        for(unsigned form=0;form<5;++form) {
            const bool raw=form>=2;
            const std::size_t payload=form==0?plain+8:form==1||form==4?aligned:form==3?plain+8:plain;
            std::vector<unsigned char> live;
            if(raw) {
                live.assign(original.begin()+offset,original.begin()+offset+plain);
                live.resize(payload,0xa5);
            } else {
                live.assign(original.begin(),original.begin()+offset+plain);
                live.resize(offset+payload+384,0xa5);
                put32(live.data()+0x20,payload);put32(live.data()+0x24,payload);put32(live.data()+0x28,offset+payload);
            }
            const auto transport=live;
            assert(patch_home_style(live.data(),live.size(),config,&observed)==HomeStyleResult::applied);
            const std::size_t start=raw?0:offset;
            assert(!std::memcmp(live.data()+start,bytes.data()+offset,plain));
            assert(!std::memcmp(live.data(),transport.data(),start));
            assert(!std::memcmp(live.data()+start+plain,transport.data()+start+plain,live.size()-start-plain));
            assert(observed.format==2&&observed.buffer_size==live.size()&&observed.payload_size==payload&&observed.failure==0);
            auto corrupt=transport;corrupt[start+starts[2]+100]^=1;
            const auto copy=corrupt;
            assert(patch_home_style(corrupt.data(),corrupt.size(),config,&observed)==HomeStyleResult::invalid&&observed.failure==4&&observed.failed_module==219&&corrupt==copy);
        }
        // Reproduce the live dimensions from the console log. The variant
        // removes 570 bytes of redundant syntax outside all six target modules.
        // Every target must still produce the exact original patch at its new
        // table address, while all other source and transport stays untouched.
        for(unsigned variant=0;variant<3;++variant) {
            const auto input=variant==0?live_variant:variant==1?app_title_test::ram_variant(original,1):app_title_test::ram_variant(original,0,true);
            auto current=input;
            assert(patch_home_style(current.data(),current.size(),config,&observed)==HomeStyleResult::applied);
            assert(observed.payload_size==get32(input.data()+0x24)&&observed.failure==0);
            assert(!std::memcmp(current.data(),input.data(),offset+6876+1795));
            for(unsigned id=0;id<858;++id) {
                const auto* entry=input.data()+offset+12+8*id;
                const auto at=offset+6876+get32(entry),n=get32(entry+4);
                if((config.outside&&app_title_test::ram_target(id))||id==219) {
                    unsigned k=0;while(k<6&&ids[k]!=id)++k;assert(k<6);
                    assert(n==lengths[k]&&!std::memcmp(current.data()+at,bytes.data()+offset+starts[k],n));
                } else assert(!std::memcmp(current.data()+at,input.data()+at,n));
            }
            const auto logical=offset+get32(input.data()+0x24);
            assert(!std::memcmp(current.data()+logical,input.data()+logical,input.size()-logical));
            if(variant==0) {
                std::ofstream out("build-host/ram_variant_style_"+std::to_string(mode)+".bin",std::ios::binary);
                out.write(reinterpret_cast<const char*>(current.data()),current.size());assert(out.good());
            }
        }
    }
    config={};
    for(unsigned mode=0;mode<23;++mode) {
        auto bytes=original;
        if(mode<6)bytes[offset+starts[mode]]^=1;
        else if(mode<12)bytes[offset+12+8*ids[mode-6]]^=1;
        else switch(mode) {
            case 12:bytes[offset+12+8*554+4]^=1;break;
            case 13:bytes[offset+6876+1795-2]^=1;break;
            case 14:put32(bytes.data()+offset+4,859);break;
            case 15:put32(bytes.data()+offset+8,1796);break;
            case 16:put32(bytes.data()+0x24,128);break;
            case 17:put32(bytes.data()+0x24,plain-1);break;
            case 18:put32(bytes.data()+0x28,0xffffffffu);break;
            case 19:put32(bytes.data()+0x1c,0xfffffff0u);break;
            case 20:bytes[offset+starts[2]+lengths[2]-1]='x';break;
            case 21:bytes[offset+starts[5]+120]=0;break;
            case 22: {
                const auto text=reinterpret_cast<const char*>(bytes.data()+offset+starts[2]);
                const auto at=std::strstr(text,"fontWeight:\"bold\"");assert(at);
                bytes[static_cast<std::size_t>(at-reinterpret_cast<const char*>(bytes.data()))+12]='c';break;
            }
        }
        const auto copy=bytes;
        assert(patch_home_style(bytes.data(),bytes.size(),config)!=HomeStyleResult::applied&&bytes==copy);
    }
    auto unchanged=original;config.color=0x1000000;
    assert(patch_home_style(unchanged.data(),unchanged.size(),config)==HomeStyleResult::ignored&&unchanged==original);
    config={};
    for(unsigned k=0;k<6;++k) {
        auto corrupt=live_variant;const auto* entry=corrupt.data()+offset+12+8*ids[k];
        const auto at=offset+6876+get32(entry);
        if(k==3) {
            auto* context=std::strstr(reinterpret_cast<char*>(corrupt.data()+at),"style:[T.default.strandContainer,z.strandContainer],testID:");
            assert(context);context[7]='U';
        } else if(k==5) {
            auto* context=std::strstr(reinterpret_cast<char*>(corrupt.data()+at),".default.titleId");assert(context);context[15]='Z';
        } else corrupt[at+100]^=1;
        const auto copy=corrupt;StyleInput detail{};
        assert(patch_home_style(corrupt.data(),corrupt.size(),config,&detail)==HomeStyleResult::invalid&&corrupt==copy);
        assert(detail.failure==(k==3||k==5?5u:4u)&&detail.failed_module==ids[k]&&detail.module_size==lengths[k]);
    }
    auto missing=live_variant;put32(missing.data()+offset+12+8*219,0);put32(missing.data()+offset+16+8*219,0);
    const auto missing_before=missing;StyleInput missing_detail{};
    assert(patch_home_style(missing.data(),missing.size(),config,&missing_detail)==HomeStyleResult::invalid&&missing==missing_before);
    assert(missing_detail.failure==4&&missing_detail.failed_module==219&&missing_detail.module_size==0);
    for(unsigned mode=0;mode<6;++mode) {
        auto bad=live_variant;const auto entry=offset+12+8*4;
        const auto at=offset+6876+get32(bad.data()+entry),n=get32(bad.data()+entry+4);
        if(mode==0)put32(bad.data()+entry,0xffffffffu);
        if(mode==1)put32(bad.data()+entry+4,0xffffffffu);
        if(mode==2)put32(bad.data()+entry,0);
        if(mode==3)put32(bad.data()+entry+4,0);
        if(mode==4)put32(bad.data()+entry,get32(bad.data()+entry-8));
        if(mode==5)bad[at+n-1]='x';
        const auto copy=bad;StyleInput detail{};
        assert(patch_home_style(bad.data(),bad.size(),config,&detail)==HomeStyleResult::invalid&&bad==copy&&detail.failure==4);
    }
    auto unknown=original;put32(unknown.data()+offset+4,859);StyleInput observed{};
    assert(patch_home_style(unknown.data(),unknown.size(),config,&observed)==HomeStyleResult::unsupported);
    assert(observed.format==2&&observed.modules==859&&observed.startup_size==1795&&observed.failure==2);
    assert(patch_home_style(unchanged.data(),plain-1,config,&observed)==HomeStyleResult::ignored);
    for(std::size_t n:{0u,1u,63u,64u,2848u,4096u,1389360u-1}) {
        assert(patch_home_style(unchanged.data(),n,config)!=HomeStyleResult::applied&&unchanged==original);
    }
    puts("PASS: 4.03 RAM, 12 styles, 60 raw/padded and 36 relocated variants including the observed 1385934-byte shape; startup/source guards, untouched unrelated source/transport, malformed tables and target edits refused with module details");
}
