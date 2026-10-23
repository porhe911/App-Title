// SPDX-License-Identifier: GPL-3.0-or-later
#include "style_probe.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int calls=0,mode=0,expected_size=4096;
static const unsigned char magic[]={0xc6,0x1f,0xbc,0x03,0xc1,0x03,0x19,0x1f};
static void put32(unsigned char* p,unsigned v) {for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(v>>(i*8));}
static decrypt_result mock_decrypt(void* bytes,int size) {
    ++calls;assert(size==expected_size);assert(((size_t)bytes&0x3fff)==0);
    if(mode==1)return (decrypt_result){.ioctl_rc=-1,.saved_errno=EACCES,.device_error=0x800f1225u};
    if(mode==2)return (decrypt_result){.ioctl_rc=0,.device_error=0x800f1213u};
    if(mode!=3) {
        unsigned char* p=bytes;p[63]^=0x33;
        if(mode==5){put32(p+256,0xfb0bd1e5u);put32(p+260,2);put32(p+264,8);}
        else if(mode!=4){memcpy(p+128,magic,8);put32(p+136,89);}
    }
    return (decrypt_result){0};
}
static void write_input(const unsigned char* bytes,size_t size) {
    FILE* f=fopen("build-host/style/input.bundle","wb");assert(f&&fwrite(bytes,1,size,f)==size&&fclose(f)==0);
}
static void read_report(char* text,size_t size) {
    FILE* f=fopen("build-host/style/probe.log","r");assert(f);
    const size_t n=fread(text,1,size-1,f);text[n]=0;assert(fclose(f)==0&&n<1024);
}
int main(int argc,char** argv) {
    mkdir("build-host",0755);mkdir("build-host/style",0755);
    if(argc==2) {
        struct stat actual;assert(stat(argv[1],&actual)==0&&actual.st_size==1784176);
        expected_size=(int)actual.st_size;mode=1;
        assert(style_probe(argv[1],"build-host/style/actual_output.bin","build-host/style/actual_probe.log",0x11600005,mock_decrypt)==1);
        assert(calls==1&&access("build-host/style/actual_output.bin",F_OK)<0);
        FILE* f=fopen("build-host/style/actual_probe.log","r");assert(f);
        char text[1024];size_t n=fread(text,1,sizeof(text)-1,f);text[n]=0;fclose(f);
        assert(strstr(text,"rnps.decrypt.failed")&&strstr(text,"trailing_size=384"));
        puts("PASS: actual uploaded 1784176-byte RNPS image reaches the mocked device with its complete 384-byte tail; no source writes");
        return 0;
    }
    unsigned char image[4096]={0};memcpy(image,"RNPSHEDR",8);put32(image+8,2);
    put32(image+0x1c,256);put32(image+0x20,3456);put32(image+0x24,3448);put32(image+0x28,3712);
    write_input(image,sizeof(image));
    assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/probe.log",0x11600005,mock_decrypt)==0);
    assert(calls==1);char report[1024];read_report(report,sizeof(report));
    assert(strstr(report,"success=1")&&strstr(report,"hermes_offset=128 hermes_version=89")&&strstr(report,"trailing_size=384"));
    FILE* f=fopen("build-host/style/input.bundle","rb");unsigned char original[4096];
    assert(f&&fread(original,1,sizeof(original),f)==sizeof(original)&&fclose(f)==0);
    assert(!memcmp(original,image,sizeof(image))); // Real input never modified.
    struct stat out;assert(stat("build-host/style/output.bin",&out)==0&&out.st_size==4096);
    unlink("build-host/style/output.bin");
    for(mode=1;mode<=3;++mode) {
        assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/probe.log",0x11600005,mock_decrypt)==1);
        assert(access("build-host/style/output.bin",F_OK)<0);
        read_report(report,sizeof(report));assert(strstr(report,"success=0"));
        if(mode==3)assert(strstr(report,"rnps.unchanged_data"));
    }
    mode=4;
    assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/probe.log",0x11600005,mock_decrypt)==0);
    read_report(report,sizeof(report));assert(strstr(report,"saved.format_pending_analysis")&&strstr(report,"hermes_offset=-1"));
    unlink("build-host/style/output.bin");
    mode=5;
    assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/probe.log",0x04030000,mock_decrypt)==0);
    read_report(report,sizeof(report));assert(strstr(report,"saved.ram_bundle_detected")&&strstr(report,"ram_modules=2 ram_startup_size=8"));
    unlink("build-host/style/output.bin");
    const int expected_calls=calls;
    for(unsigned field=0;field<3;++field) {
        unsigned char bad[4096];memcpy(bad,image,sizeof(bad));
        if(field==0)bad[0]^=1;else if(field==1)put32(bad+8,3);else put32(bad+0x28,2048);
        write_input(bad,sizeof(bad));
        assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/probe.log",0,mock_decrypt)==1);
        read_report(report,sizeof(report));assert(strstr(report,"input.header_rejected"));
    }
    write_input(image,32);
    assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/probe.log",0,mock_decrypt)==1);
    assert(calls==expected_calls);
    write_input(image,sizeof(image));
    assert(style_probe("build-host/style/input.bundle","build-host/style/input.bundle","build-host/style/probe.log",0,mock_decrypt)==1);
    assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/input.bundle",0,mock_decrypt)==1);
    assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/output.bin",0,mock_decrypt)==1);
    unlink("build-host/style/link.bundle");assert(symlink("input.bundle","build-host/style/link.bundle")==0);
    assert(style_probe("build-host/style/link.bundle","build-host/style/output.bin","build-host/style/probe.log",0,mock_decrypt)==1);
    assert(calls==expected_calls);
    unlink("build-host/style/link.bundle");
    mode=0;
    assert(style_probe("build-host/style/input.bundle","build-host/style/missing/output.bin","build-host/style/probe.log",0,mock_decrypt)==1);
    read_report(report,sizeof(report));assert(strstr(report,"output.write success=0"));
    // The exact observed header values used to be rejected by 0.1.0.
    expected_size=1784176;mode=1;
    unsigned char* real_header=calloc(1,(size_t)expected_size);assert(real_header);
    memcpy(real_header,"RNPSHEDR",8);put32(real_header+8,2);
    put32(real_header+0x1c,2848);put32(real_header+0x20,1780944);
    put32(real_header+0x24,1780936);put32(real_header+0x28,1783792);
    write_input(real_header,(size_t)expected_size);free(real_header);
    const int before_real=calls;
    assert(style_probe("build-host/style/input.bundle","build-host/style/output.bin","build-host/style/probe.log",0,mock_decrypt)==1);
    assert(calls==before_real+1);read_report(report,sizeof(report));
    assert(strstr(report,"rnps.decrypt.failed")&&strstr(report,"trailing_size=384"));
    unlink("build-host/style/probe.log");unlink("build-host/style/input.bundle");
    puts("PASS: tail-preserving RNPS bounds, observed 11.60 header regression, validation/device failures, bounded report, unchanged/source protection and output errors");
}

