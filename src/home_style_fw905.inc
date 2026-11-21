// App Title / FW 9.05 HomeUI Hermes v89 profile.
// Metadata and independently generated instructions only; no HomeUI bytes shipped.
// Deliberately exact-match: unknown HBC variants must remain untouched.
constexpr std::uint8_t source_hash905[20] = {
    0x58,0x76,0x35,0x68,0x7e,0x0a,0x19,0x0e,0x38,0x42,
    0x52,0x32,0xc3,0x90,0x92,0x88,0x8d,0xa5,0xad,0xbe};
constexpr std::uint8_t bytecode_hash905[20] = {
    0x16,0xbc,0x51,0x56,0x31,0x18,0xe0,0xc0,0xe7,0x41,
    0x7b,0x44,0x0a,0xd8,0x2d,0xbd,0x79,0x51,0x97,0xf0};
constexpr ProfileFunction functions905[] = {
    {1390,0xd6e93,183,
        {0x93,0x6e,0x0d,0x08,0xb7,0x80,0x7f,0x00,0xa4,0x3a,0x18,0x26,0x00,0x08,0x02,0x02},
        {0x42,0x04,0xe7,0x55,0x61,0xc0,0x0c,0xa3,0xec,0x03,0x6b,0x88,0xe3,0x31,0xef,0x25,0x17,0xc0,0xc4,0xeb}},
    {5610,0x136970,595,
        {0x70,0x69,0x13,0x04,0x53,0x02,0x95,0x00,0xd0,0x41,0x18,0x3a,0x04,0x18,0x00,0x02},
        {0xd7,0xe9,0xd3,0x40,0x93,0x86,0x12,0x1b,0xda,0x08,0x5e,0xc3,0x81,0xb8,0x1c,0x1c,0x50,0x3f,0xbc,0x6e}},
    {5408,0x1312cf,78,
        {0xcf,0x12,0x13,0x04,0x4e,0x80,0x7f,0x00,0xfc,0x40,0x18,0x1c,0x00,0x03,0x00,0x02},
        {0xd5,0xa6,0x71,0x70,0x3c,0xcc,0xc0,0x92,0xcb,0xd9,0x50,0xa7,0x76,0xf0,0x79,0x92,0x81,0x41,0x39,0x66}},
    {5404,0x131116,267,
        {0x16,0x11,0x13,0x04,0x0b,0x81,0x7f,0x00,0xfc,0x40,0x18,0x28,0x06,0x0d,0x00,0x02},
        {0x91,0x5a,0x6b,0x6b,0x62,0xdd,0x36,0xcf,0x14,0x1d,0x8f,0x47,0xef,0x4a,0xff,0xb7,0xea,0x3d,0x06,0xaf}},
    {253,0xbfaae,402,
        {0xae,0xfa,0x0b,0x08,0x92,0x81,0x7f,0x00,0x34,0x39,0x18,0x28,0x00,0x06,0x0b,0x02},
        {0xe2,0x38,0xea,0x3f,0xf1,0x2f,0xdc,0x6c,0x51,0x08,0xae,0x66,0x6d,0x9f,0xb0,0xc0,0xe1,0x8d,0xaa,0x8e}},
    {972,0xcd3d2,361,
        {0xd2,0xd3,0x0c,0x08,0x69,0x81,0x7f,0x00,0x20,0x3a,0x18,0x26,0x00,0x07,0x07,0x02},
        {0x73,0xe3,0x0a,0xf7,0x05,0x19,0x3e,0xc0,0x6c,0x33,0x06,0xc5,0xb6,0xa4,0x87,0x29,0x04,0xec,0x4a,0x87}}
};

