// SPDX-License-Identifier: GPL-3.0-or-later
// Host-only RAM transport fixtures, derived locally from the user's input.
#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
namespace app_title_test {
inline std::uint32_t ram_u32(const std::vector<unsigned char>& b,std::size_t at) {
    assert(at+4<=b.size());std::uint32_t n;std::memcpy(&n,b.data()+at,4);return n;
}
inline void ram_put(std::vector<unsigned char>& b,std::size_t at,std::uint32_t n) {
    assert(at+4<=b.size());std::memcpy(b.data()+at,&n,4);
}
inline bool ram_target(unsigned id) {return id==25||id==102||id==219||id==527||id==531||id==552;}
template<class Edit> inline std::vector<unsigned char> ram_rewrite(const std::vector<unsigned char>& input,Edit edit) {
    const auto start=ram_u32(input,0x1c),plain=ram_u32(input,0x24),count=ram_u32(input,start+4);
    const auto base=12+8*count,startup=ram_u32(input,start+8),old_end=ram_u32(input,0x28);
    assert(count==858&&startup==1795&&start+plain<=input.size()&&old_end<=input.size());
    std::vector<unsigned char> ram(input.begin()+start,input.begin()+start+base+startup);
    for(unsigned id=0;id<count;++id) {
        const auto at=start+base+ram_u32(input,start+12+8*id),n=ram_u32(input,start+16+8*id);
        assert(n&&at+n<=start+plain&&input[at+n-1]==0);
        std::string text(reinterpret_cast<const char*>(input.data()+at),n-1);
        edit(id,text);
        ram_put(ram,12+8*id,static_cast<std::uint32_t>(ram.size()-base));
        ram_put(ram,16+8*id,static_cast<std::uint32_t>(text.size()+1));
        ram.insert(ram.end(),text.begin(),text.end());ram.push_back(0);
    }
    const auto region=(ram.size()+15)&~std::size_t(15);
    std::vector<unsigned char> out(input.begin(),input.begin()+start);
    out.insert(out.end(),ram.begin(),ram.end());out.resize(start+region,0xa5);
    out.insert(out.end(),input.begin()+old_end,input.end());
    ram_put(out,0x20,static_cast<std::uint32_t>(region));ram_put(out,0x24,static_cast<std::uint32_t>(ram.size()));
    ram_put(out,0x28,static_cast<std::uint32_t>(start+region));return out;
}
inline std::vector<unsigned char> ram_variant(const std::vector<unsigned char>& input,unsigned unwrap=285,bool grow=false,unsigned debug=0) {
    unsigned changed=0;
    auto result=ram_rewrite(input,[&](unsigned id,std::string& text) {
        const auto prefix="__haul_application.l("+std::to_string(id)+",(";
        // Removing parentheses around a function expression argument preserves
        // the complete function body and behavior, while moving later modules.
        if(changed<unwrap&&!ram_target(id)&&text.compare(0,prefix.size()+8,prefix+"function")==0&&
           text.size()>=3&&text.compare(text.size()-3,3,"));")==0) {
            text.erase(text.size()-3,1);text.erase(prefix.size()-1,1);++changed;
        }
        if(grow&&id==19)text+="/* transport variant */";
        if(debug) {
            const auto at=text.find("\"/home/jenkins/");
            if(at!=std::string::npos) {
                // Filename literals belong to __source metadata, not runtime
                // paths. Mode 1 shrinks each of 110 paths by five bytes.
                if(debug==1)text.erase(at+1,5);
                else text.insert(at+1,"/"+std::string(debug==2?300:20,'x'));
            }
            if(debug==3) {
                std::size_t at=0;
                while((at=text.find("lineNumber:",at))!=std::string::npos) {
                    at+=11;const auto end=text.find_first_not_of("0123456789",at);
                    assert(end!=std::string::npos&&end>at);text.replace(at,end-at,"123456");at+=6;
                }
                if(id==527||id==552) {
                    const auto prefix="__haul_application.l("+std::to_string(id)+",(";
                    assert(text.compare(0,prefix.size(),prefix)==0&&text.compare(text.size()-3,3,"));")==0);
                    text.erase(text.size()-3,1);text.erase(prefix.size()-1,1);
                }
                if(id==527)text+="/* focus and launch source retained */";
            }
        }
    });
    assert(changed==unwrap);
    return result;
}
}
