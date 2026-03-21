#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

#include "CONFIG.H"
#include "PLATFORM.H"
#include "PTRAP.H"
#include "LINEAR.H"

typedef struct 
{
  const char * prog_name;
  const char * prog_ver_s;
  const int prog_ver;
  const char * author;
  const int known_offset;
} version_info_s;

static version_info_s supported_versions[] =
{
  { "KEYS2JOY", " 0.03", 3, "  Bret Johnson", 0x506 },
  { "USBJSTIK", " 0.11", 0x11,"  Bret Johnson", 0x1AAC } 
};

#define NUM_VERSIONS (sizeof(supported_versions) / sizeof (supported_versions[0]))
#define DOS_STR_PTR(r) (char *)((void *)((r.x.es << 4) + r.x.di))
#define IS_JUNK_PTR(r) (r.x.es == 0)


#define PORT_WRITE 0x4
#define PORT_READ  0

typedef int (* virt_io_f)(uint16_t byte, uint16_t port, uint16_t dummy, uint16_t op);

typedef struct 
{
  int handle; 
  int found;
  int seg;
  int offs;
  virt_io_f fun; 
} tsr_info_s;

static tsr_info_s tsr_info;

static void print_version(version_info_s * ver)
{
  printf("%s %s %s", ver->prog_name, ver->prog_ver_s, ver->author);
}

void JOY_FindTSR()
{
  int handle = 0xc0;
  int score[NUM_VERSIONS];
  int found = -1;

  memset(&tsr_info, 0, sizeof(tsr_info));
  memset(score, 0, sizeof(score));
  for( handle = 0xc0 ; handle <= 0xff ; handle++) {
    __dpmi_regs r;
    const char * str;
    int i;

    memset(&r, 0, sizeof(r));
    r.x.ax = (handle << 8);
    __dpmi_simulate_real_mode_interrupt(0x2f, &r);
    if(r.x.ax & 0xff != 0xff) continue;

    memset(&r, 0, sizeof(r));
    r.x.ax = (handle << 8) | 1;
    __dpmi_simulate_real_mode_interrupt(0x2f, &r);
 
    if(IS_JUNK_PTR(r)) continue;   
    for(i = 0 ; i < NUM_VERSIONS ; i++) {
      if(!strncmp(DOS_STR_PTR(r), supported_versions[i].prog_name,
          strlen(supported_versions[i].prog_name))) score[i]++;
    }
    
    memset(&r, 0, sizeof(r));
    r.x.ax = (handle << 8) | 2;
    __dpmi_simulate_real_mode_interrupt(0x2f, &r);
    for(i = 0 ; i < NUM_VERSIONS ; i++) {
      if(supported_versions[i].prog_ver == r.x.ax) score[i]++;
    }

    memset(&r, 0, sizeof(r));
    r.x.ax = (handle << 8) | 3;
    __dpmi_simulate_real_mode_interrupt(0x2f, &r);
    if(IS_JUNK_PTR(r)) continue;   
    for(i = 0 ; i < NUM_VERSIONS ; i++) {
      if(!strncmp(DOS_STR_PTR(r), supported_versions[i].author,
         strlen(supported_versions[i].author))) score[i]++;
    }

    for(i = 0 ; i < NUM_VERSIONS ; i++) {
      if(i == found) continue;
      if(score[i] == 3 && found < 0) {
        found = i;
        tsr_info.found = i;
        tsr_info.seg = r.x.es;
        tsr_info.offs = supported_versions[i].known_offset;
        tsr_info.handle = handle;
      }
      else if(score[i] == 3) {
        printf("Found at least two compatible TRS versions:\n");
        print_version(&supported_versions[found]);
        printf("\n");
        print_version(&supported_versions[i]);
        printf("\n");
        printf("Aborting...\nTry running without /J to proceed.\n");
        exit(1);
      }
    }
  }

  if(found < 0) {
    printf("Found no compatible joystick TSR. Aborting...\n");
    printf("Try running without /J to proceed.\n");
    exit(1);
  }
  
  printf("Found compatible joystick TSR: ");
  print_version(&supported_versions[found]);
  printf("\n");

  {
    int i;
    void * code = (void *)((tsr_info.seg << 4) + tsr_info.offs);
    if(((uint8_t *)code)[0] == 0xe8) memset(code, 0x90, 5);
    for(i = 0; i < 0x100 ; i ++ )
    {
      if(i%16==0) printf("%08x", i);
      if(i%8==0) printf(" ");
      printf("%02x", ((uint8_t *) code)[i]);
      if((i+1)%16==0) printf("\n");
    }
   
    tsr_info.fun = (virt_io_f) code;
  }

}

static int JOY_Call_TSR(uint16_t val, uint16_t port, uint16_t dir)
{
  //ah if it only were so easy...
  //return tsr_info.fun(port, val, 0, dir);
  __dpmi_regs r;

  memset(&r, 0, sizeof(r));
  r.x.ax = val;
  r.x.dx = port;
  r.x.cx = dir;
  r.x.cs = r.x.ds = tsr_info.seg;
  r.x.ip = tsr_info.offs;
  if(__dpmi_simulate_real_mode_procedure_retf(&r) == 0) {
    if(!(r.x.flags&CPU_CFLAG)) return r.x.ax;
  }

  return 0xff;
}


uint8_t JOY_Port_Acc(uint16_t port, uint8_t val, uint16_t flags)
{
  if(tsr_info.found >= 0)
  {
    return JOY_Call_TSR(val, port, (flags & TRAPF_OUT) ? PORT_WRITE : PORT_READ);
  }
  
  return 0xcc;
}