std::size_t fw905_style(std::uint8_t* output, std::size_t capacity, const Config& config) {
    if (!output || config.color > 0xffffffu) return 0;
    Writer w(false);
    w.op3(106,2,2); w.op3(106,5,3);
    w.op2(116,0); w.op2(118,1); w.cached_put(2,1,1,49); // exports.__esModule=true
    w.op3(108,1,1); w.call(3,5,0,1); // import the React Native style module
    if (config.outside) {
        // Reuse the stock immutable {height:106,width:106} literal, then override
        // only the wrapper height and padding. Yoga locates artwork at y=48.
        w.u8(1); w.u8(8); w.u16(2); w.u16(2); w.u16(117); w.u16(221);
        w.op3(108,6,202); w.cached_put(8,6,4,146); // height
        w.op3(108,6,48); w.put(8,6,10339); // paddingTop
        w.u8(1); w.u8(7); w.u16(2); w.u16(2); w.u16(117); w.u16(221);
        w.op3(108,6,16); w.put(7,6,9341); // image borderRadius
        w.op3(108,1,1); w.call(1,5,0,1);
    } else {
        w.op3(108,1,23); w.call(5,5,0,1); // native geometry module
        w.get(6,5,1,5715); w.op2(3,8); w.put(8,6,9341); // BORDER_RADIUS
        w.get(6,5,2,4909); w.op2(3,7); // EXPERIENCE_SIZE
        w.put(7,6,245); w.put(7,6,146); // width,height
    }
    w.get(1,3,3,29); w.get(1,1,4,6019); // FontSizePS.Size4XSmall
    // Stock 9.05 literal: {fontSize:null,fontWeight:'bold',textAlign:'center'}.
    w.u8(1); w.u8(6); w.u16(3); w.u16(3); w.u16(857); w.u16(1374);
    w.cached_put(6,1,3,50); // fontSize
    w.string(1,2143); w.put(6,1,197); // position:absolute
    w.op2(120,1); w.put(6,1,config.top ? 239 : 7340); // top/bottom:0
    w.put(6,1,160); w.put(6,1,7655); // left:0,right:0
    // Always serialize unsigned RGBA as Double to preserve white/large values.
    w.number(1,static_cast<double>((config.color<<8)|0xffu)); w.put(6,1,4129);
    if (!config.outside) {
        w.op2(3,5); w.string(1,1192); w.put(5,1,197); // telemetryContainer relative
    }
    // Outside: omit the redundant telemetryContainer style key. RN View's
    // default is position:relative, so its children still get the same frame.
    w.get(4,3,5,56); w.get(3,4,6,105); w.op2(3,1);
    w.put(1,8,10188); w.put(1,7,4497); w.put(1,6,44);
    if (!config.outside) w.put(1,5,4798);
    w.call(1,3,4,1); w.cached_put(2,1,2,111); w.op2(90,0);
    if (!w.ok || w.used>capacity) return 0;
    std::memcpy(output,w.bytes,w.used); return w.used;
}

std::size_t fw905_tile(const std::uint8_t* original, std::uint8_t* output) {
    std::size_t used=0;
    const auto copy=[&](std::size_t first,std::size_t last) {
        const auto n=last-first; std::memcpy(output+used,original+first,n); used+=n;
    };
    const auto emit=[&](const std::uint8_t* x,std::size_t n) {
        std::memcpy(output+used,x,n); used+=n;
    };
    copy(0,0x197);
    // Remove the embedded Title ID from Animated.View. We retain its construction
    // and move it alongside ImagePS under the outer wrapper.
    copy(0x19b,0x1cd);
    const std::uint8_t outside_children[]={8,7,3,7,19,2,0,66,19,5,0,66,19,15,1};
    emit(outside_children,sizeof(outside_children));
    // Parent style is imageWrapper only: do not override height with image(106).
    copy(0x1e8,0x223);
    const std::uint8_t ordinary_children[]={61,0,19,94};
    emit(ordinary_children,sizeof(ordinary_children));
    copy(0x227,0x247);
    const std::uint8_t capture_children[]={61,1,19,94};
    emit(capture_children,sizeof(capture_children));
    copy(0x24b,595);
    // Branch endpoints do not cross edits; relative offsets are preserved.
    return used;
}

std::size_t fw905_geometry(std::uint8_t* output) {
    Writer w(false);
    w.op3(106,0,1); w.get(3,0,1,8157); // itemSize
    w.op2(3,4); w.string(5,2143); w.put(4,5,197); w.put(4,3,245);
    w.op3(108,0,96); w.u8(22); w.u8(0); w.u8(3); w.u8(0); w.put(4,0,146); // height=itemSize+96
    w.op3(108,5,2); w.u8(26); w.u8(0); w.u8(0); w.u8(5); w.op3(10,0,0);
    w.op2(3,3); w.put(3,0,170); // marginTop=-height/2
    w.op2(3,0); w.put(0,4,4609); w.put(0,3,19); w.op2(90,0);
    if (!w.ok || w.used>78) return 0;
    std::memcpy(output,w.bytes,w.used); return w.used;
}

std::size_t fw905_animation(const std::uint8_t* original, std::uint8_t* output) {
    std::size_t used=0;
    const auto copy=[&](std::size_t first,std::size_t last) {
        const auto n=last-first; std::memcpy(output+used,original+first,n); used+=n;
    };
    copy(0,0x88);
    // selected translateY=(itemMargin+96)/2 for an extra 48-point band each side
    const std::uint8_t add_band[]={108,0,96,22,1,1,0};
    std::memcpy(output+used,add_band,sizeof(add_band)); used+=sizeof(add_band);
    copy(0x88,0x93);
    // StyleSheet.create(object) is unnecessary here; React Native accepts a plain
    // style object and the helper's own return value is already the same object.
    copy(0xa4,0x104); copy(0x109,267);
    return used;
}

