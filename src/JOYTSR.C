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

int JOY_get_tsc(void);

typedef struct 
{
  const char * prog_name;
  const char * prog_ver_s;
  const int prog_ver;
  const char * author;
  const int proc_offset;
  const int flag_offset;
  const int buttons_offset;
  const int axes_offset;
  const int time_offset;
  const int timer_mask;
} version_info_s;

static version_info_s supported_versions[] =
{
  { "KEYS2JOY", " 0.03", 3, "  Bret Johnson", 0x506, 0, 0, 0, 0, 0 },
  { "USBJSTIK", " 0.11", 0x11,"  Bret Johnson", 0x1AAC, 0x125, 0x133, 0x134, 0x194, 0x4} 
};

#define NUM_VERSIONS (sizeof(supported_versions) / sizeof (supported_versions[0]))
#define MK_LINEAR(s, a) (((s) << 4) + (a))
#define DOS_ADDR(r) MK_LINEAR(r.x.es, r.x.di)
#define IS_JUNK_PTR(r) (r.x.es == 0)


#define PORT_WRITE 0x4
#define PORT_READ  0

typedef struct 
{
  int handle; 
  int found;
  int seg;
  int proc_offs;
  int flag_offs;
  int data_offs;
  unsigned char *flag;
  unsigned short *axes; 
  unsigned char *buttons;
  unsigned short *time;
  unsigned char mask;
  int reads;
  int writes;
  unsigned stamp;
  unsigned char state;
  int factor;
} tsr_info_s;

static tsr_info_s tsr_info;

static void print_version(version_info_s * ver)
{
  printf("%s %s %s", ver->prog_name, ver->prog_ver_s, ver->author);
}

void JOY_FindTSR(int verbosity, int factor)
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
      if(!strncmp(NearPtr(DOS_ADDR(r)), supported_versions[i].prog_name,
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
      if(!strncmp(NearPtr(DOS_ADDR(r)), supported_versions[i].author,
         strlen(supported_versions[i].author))) score[i]++;
    }

    for(i = 0 ; i < NUM_VERSIONS ; i++) {
      if(i == found) continue;
      if(score[i] == 3 && found < 0) {
        found = i;
        memset(&tsr_info, 0, sizeof(tsr_info));
        tsr_info.found = i;
        tsr_info.seg = r.x.es;
        tsr_info.proc_offs = supported_versions[i].proc_offset;
        if(supported_versions[i].flag_offset)
        {
          tsr_info.flag = (unsigned char *) NearPtr(
                            MK_LINEAR(tsr_info.seg, supported_versions[i].flag_offset));
          tsr_info.time = (unsigned short *) NearPtr(
                            MK_LINEAR(tsr_info.seg, supported_versions[i].time_offset));
          tsr_info.buttons = (unsigned char *) NearPtr(
                            MK_LINEAR(tsr_info.seg, supported_versions[i].buttons_offset));
          tsr_info.axes = (unsigned short *) NearPtr(
                            MK_LINEAR(tsr_info.seg, supported_versions[i].axes_offset));
          tsr_info.mask = supported_versions[i].timer_mask; 
        } 
        tsr_info.handle = handle;
        tsr_info.factor = factor;
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
    void * code = NearPtr(MK_LINEAR(tsr_info.seg, tsr_info.proc_offs));
    if(((uint8_t *)code)[0] == 0xe8) memset(code, 0x90, 5);
    for(i = 0; verbosity && i < 0x100 ; i ++ )
    {
      if(i%16==0) printf("%08x", i);
      if(i%8==0) printf(" ");
      printf("%02x", ((uint8_t *) code)[i]);
      if((i+1)%16==0) printf("\n");
    }

    if(verbosity && tsr_info.mask)
    {
      printf("data: %02x %02x %04x %04x %04x %04x %04x\n",
             *tsr_info.flag, *tsr_info.buttons, 
             tsr_info.axes[0], tsr_info.axes[1],
             tsr_info.axes[2], tsr_info.axes[3],
             *tsr_info.time);
    }
   
  }
  
}

static int JOY_Call_TSR(uint16_t val, uint16_t port, uint16_t dir)
{
  __dpmi_regs r;

  memset(&r, 0, sizeof(r));
  r.x.ax = val;
  r.x.dx = port;
  r.x.cx = dir;
  r.x.cs = r.x.ds = tsr_info.seg;
  r.x.ip = tsr_info.proc_offs;
  if(__dpmi_simulate_real_mode_procedure_retf(&r) == 0) {
    if(!(r.x.flags&CPU_CFLAG)) return r.x.ax;
  }

  return 0xff;
}

static int JOY_Handle_Read()
{
  unsigned now = 0, then = tsr_info.stamp, diff;
  int i = 0;

  diff = JOY_get_tsc() - then;

  if(tsr_info.reads != tsr_info.writes)
  {
    tsr_info.state = 0xf;
    tsr_info.reads++;
  } 
  
  for(i = 0 ; i < 4 ; i += 1)
  {
    unsigned val;
    if(!(tsr_info.state & (1 << i))) continue;
    val = tsr_info.axes[i];
    val <<= tsr_info.factor + 4;
    if(diff >= val) tsr_info.state &= ~(1 << i);
  }  

  return tsr_info.state | *tsr_info.buttons;
}

static int JOY_Handle_Write()
{
  unsigned now;

  now = JOY_get_tsc();

  tsr_info.stamp = now;
  tsr_info.writes ++;  

  return 0;
}


uint8_t JOY_Port_Acc(uint16_t port, uint8_t val, uint16_t flags)
{
  if(tsr_info.found >= 0)
  {
    int dir = (flags & TRAPF_OUT) ? PORT_WRITE : PORT_READ;
    if(!tsr_info.mask)
      return JOY_Call_TSR(val, port, dir);
    return dir == PORT_READ ? JOY_Handle_Read() : JOY_Handle_Write();
  }
  
  return 0xcc;
}


