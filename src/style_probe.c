// App Title Style Probe, Copyright (C) 2026 porhe911
// SPDX-License-Identifier: GPL-3.0-or-later
// Clean implementation using the public /dev/rnps ioctl ABI documented in
// zecoxao/decrypt_rnps, commit 5ad3e75156476464e6ff00b12db54c93566a99dd.
#include "style_probe.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PROBE_VERSION "0.2.14"
#define MAX_IMAGE (32u*1024u*1024u)
static const unsigned char hbc_magic[8]={0xc6,0x1f,0xbc,0x03,0xc1,0x03,0x19,0x1f};

static uint32_t u32(const unsigned char* p) {
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static uint64_t fingerprint(const unsigned char* data,size_t size) {
    uint64_t value=UINT64_C(14695981039346656037);
    for(size_t i=0;i<size;++i){value^=data[i];value*=UINT64_C(1099511628211);}
    return value;
}
static int atomic_copy(const char* path,const void* data,size_t size) {
    char temp[1024];
    const int n=snprintf(temp,sizeof(temp),"%s.%d.tmp",path,(int)getpid());
    if(n<=0||(size_t)n>=sizeof(temp)){errno=ENAMETOOLONG;return -1;}
    const int fd=open(temp,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);
    if(fd<0)return -1;
    size_t used=0;int error=0;
    while(used<size) {
        const ssize_t count=write(fd,(const char*)data+used,size-used);
        if(count<0&&errno==EINTR)continue;
        if(count<=0){error=count<0?errno:EIO;break;}
        used+=(size_t)count;
    }
    if(!error&&fsync(fd)<0)error=errno;
    if(close(fd)<0&&!error)error=errno;
    if(!error&&rename(temp,path)<0)error=errno;
    if(error){unlink(temp);errno=error;return -1;}
    return 0;
}
typedef struct {
    const char* stage;
    int success,error;
    uint32_t fw;
    size_t size;
    uint64_t before,after;
    char header_magic[17];
    uint32_t header_version,region_offset,region_size,region_end;
    int64_t trailing_size;
    int64_t hbc_offset;
    uint32_t hbc_version;
    uint32_t ram_modules,ram_startup_size;
    decrypt_result device;
} result;
static int publish(const char* path,const result* r) {
    char text[1024];
    const int n=snprintf(text,sizeof(text),
        "App Title v%s diagnostic\npid=%d fw=0x%08" PRIx32 "\n"
        "stage=%s success=%d errno=%d\n"
        "image_size=%zu input_fnv1a64=%016" PRIx64 " output_fnv1a64=%016" PRIx64 "\n"
        "header_magic=%s version=%" PRIu32 "\n"
        "region_offset=%" PRIu32 " region_size=%" PRIu32 " region_end=%" PRIu32 " trailing_size=%" PRId64 "\n"
        "ioctl_rc=%d ioctl_errno=%d device_error=0x%08" PRIx32 "\n"
        "hermes_offset=%" PRId64 " hermes_version=%" PRIu32 "\n"
        "ram_modules=%" PRIu32 " ram_startup_size=%" PRIu32 "\n"
        "Scope=NPXS40002 only; source opened read-only; no process hooks or system-file writes.\n",
        PROBE_VERSION,(int)getpid(),r->fw,r->stage,r->success,r->error,
        r->size,r->before,r->after,r->header_magic,r->header_version,
        r->region_offset,r->region_size,r->region_end,r->trailing_size,
        r->device.ioctl_rc,r->device.saved_errno,
        r->device.device_error,r->hbc_offset,r->hbc_version,r->ram_modules,r->ram_startup_size);
    return n>0&&(size_t)n<sizeof(text)?atomic_copy(path,text,(size_t)n):-1;
}
int style_probe(const char* input,const char* output,const char* report,
                uint32_t firmware,decrypt_fn decrypt) {
    result r={.stage="main.enter",.fw=firmware,.hbc_offset=-1,
              .device={.ioctl_rc=-777,.device_error=0x800f1225u}};
    unsigned char* data=NULL;int fd=-1;
    if(!input||!output||!report||!decrypt||!strcmp(input,output)||
       !strcmp(input,report)||!strcmp(output,report))return 1;
    // At most four bounded snapshots in a single invocation. No worker loop.
    if(publish(report,&r)<0)return 1;
    r.stage="input.open";
    fd=open(input,O_RDONLY|O_NOFOLLOW);
    if(fd<0){r.error=errno;goto done;}
    struct stat st;
    r.stage="input.stat";
    if(fstat(fd,&st)<0){r.error=errno;goto done;}
    if(!S_ISREG(st.st_mode)||st.st_size<64||(uint64_t)st.st_size>MAX_IMAGE) {
        r.stage="input.size_rejected";r.error=EINVAL;goto done;
    }
    r.size=(size_t)st.st_size;
    r.stage="input.allocate";
    const int alloc=posix_memalign((void**)&data,0x4000,r.size);
    if(alloc){r.error=alloc;goto done;}
    r.stage="input.read";
    size_t used=0;
    while(used<r.size) {
        const ssize_t n=read(fd,data+used,r.size-used);
        if(n<0&&errno==EINTR)continue;
        if(n<=0){r.error=n<0?errno:EIO;goto done;}
        used+=(size_t)n;
    }
    // Refuse a racing/incomplete source rather than passing it to a device.
    unsigned char tail;
    ssize_t extra;
    do {extra=read(fd,&tail,1);}while(extra<0&&errno==EINTR);
    if(extra!=0){r.stage="input.size_changed";r.error=extra<0?errno:EINVAL;goto done;}
    if(close(fd)<0){fd=-1;r.error=errno;goto done;}fd=-1;
    r.before=fingerprint(data,r.size);
    for(unsigned i=0;i<8;++i)snprintf(r.header_magic+i*2,3,"%02x",data[i]);
    r.header_version=u32(data+8);r.region_offset=u32(data+0x1c);
    r.region_size=u32(data+0x20);r.region_end=u32(data+0x28);
    r.trailing_size=(int64_t)r.size-(int64_t)r.region_end;
    // +0x28 ends the region, not the complete file. The real 11.60 input
    // has another 384 bytes after it. Validate ranges in 64-bit arithmetic
    // while allowing that tail; never trim the buffer passed to the driver.
    if(memcmp(data,"RNPSHEDR",8)||r.header_version!=2||
       r.region_offset<64||r.region_size==0||
       (uint64_t)r.region_offset+(uint64_t)r.region_size!=r.region_end||
       (uint64_t)r.region_end>r.size) {
        r.stage="input.header_rejected";r.error=EINVAL;goto done;
    }
    r.stage="rnps.decrypt.begin";
    if(publish(report,&r)<0){r.error=errno;goto done;}
    r.device=decrypt(data,(int)r.size);
    r.after=fingerprint(data,r.size);
    if(r.device.ioctl_rc<0||r.device.device_error!=0) {
        r.stage="rnps.decrypt.failed";r.error=r.device.saved_errno;goto done;
    }
    if(r.before==r.after){r.stage="rnps.unchanged_data";r.error=EIO;goto done;}
    for(size_t i=0;i+12<=r.size;++i)if(!memcmp(data+i,hbc_magic,8)) {
        r.hbc_offset=(int64_t)i;r.hbc_version=u32(data+i+8);break;
    }
    if(r.region_size>=12&&u32(data+r.region_offset)==0xfb0bd1e5u) {
        const uint32_t count=u32(data+r.region_offset+4),startup=u32(data+r.region_offset+8);
        if(count>0&&12ull+8ull*count+startup<=r.region_size) {
            r.ram_modules=count;r.ram_startup_size=startup;
        }
    }
    r.stage="output.write";
    if(atomic_copy(output,data,r.size)<0){r.error=errno;goto done;}
    r.stage=r.ram_modules?"saved.ram_bundle_detected":r.hbc_offset>=0?"saved.hermes_detected":"saved.format_pending_analysis";
    r.success=1;
done:
    if(fd>=0)close(fd);
    free(data);
    if(publish(report,&r)<0)return 1;
    return r.success?0:1;
}