std::size_t fw905_row(const std::uint8_t* original, std::uint8_t* output) {
    std::size_t used=0;
    const auto copy=[&](std::size_t first,std::size_t last) {
        const auto n=last-first; std::memcpy(output+used,original+first,n); used+=n;
    };
    const auto emit=[&](const std::uint8_t* x,std::size_t n) {
        std::memcpy(output+used,x,n); used+=n;
    };
    copy(0,0x06);
    const std::uint8_t es_module[]={118,0,57,2,0,12,49,0};
    emit(es_module,sizeof(es_module));
    copy(0x26,0x135);
    const std::uint8_t row_height[]={109,10,0x50,0x01,0,0,57,9,10,13,146,0};
    emit(row_height,sizeof(row_height));
    copy(0x135,0x167);
    const std::uint8_t strand_height[]={109,10,0x50,0x01,0,0,57,7,10,13,146,0};
    emit(strand_height,sizeof(strand_height));
    copy(0x167,402);
    return used;
}

std::size_t fw905_title(const std::uint8_t* original, std::uint8_t* output) {
    std::memcpy(output,original,0x7f);
    const std::uint8_t y[]={108,5,214};
    std::memcpy(output+0x7f,y,sizeof(y));
    std::memcpy(output+0x7f+sizeof(y),original+0x85,361-0x85);
    return 0x7f+sizeof(y)+361-0x85;
}

HomeStyleResult patch_home_style_905(std::uint8_t* hbc,std::size_t plain,const Config& config) {
    constexpr std::size_t expected_size=1591008;
    if (!hbc || plain!=expected_size || config.color>0xffffffu ||
        read32(hbc+8)!=89 || read32(hbc+32)!=expected_size ||
        read32(hbc+40)!=8261 || read32(hbc+52)!=10727 ||
        std::memcmp(hbc+12,source_hash905,20) ||
        std::memcmp(hbc+expected_size-20,bytecode_hash905,20))
        return HomeStyleResult::invalid;
    std::uint8_t digest[20];
    sha1_digest(hbc,expected_size-20,digest);
    if (std::memcmp(digest,bytecode_hash905,20)) return HomeStyleResult::invalid;
    for (const auto& f: functions905) {
        if (std::memcmp(hbc+128+16*f.number,f.header,16)) return HomeStyleResult::invalid;
        sha1_digest(hbc+f.offset,f.size,digest);
        if (std::memcmp(digest,f.hash,20)) return HomeStyleResult::invalid;
    }
    // Stage all rewritten functions first. Never partially write the live HBC.
    std::uint8_t style[183]{},tile[595]{},geometry[78]{},animation[267]{},row[402]{},title[361]{};
    const std::size_t a=fw905_style(style,sizeof(style),config);
    if (!a || a>183) return HomeStyleResult::invalid;
    std::size_t b=0,c=0,d=0,e=0,f=0;
    if (config.outside) {
        b=fw905_tile(hbc+functions905[1].offset,tile);
        c=fw905_geometry(geometry);
        d=fw905_animation(hbc+functions905[3].offset,animation);
        e=fw905_row(hbc+functions905[4].offset,row);
        f=fw905_title(hbc+functions905[5].offset,title);
        if (b!=579 || c!=63 || d!=252 || e!=402 || f!=358)
            return HomeStyleResult::invalid;
    }
    const auto replace=[&](unsigned index,const std::uint8_t* code,std::size_t count) {
        const auto& info=functions905[index];
        std::memcpy(hbc+info.offset,code,count);
        std::memset(hbc+info.offset+count,0,info.size-count);
        auto* header=hbc+128+16*info.number;
        write32(header+4,(read32(header+4)&~0x7fffu)|static_cast<std::uint32_t>(count));
    };
    replace(0,style,a);
    hbc[128+16*functions905[0].number+14]=config.outside?4:3;
    if (config.outside) {
        replace(1,tile,b); replace(2,geometry,c); replace(3,animation,d);
        replace(4,row,e); hbc[128+16*functions905[4].number+14]=13;
        replace(5,title,f);
    }
    sha1_digest(hbc,expected_size-20,hbc+expected_size-20);
    return HomeStyleResult::applied;
}
