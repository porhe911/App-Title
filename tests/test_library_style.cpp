// SPDX-License-Identifier: GPL-3.0-or-later
#include "home_style.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>
namespace {
std::uint32_t get32(const unsigned char* p){std::uint32_t v;std::memcpy(&v,p,4);return v;}
void put32(unsigned char* p,std::uint32_t v){std::memcpy(p,&v,4);}
// Numeric metadata of the optional user-supplied 11.40 fixture.
constexpr std::size_t plain_size=3133948,region_first=0x1b0796,region_last=0x1b0837;
constexpr unsigned report_function=6694,label_function=6695;
}
int main(int argc,char** argv) {
    using namespace app_title;
    unsigned char malformed[512]{};assert(patch_library_style(malformed,sizeof(malformed))==HomeStyleResult::ignored);
    std::memcpy(malformed,"RNPSHEDR",8);put32(malformed+8,2);put32(malformed+0x1c,64);put32(malformed+0x20,448);put32(malformed+0x24,448);put32(malformed+0x28,512);
    std::memcpy(malformed+64,"\xc6\x1f\xbc\x03\xc1\x03\x19\x1f",8);
    const std::vector<unsigned char> before(malformed,malformed+sizeof(malformed));
    assert(patch_library_style(malformed,sizeof(malformed))==HomeStyleResult::ignored);
    assert(!std::memcmp(malformed,before.data(),sizeof(malformed)));
    for(std::size_t n=0;n<sizeof(malformed);++n)(void)patch_library_style(malformed,n);
    if(argc>=2) {
        std::ifstream input(argv[1],std::ios::binary);assert(input.good());
        const std::vector<unsigned char> original((std::istreambuf_iterator<char>(input)),{});
        assert(original.size()>=64&&!std::memcmp(original.data(),"RNPSHEDR",8));
        const auto off=get32(original.data()+0x1c),plain=get32(original.data()+0x24);
        assert(plain==plain_size&&off>=64&&off<=original.size()&&plain<=original.size()-off);
        // HomeUI configuration never affects the Library and vice versa.
        auto bytes=original;Config config{};
        assert(patch_home_style(bytes.data(),bytes.size(),config)==HomeStyleResult::ignored&&bytes==original);
        assert(patch_library_style(bytes.data(),bytes.size())==HomeStyleResult::applied);
        for(std::size_t j=0;j<bytes.size();++j)if(bytes[j]!=original[j]) {
            bool allowed=false;
            if(j>=off) {
                const auto at=j-off;
                allowed|=at>=region_first&&at<region_last;
                for(const auto fn:{report_function,label_function})allowed|=at>=128+16*fn&&at<144+16*fn;
                allowed|=at>=plain-20&&at<plain;
            }
            assert(allowed);
        }
        const auto* hbc=bytes.data()+off;
        std::uint8_t hash[20];sha1_digest(hbc,plain-20,hash);assert(!std::memcmp(hash,hbc+plain-20,20));
        const auto report=hbc+128+16*report_function,label=hbc+128+16*label_function;
        assert((get32(report)&0x1ffffff)==region_first&&(get32(report+4)&0x7fff)==85);
        assert((get32(label)&0x1ffffff)==region_first+85&&(get32(label+4)&0x7fff)==74&&get32(label+8)>>25==4);
        assert(region_first+85+74<=region_last);
        std::ofstream output("build-host/library_style.hbc",std::ios::binary);output.write(reinterpret_cast<const char*>(hbc),plain);assert(output.good());
        // Already patched or altered inputs are refused without any write.
        for(unsigned mode=0;mode<6;++mode) {
            auto altered=mode?original:bytes;
            switch(mode) {
                case 1:altered[off+region_first]^=1;break;
                case 2:altered[off+0x1b07f4]^=1;break;
                case 3:altered[off+128+16*label_function+8]^=1;break;
                case 4:altered[off+plain-1]^=1;break;
                case 5:put32(altered.data()+off+8,90);break;
            }
            const auto copy=altered;assert(patch_library_style(altered.data(),altered.size())==HomeStyleResult::invalid);assert(altered==copy);
        }
        puts("PASS: supplied Game Library patched; only the two adjacent functions, their headers and checksum changed; patched/altered inputs refused intact");
    } else puts("INFO: supply the 11.40 NPXS40071 bundle to additionally validate the known console profile");
    puts("PASS: malformed and unrelated bundles ignored without modification");
}
