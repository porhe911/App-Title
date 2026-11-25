// SPDX-License-Identifier: GPL-3.0-or-later
// One-shot, no hooks, no settings changes. Run only for a requested diagnostic.
#include "style_probe.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ps5/kernel.h>

struct args { void *data; int size; uint32_t error; };
_Static_assert(sizeof(struct args)==16,"RNPS ABI");
static decrypt_result decrypt_once(void *data,int size) {
    decrypt_result r={-1,0,0x800f1225u};
    const int fd=open("/dev/rnps",O_RDWR);
    if(fd<0){r.saved_errno=errno;return r;}
    struct args a={data,size,0x800f1225u};
    r.ioctl_rc=ioctl(fd,0xc0105203UL,&a);
    r.saved_errno=r.ioctl_rc<0?errno:0;
    r.device_error=a.error;
    close(fd);
    return r;
}
int main(void) {
    const uint32_t fw=kernel_get_fw_version();
    printf("App Title one-shot diagnostic firmware 0x%08x\n",fw);
    if((fw>>16)!=0x1300u){puts("Not PS5 FW13.00; no export performed.");return 2;}
    if(mkdir("/data/AppTitle",0755)<0&&errno!=EEXIST)return 3;
    if(mkdir("/data/AppTitle/FW1300_probe",0755)<0&&errno!=EEXIST)return 4;
    const char* input="/system_ex/rnps/apps/NPXS40002/application.ps.bundle";
    if(access(input,F_OK)<0&&errno==ENOENT)input="/mnt/rnps/apps/NPXS40002/application.ps.bundle";
    const int rc=style_probe(input,"/data/AppTitle/FW1300_probe/HomeUI_FW1300.bin",
         "/data/AppTitle/FW1300_probe/Probe_FW1300.log",fw,decrypt_once);
    printf("App Title FW13.00 diagnostic %s. See /data/AppTitle/FW1300_probe/\n",rc==0?"saved":"failed");
    return rc;
}
