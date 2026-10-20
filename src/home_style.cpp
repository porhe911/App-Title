// SPDX-License-Identifier: GPL-3.0-or-later
#include "home_style.hpp"
#include <cstring>

namespace app_title {
namespace {
std::uint32_t read32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0])|(static_cast<std::uint32_t>(p[1])<<8)|
        (static_cast<std::uint32_t>(p[2])<<16)|(static_cast<std::uint32_t>(p[3])<<24);
}
void write32(std::uint8_t* p,std::uint32_t n) {for(unsigned i=0;i<4;++i)p[i]=static_cast<std::uint8_t>(n>>(8*i));}
std::uint32_t rol(std::uint32_t n,unsigned shift){return (n<<shift)|(n>>(32-shift));}
void sha1_block(std::uint32_t state[5],const std::uint8_t* block) {
    std::uint32_t w[80];
    for(unsigned i=0;i<16;++i)w[i]=(static_cast<std::uint32_t>(block[i*4])<<24)|
        (static_cast<std::uint32_t>(block[i*4+1])<<16)|(static_cast<std::uint32_t>(block[i*4+2])<<8)|block[i*4+3];
    for(unsigned i=16;i<80;++i)w[i]=rol(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
    auto a=state[0],b=state[1],c=state[2],d=state[3],e=state[4];
    for(unsigned i=0;i<80;++i) {
        std::uint32_t f,k;
        if(i<20){f=(b&c)|(~b&d);k=0x5a827999;}
        else if(i<40){f=b^c^d;k=0x6ed9eba1;}
        else if(i<60){f=(b&c)|(b&d)|(c&d);k=0x8f1bbcdc;}
        else {f=b^c^d;k=0xca62c1d6;}
        const auto next=rol(a,5)+f+e+k+w[i];e=d;d=c;c=rol(b,30);b=a;a=next;
    }
    state[0]+=a;state[1]+=b;state[2]+=c;state[3]+=d;state[4]+=e;
}
// Hermes v89 opcodes and string IDs for the verified HomeUI profile. Numeric
// metadata only; no system bytecode, strings blob or decrypted bundle is shipped.
struct Writer {
    std::uint8_t bytes[199]{};std::size_t used=0;bool ok=true;bool modern=false;
    explicit Writer(bool fw1360=false):modern(fw1360){}
    unsigned id(unsigned n) const {
        if(!modern)return n;
        // Audited string IDs in the stock 13.600.007 HomeUI. These are IDs,
        // not offsets into the older profile or global search replacements.
        switch(n) {
            case 58:return 54;case 5305:return 6515;case 9034:return 9180;
            case 5800:return 5873;case 145:return 143;case 27:return 26;
            case 4494:return 4527;case 59:return 55;case 3067:return 3128;
            case 197:return 198;case 240:return 242;case 4859:return 4895;
            case 8486:return 8574;case 4583:return 4634;
            case 4220:return 4260;case 47:return 45;case 107:return 105;
            case 10650:return 10779;case 5015:return 5021;case 237:return 238;
            case 6883:return 6676;case 110:return 108;case 6670:return 9942;
            case 166:return 169;case 5150:return 5163;case 19:return 18;
            default:return n;
        }
    }
    void u8(unsigned n){if(used<sizeof(bytes))bytes[used++]=static_cast<std::uint8_t>(n);else ok=false;}
    void u16(unsigned n){u8(n);u8(n>>8);}
    void op2(unsigned op,unsigned a){u8(op);u8(a);}
    void op3(unsigned op,unsigned a,unsigned b){u8(op);u8(a);u8(b);}
    void get(unsigned dst,unsigned object,unsigned cache,unsigned id) {
        id=this->id(id);
        u8(id<256?52:53);u8(dst);u8(object);u8(cache);if(id<256)u8(id);else u16(id);
    }
    void put(unsigned object,unsigned value,unsigned id) {
        id=this->id(id);
        u8(id<256?61:62);u8(object);u8(value);if(id<256)u8(id);else u16(id);
    }
    void cached_put(unsigned object,unsigned value,unsigned cache,unsigned id) {
        id=this->id(id);
        u8(57);u8(object);u8(value);u8(cache);u16(id);
    }
    void call(unsigned dst,unsigned fn,unsigned self,unsigned arg){u8(81);u8(dst);u8(fn);u8(self);u8(arg);}
    void string(unsigned dst,unsigned id){u8(113);u8(dst);u16(this->id(id));}
    void number(unsigned dst,double n) {
        static_assert(sizeof(double)==8,"Hermes double ABI");
        std::uint64_t bits=0;std::memcpy(&bits,&n,8);u8(110);u8(dst);
        for(unsigned i=0;i<8;++i)u8(static_cast<unsigned>(bits>>(8*i)));
    }
};
constexpr std::uint8_t source_hash[20]={0xf3,0x21,0xf8,0x3f,0x91,0x43,0x03,0x5f,0x5d,0x97,0xee,0x5a,0xd9,0x8c,0xeb,0x75,0x13,0x3c,0x89,0x0e};
constexpr std::uint8_t bytecode_hash[20]={0x3d,0xac,0xc3,0x1c,0x20,0x4c,0x03,0x7c,0xb7,0x92,0x2f,0x3b,0xf8,0x63,0xcf,0x96,0x2b,0x23,0x49,0x7e};
struct ProfileFunction {
    unsigned number;std::size_t offset,size;std::uint8_t header[16],hash[20];
};
constexpr ProfileFunction functions1160[]={
    {1461,0xe3d20,183,{0x20,0x3d,0x0e,0x08,0xb7,0x80,0x7f,0x00,0xb8,0x1e,0x1b,0x26,0x00,0x08,0x02,0x02},{0x35,0xc4,0x17,0xac,0x64,0x05,0xc8,0xa6,0xee,0xa5,0x55,0xae,0xa8,0x63,0xfb,0x1e,0xf1,0x81,0x2f,0x5d}},
    {6665,0x15b120,622,{0x20,0xb1,0x15,0x04,0x6e,0x02,0xa1,0x00,0xc4,0x26,0x1b,0x3c,0x05,0x18,0x00,0x02},{0x05,0xed,0x40,0x11,0x01,0xd8,0x69,0x98,0xdc,0x48,0xb9,0x49,0xc5,0xb2,0x97,0x1e,0xbf,0xe9,0x2f,0xc5}},
    {6513,0x15654a,78,{0x4a,0x65,0x15,0x04,0x4e,0x80,0x7f,0x00,0x08,0x26,0x1b,0x1c,0x00,0x03,0x00,0x02},{0x02,0x74,0x65,0x70,0xa1,0x7f,0x8b,0x2b,0x4d,0x86,0xad,0xd7,0x79,0x17,0x6d,0xc3,0x32,0x21,0x5b,0x48}},
    {6509,0x156394,266,{0x94,0x63,0x15,0x04,0x0a,0x81,0x7f,0x00,0x08,0x26,0x1b,0x28,0x06,0x0d,0x00,0x02},{0x4a,0x4a,0x53,0x35,0xd1,0x92,0x95,0xdf,0x89,0x47,0x7f,0x60,0xd9,0x85,0xef,0x1c,0x2a,0xd4,0xc2,0xc6}},
    {282,0xcbf25,402,{0x25,0xbf,0x0c,0x08,0x92,0x81,0x7f,0x00,0x48,0x1d,0x1b,0x28,0x00,0x06,0x0b,0x02},{0x0c,0xab,0x13,0xf3,0x0f,0x84,0xc6,0xc4,0x5e,0x23,0x70,0x15,0xf6,0xbb,0x75,0x3e,0x55,0xb3,0x28,0x74}},
    {6448,0x153de8,1747,{0xe8,0x3d,0x15,0x06,0xd3,0x86,0x7f,0x00,0x08,0x26,0x1b,0x58,0x23,0x2f,0x00,0x02},{0x78,0x74,0x76,0xd1,0x6e,0xdc,0x45,0x72,0xf6,0xb2,0x56,0x01,0x7e,0x57,0xd8,0xeb,0x3c,0x33,0x12,0x28}},
    {1079,0xdaff4,361,{0xf4,0xaf,0x0d,0x08,0x69,0x81,0x7f,0x00,0x44,0x1e,0x1b,0x26,0x00,0x07,0x07,0x02},{0x25,0x70,0xcf,0x43,0x11,0xad,0xb1,0xbc,0x97,0xcd,0xc3,0xd6,0x26,0x3a,0x9c,0x22,0xd2,0x20,0x65,0xdc}},
};
#include "home_style_1360.inc"
constexpr std::size_t tile_size=622;
std::size_t build_outer_tile(const std::uint8_t* original,std::uint8_t output[tile_size],bool modern=false) {
    std::size_t used=0;
    const auto copy=[&](std::size_t first,std::size_t last){const auto n=last-first;std::memcpy(output+used,original+first,n);used+=n;};
    const auto emit=[&](const std::uint8_t* data,std::size_t n){std::memcpy(output+used,data,n);used+=n;};
    copy(0,0x8e);
    constexpr std::uint8_t source_height[]={8,7,8};emit(source_height,sizeof(source_height));
    copy(0x98,0x160);
    constexpr std::uint8_t animated_style[]={66,17,11,1};emit(animated_style,sizeof(animated_style));
    copy(0x173,0x19b); // original TitleID; retain its registry-backed enabled atom
    copy(0x19f,0x1b5); // Animated.View has its original mat style, no ID child
    // Only Image is displaced down by 48. Its animated mat keeps square bounds.
    const std::uint8_t image_style[]={7,17,2,0,66,17,11,0,3,14,108,13,48,61,14,13,static_cast<std::uint8_t>(modern?169:166),66,17,14,1,61,5,17,static_cast<std::uint8_t>(modern?229:228)};
    emit(image_style,sizeof(image_style));
    copy(0x1b9,0x1c3);
    // Both children now occupy positive coordinates in the 202-high View.
    constexpr std::uint8_t children[]={7,17,2,0,66,17,5,0,66,17,16,1};emit(children,sizeof(children));
    copy(0x1c3,0x1d2);
    constexpr std::uint8_t wrapper_style[]={8,7,8};emit(wrapper_style,sizeof(wrapper_style));
    copy(0x1ed,0x23a);
    const std::uint8_t normal_children[]={61,0,17,static_cast<std::uint8_t>(modern?92:94)};emit(normal_children,sizeof(normal_children));
    copy(0x23e,0x262);
    const std::uint8_t capture_children[]={61,1,17,static_cast<std::uint8_t>(modern?92:94)};emit(capture_children,sizeof(capture_children));
    copy(0x266,tile_size);
    // Every relative branch has both endpoints on the same side of each edit.
    return used;
}
std::size_t build_geometry(std::uint8_t output[78],bool modern=false) {
    Writer w(modern);
    w.op3(106,0,1);w.get(3,0,1,6670);w.op2(3,4);w.string(5,3067);w.put(4,5,197);w.put(4,3,254);
    // Preserve dynamic icon width. Reserve two 48-point bands in tile height.
    w.op3(108,0,96);w.u8(22);w.u8(0);w.u8(3);w.u8(0);w.put(4,0,145);
    w.op3(108,5,2);w.u8(26);w.u8(0);w.u8(0);w.u8(5);w.op3(10,0,0);
    w.op2(3,3);w.put(3,0,166);w.op2(3,0);w.put(0,4,5150);w.put(0,3,19);w.op2(90,0);
    // Plain style objects are valid RN styles. No native StyleSheet IDs needed.
    if(!w.ok||w.used>78)return 0;
    std::memcpy(output,w.bytes,w.used);return w.used;
}
std::size_t build_animation(const std::uint8_t* original,std::uint8_t output[266]) {
    std::size_t used=0;
    const auto copy=[&](std::size_t first,std::size_t last){const auto n=last-first;std::memcpy(output+used,original+first,n);used+=n;};
    copy(0,0x89);
    // The selected tile's translateY also uses half the full reserved height.
    constexpr std::uint8_t band[]={108,0,96,22,1,1,0};std::memcpy(output+used,band,sizeof(band));used+=sizeof(band);
    copy(0x89,0x94);
    // Return the constructed style object directly, freeing enough bytecode
    // space without altering the map/interpolation callbacks or their bindings.
    copy(0xa5,0x103);copy(0x108,266);return used;
}
std::size_t build_row(const std::uint8_t* original,std::uint8_t output[402],bool modern=false) {
    std::size_t used=0;
    const auto copy=[&](std::size_t first,std::size_t last){const auto n=last-first;std::memcpy(output+used,original+first,n);used+=n;};
    copy(0,6);
    // Equivalent __esModule export assignment, as in the working style module.
    const std::uint8_t es_module[]={118,0,57,2,0,12,static_cast<std::uint8_t>(modern?54:58),0};std::memcpy(output+used,es_module,sizeof(es_module));used+=sizeof(es_module);
    copy(0x26,0x13a);
    const std::uint8_t row_height[]={109,8,0x50,1,0,0,57,9,8,13,static_cast<std::uint8_t>(modern?143:145),0}; // 336
    std::memcpy(output+used,row_height,sizeof(row_height));used+=sizeof(row_height);
    copy(0x13a,0x167);
    const std::uint8_t strand_height[]={57,7,8,13,static_cast<std::uint8_t>(modern?143:145),0};std::memcpy(output+used,strand_height,sizeof(strand_height));used+=sizeof(strand_height);
    copy(0x167,402);return used;
}
std::size_t build_game_title(const std::uint8_t* original,std::uint8_t* output,std::size_t size=361) {
    // The original TITLE_Y equals the 106-point icon height. The outside
    // card now occupies 202 points. Move the complete native title/platform
    // line below it, leaving a 12-point gap. Both the exported TITLE_Y used
    // by transitions and experienceTitleContainer.top read the same value.
    // No branches cross the replacement; all remaining instructions stay exact.
    constexpr std::size_t first=0x7f,last=0x85;
    std::memcpy(output,original,first);
    constexpr std::uint8_t title_y[]={108,5,214};
    std::memcpy(output+first,title_y,sizeof(title_y));
    std::memcpy(output+first+sizeof(title_y),original+last,size-last);
    return first+sizeof(title_y)+size-last;
}
bool contains(const std::uint8_t* data,std::size_t size,const char* text) {
    const auto n=std::strlen(text);if(size<n)return false;
    for(std::size_t i=0;i<=size-n;++i)if(data[i]==static_cast<unsigned char>(text[0])&&!std::memcmp(data+i,text,n))return true;
    return false;
}
}
void sha1_digest(const void* value,std::size_t size,std::uint8_t output[20]) {
    const auto* data=static_cast<const std::uint8_t*>(value);
    std::uint32_t state[5]={0x67452301,0xefcdab89,0x98badcfe,0x10325476,0xc3d2e1f0};
    const auto total=size;
    while(size>=64){sha1_block(state,data);data+=64;size-=64;}
    std::uint8_t tail[128]{};if(size)std::memcpy(tail,data,size);tail[size]=0x80;
    const unsigned blocks=size<56?1:2;const auto bits=static_cast<std::uint64_t>(total)*8;
    for(unsigned i=0;i<8;++i)tail[blocks*64-1-i]=static_cast<std::uint8_t>(bits>>(8*i));
    for(unsigned i=0;i<blocks;++i)sha1_block(state,tail+i*64);
    for(unsigned i=0;i<20;++i)output[i]=static_cast<std::uint8_t>(state[i/4]>>(24-(i%4)*8));
}
static std::size_t build_title_style_profile(std::uint8_t* output,std::size_t capacity,const Config& config,bool modern) {
    if(!output||config.color>0xffffff)return 0;
    Writer w(modern);
    // module(exports, require): retain image geometry, telemetry container,
    // native font scale, bold/centred text and StyleSheet.create().
    if(!config.outside) {
    w.op3(106,2,2);w.op3(106,5,3);w.op2(116,0);w.op2(118,1);w.cached_put(2,1,1,58);
    w.op3(108,1,1);w.call(3,5,0,1);w.op3(108,1,25);w.call(5,5,0,1);
    w.get(6,5,1,5305);w.op2(3,8);w.put(8,6,9034);
    w.get(6,5,2,5800);w.op2(3,7);w.put(7,6,254);w.put(7,6,145);
    w.get(1,3,3,27);w.get(1,1,4,4494);
    // Verified, unmodified literal: {fontSize:null,fontWeight:'bold',textAlign:'center'}.
    w.u8(1);w.u8(6);w.u16(3);w.u16(3);w.u16(modern?897:895);w.u16(modern?1488:1486);w.cached_put(6,1,3,59);
    w.string(1,3067);w.put(6,1,197);w.op2(120,1);
    w.put(6,1,config.top?240:4859);w.put(6,1,158);w.put(6,1,8486);
    // React Native receives an unsigned RGBA number. A double is required:
    // LoadConstInt would turn e.g. white into a negative, invalid colour.
    w.number(1,static_cast<double>((config.color<<8)|0xffu));w.put(6,1,4583);
    w.op2(3,5);w.string(1,4220);w.put(5,1,197);
    w.get(4,3,5,47);w.get(3,4,6,107);w.op2(3,1);
    w.put(1,8,10650);w.put(1,7,5015);w.put(1,6,237);w.put(1,5,6883);
    if(modern){w.op2(3,9);w.string(6,3252);w.put(9,6,7810);w.put(1,9,6376);}
    w.call(1,3,4,1);w.cached_put(2,1,2,110);w.op2(90,0);
    } else {
    w.op3(106,2,2);w.op3(106,5,3);w.op2(116,0);w.op2(118,1);w.cached_put(2,1,1,58);
    w.op3(108,1,1);w.call(3,5,0,1);
    w.u8(1);w.u8(8);w.u16(2);w.u16(2);w.u16(120);w.u16(222);
    w.op3(108,6,202);w.cached_put(8,6,4,145);
    w.u8(1);w.u8(7);w.u16(2);w.u16(2);w.u16(120);w.u16(222);
    w.op3(108,6,16);w.put(7,6,9034);
    w.get(1,3,3,27);w.get(1,1,4,4494);
    w.u8(1);w.u8(6);w.u16(3);w.u16(3);w.u16(modern?897:895);w.u16(modern?1488:1486);w.cached_put(6,1,3,59);
    w.string(1,3067);w.put(6,1,197);w.op2(120,1);
    // Auto-height text uses the exact working 0.2.3 anchoring, now inside the
    // reserved band rather than at a negative coordinate or outside a mask.
    w.put(6,1,config.top?240:4859);w.put(6,1,158);w.put(6,1,8486);
    w.number(1,static_cast<double>((config.color<<8)|0xffu));w.put(6,1,4583);
    w.op2(3,5);w.string(1,4220);w.put(5,1,197);
    w.get(4,3,5,47);w.get(3,4,6,107);w.op2(3,1);
    w.put(1,8,10650);w.put(1,7,5015);w.put(1,6,237);w.put(1,5,6883);
    if(modern){w.op2(3,9);w.put(1,9,6376);}
    w.call(1,3,4,1);w.cached_put(2,1,2,110);w.op2(90,0);
    }
    if(!w.ok||w.used>capacity)return 0;
    std::memcpy(output,w.bytes,w.used);return w.used;
}
std::size_t build_title_style(std::uint8_t* output,std::size_t capacity,const Config& config) {
    return build_title_style_profile(output,capacity,config,false);
}
HomeStyleResult patch_home_style(void* value,std::size_t size,const Config& config) {
    if(!value||size<64||size>32*1024*1024||config.color>0xffffff)return HomeStyleResult::ignored;
    auto* bundle=static_cast<std::uint8_t*>(value);
    if(std::memcmp(bundle,"RNPSHEDR",8)||read32(bundle+8)!=2)return HomeStyleResult::ignored;
    const auto offset=read32(bundle+0x1c),region=read32(bundle+0x20),plain=read32(bundle+0x24),end=read32(bundle+0x28);
    if(offset<64||region<128||plain<128||plain>region||offset>size||region>size-offset||end!=offset+region)return HomeStyleResult::ignored;
    auto* hbc=bundle+offset;
    constexpr std::uint8_t magic[]={0xc6,0x1f,0xbc,0x03,0xc1,0x03,0x19,0x1f};
    if(std::memcmp(hbc,magic,8))return HomeStyleResult::ignored;
    const bool modern=!std::memcmp(hbc+12,source_hash1360,20);
    if(!modern&&std::memcmp(hbc+12,source_hash,20))
        return contains(hbc,plain,"isHomeTitleIdDisplayEnabled")?HomeStyleResult::unsupported:HomeStyleResult::ignored;
    const std::size_t hbc_size=modern?1795460:1780936;
    const auto* expected_hash=modern?bytecode_hash1360:bytecode_hash;
    const auto* functions=modern?functions1360:functions1160;
    if(read32(hbc+8)!=89||plain!=hbc_size||read32(hbc+32)!=hbc_size||read32(hbc+40)!=(modern?9727u:9643u)||
       std::memcmp(hbc+hbc_size-20,expected_hash,20))return HomeStyleResult::invalid;
    std::uint8_t digest[20];sha1_digest(hbc,hbc_size-20,digest);
    if(std::memcmp(digest,expected_hash,20))return HomeStyleResult::invalid;
    for(unsigned i=0;i<7;++i) {
        const auto& f=functions[i];
        if(std::memcmp(hbc+128+16*f.number,f.header,16))return HomeStyleResult::invalid;
        sha1_digest(hbc+f.offset,f.size,digest);if(std::memcmp(digest,f.hash,20))return HomeStyleResult::invalid;
    }
    std::uint8_t style[199]{},tile[622]{},geometry[78]{},animation[266]{},row[402]{},game_title[376]{};
    const auto style_count=build_title_style_profile(style,sizeof(style),config,modern);
    if(!style_count||style_count>functions[0].size)return HomeStyleResult::invalid;
    std::size_t tile_count=0,geometry_count=0,animation_count=0,row_count=0,game_title_count=0;
    if(config.outside) {
        tile_count=build_outer_tile(hbc+functions[1].offset,tile,modern);
        geometry_count=build_geometry(geometry,modern);
        animation_count=build_animation(hbc+functions[3].offset,animation);
        row_count=build_row(hbc+functions[4].offset,row,modern);
        game_title_count=build_game_title(hbc+functions[6].offset,game_title,functions[6].size);
        if(!tile_count||tile_count>622||!geometry_count||geometry_count>78||!animation_count||animation_count>266||!row_count||row_count>402||!game_title_count||game_title_count>functions[6].size)return HomeStyleResult::invalid;
    }
    const auto replace=[&](unsigned index,const std::uint8_t* code,std::size_t count) {
        const auto& f=functions[index];std::memcpy(hbc+f.offset,code,f.size);
        auto* header=hbc+128+16*f.number;
        write32(header+4,(read32(header+4)&~0x7fffu)|static_cast<std::uint32_t>(count));
    };
    replace(0,style,style_count);hbc[128+16*functions[0].number+14]=config.outside?4:3;
    if(config.outside) {
        replace(1,tile,tile_count);replace(2,geometry,geometry_count);
        replace(3,animation,animation_count);replace(4,row,row_count);hbc[128+16*282+14]=13;
        // Startup animation styles follow base styles in Sony's array. Keep
        // their transforms/opacity, with reserved row geometry taking priority.
        hbc[functions[5].offset+0x54e]=1;hbc[functions[5].offset+0x55c]=0;
        replace(6,game_title,game_title_count);
    }
    sha1_digest(hbc,hbc_size-20,hbc+hbc_size-20);
    return HomeStyleResult::applied;
}
}
