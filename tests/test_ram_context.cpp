// SPDX-License-Identifier: GPL-3.0-or-later
// Regress live module lengths as well as the outer buffer dimensions.
#include "home_style.hpp"
#include "ram_fixture.hpp"
#include <fstream>
#include <iterator>
using Bytes=std::vector<unsigned char>;
using namespace app_title_test;
namespace {
std::pair<std::size_t,std::size_t> slot(const Bytes& b,unsigned id) {
    const auto start=ram_u32(b,0x1c),base=12+8*ram_u32(b,start+4);
    return {start+base+ram_u32(b,start+12+8*id),ram_u32(b,start+16+8*id)};
}
std::string source(const Bytes& b,unsigned id) {
    const auto [at,n]=slot(b,id);return {reinterpret_cast<const char*>(b.data()+at),n-1};
}
void save(const std::string& path,const Bytes& b) {
    std::ofstream out(path,std::ios::binary);out.write(reinterpret_cast<const char*>(b.data()),b.size());assert(out.good());
}
void replace(std::string& s,const std::string& from,const std::string& to) {
    const auto at=s.find(from);assert(at!=std::string::npos);s.replace(at,from.size(),to);
}
void replace_all(std::string& s,const std::string& from,const std::string& to) {
    std::size_t p=0;while((p=s.find(from,p))!=std::string::npos){s.replace(p,from.size(),to);p+=to.size();}
}
constexpr char old_context[]="style:[T.default.strandContainer,z.strandContainer],testID:\"strand-container\"";
constexpr char new_context[]="style:[z.strandContainer,T.default.strandContainer],testID:\"strand-container\"";
}
int main(int argc,char** argv) {
    using namespace app_title;
    assert(argc==2);std::ifstream in(argv[1],std::ios::binary);assert(in.good());
    const Bytes reference((std::istreambuf_iterator<char>(in)),{});
    const auto compact=ram_variant(reference,10,false,1);
    assert(compact.size()==1389168&&ram_u32(compact,0x24)==1385934);
    assert(slot(compact,527).second==5271&&slot(compact,552).second==1943);
    unsigned accepted=0;
    for(unsigned variant=0;variant<4;++variant) {
        const auto input=variant==0?compact:variant<3?ram_variant(reference,0,false,variant+1):ram_rewrite(compact,[](unsigned id,std::string& s) {
            if(id==527||id==552) {
                replace_all(s,"function l()","function reactGetter()");replace_all(s,"return l=function","return reactGetter=function");replace_all(s,"l().","reactGetter().");
            }
            if(id==527){replace(s,",T=r(n(25))",",rowStyles=r(n(25))");replace_all(s,"T.","rowStyles.");replace(s,"var j=","var auxiliary=6/2;var j=");}
            if(id==552) {
                replace_all(s,"function i()","function nativeGetter()");replace_all(s,"return i=function","return nativeGetter=function");replace_all(s,"i().","nativeGetter().");
                replace(s,"style:d.default.image,__source","style:d.default.image,onError:f,resizeMode:\"contain\",__source");
                replace(s,",d=n(r(219))",",tileStyles=n(r(219))");replace_all(s,"d.default.","tileStyles.default.");
            }
        });
        save("build-host/ram_context_input_"+std::to_string(variant)+".bin",input);
        for(unsigned mode=0;mode<12;++mode) {
            Config c{};c.outside=mode<4||(mode>=8&&mode<10);c.top=(mode&1)!=0;
            c.color=mode>=8?0:mode%4<2?0xffffff:0x12abef;
            auto actual=input;StyleInput detail{};
            assert(patch_home_style(actual.data(),actual.size(),c,&detail)==HomeStyleResult::applied);
            assert(!detail.failure&&detail.payload_size==ram_u32(input,0x24));
            const auto start=ram_u32(input,0x1c);
            assert(!std::memcmp(actual.data(),input.data(),start+6876+1795));
            for(unsigned id=0;id<858;++id) {
                const auto [at,n]=slot(input,id);assert(slot(actual,id)==slot(input,id));
                if((!c.outside||!ram_target(id))&&id!=219)
                    assert(!std::memcmp(actual.data()+at,input.data()+at,n));
                assert(actual[at+n-1]==0&&!std::memchr(actual.data()+at,0,n-1));
            }
            if(c.outside) {
                // Undo the single, same-length edit and require every other
                // byte of the live Space component (launch/focus included).
                auto restored=source(actual,527);
                if(variant==3)replace(restored,"style:[z.strandContainer,rowStyles.default.strandContainer]","style:[rowStyles.default.strandContainer,z.strandContainer]");
                else replace(restored,new_context,old_context);
                assert(restored==source(input,527));
            }
            const auto tail=start+ram_u32(input,0x24);
            assert(!std::memcmp(actual.data()+tail,input.data()+tail,input.size()-tail));
            save("build-host/ram_context_"+std::to_string(variant)+"_style_"+std::to_string(mode)+".bin",actual);
            // Also run the compact/growing target slots as raw decrypted RAM.
            Bytes raw(input.begin()+start,input.begin()+tail);StyleInput raw_detail{};
            assert(patch_home_style(raw.data(),raw.size(),c,&raw_detail)==HomeStyleResult::applied);
            assert(!std::memcmp(raw.data(),actual.data()+start,raw.size()));accepted+=2;
        }
    }
    for(unsigned bad=0;bad<8;++bad) {
        auto input=ram_rewrite(compact,[&](unsigned id,std::string& s) {
            if(id==527) {
                if(bad==0)replace(s,old_context,"style:[],testID:\"strand-container\"");
                if(bad==1)replace(s,"t.default=j","void l().default.createElement(o().Animated.View,{"+std::string(old_context)+"});t.default=j");
                if(bad==2) {
                    replace(s,"testID:\"strand-container\"","testID:\"strand-disabled\"");
                    replace(s,"t.default=j","var fake='l().default.createElement(o().Animated.View,{"+std::string(old_context)+"})';t.default=j");
                }
                if(bad==3)s+=std::string(8192,' ');
                if(bad==7) {
                    replace(s,"testID:\"strand-container\"","testID:\"strand-disabled\"");
                    replace(s,"t.default=j","var fake=/l().default.createElement(o().Animated.View,{"+std::string(old_context)+"})/;t.default=j");
                }
            }
            if(id==552) {
                if(bad==4)replace(s,"style:d.default.titleId","style:d.default.image");
                if(bad==5)replace(s,"d=n(r(219))","d=n(r(220))");
                if(bad==6)replace(s,"k&&l().default.createElement","k||l().default.createElement");
            }
        });
        const auto before=input;StyleInput detail{};Config c{};
        assert(patch_home_style(input.data(),input.size(),c,&detail)==HomeStyleResult::invalid&&input==before);
        assert(detail.failed_module==(bad<4||bad==7?527u:552u));
        // Inside mode only changes the independently verified style module.
        // Incompatible outside-only components must not block that option.
        c.outside=false;
        assert(patch_home_style(input.data(),input.size(),c)==HomeStyleResult::applied);
        for(unsigned id=0;id<858;++id)if(id!=219)assert(source(input,id)==source(before,id));
    }
    assert(accepted==96);
    puts("PASS: 96 envelope/raw cases with compact/growing slots, renamed React/native/style bindings and retained image callbacks/props; exact untouched Space launch/focus source; ambiguous/non-code/malformed render contexts refused before any write; inside mode independent");
}
