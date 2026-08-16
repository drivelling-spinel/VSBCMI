//**************************************************************************
//*                     This file is part of the                           *
//*                      Mpxplay - audio player.                           *
//*                  The source code of Mpxplay is                         *
//*        (C) copyright 1998-2008 by PDSoft (Attila Padar)                *
//*                http://mpxplay.sourceforge.net                          *
//*                  email: mpxplay@freemail.hu                            *
//**************************************************************************
//*  This program is distributed in the hope that it will be useful,       *
//*  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
//*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
//*  Please contact with the author (with me) if you want to use           *
//*  or modify this source.                                                *
//**************************************************************************
//function: CMI 8338/8738 (PCI) low level routines
//based on the ALSA (http://www.alsa-project.org)

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#ifndef DJGPP
#include <conio.h>
#endif
#include <string.h>

#define _CMI_LOG(...) dbgprintf((__VA_ARGS__))
#define mpxplay_debugf(s, ...) dbgprintf((__VA_ARGS__))

//ref: CMI-8738/PCI-SX AUDIO Specification

#define CMI_DEBUG_OUTPUT stdout

#include "CONFIG.H"
#include "MPXPLAY.H"
#include "DMABUFF.H"
#include "PCIBIOS.H"
#include "AC97MIX.H"

#define CMI8X38_CARD "CMI 8338/8738"

static char cmi8x38_shortname[] = CMI8X38_CARD " (000)  ";

#define CMI8X38_LINK_MULTICHAN 1

#if 1 //def SBEMU
#define PCMBUFFERPAGESIZE      512
#else
#define PCMBUFFERPAGESIZE      4096
#endif
#define PCMBUFFERALIGNMENT     4096

/*
 * CM8x38 registers definition
 */

#define CM_REG_FUNCTRL0        0x00
#define CM_RST_CH1        0x00080000
#define CM_RST_CH0        0x00040000
#define CM_CHEN1        0x00020000    /* ch1: enable */
#define CM_CHEN0        0x00010000    /* ch0: enable */
#define CM_PAUSE1        0x00000008    /* ch1: pause */
#define CM_PAUSE0        0x00000004    /* ch0: pause */
#define CM_CHADC1        0x00000002    /* ch1, 0:playback, 1:record */
#define CM_CHADC0        0x00000001    /* ch0, 0:playback, 1:record */

#define CM_REG_FUNCTRL1        0x04
#define CM_ASFC_MASK        0x0000E000    /* ADC sampling frequency */
#define CM_ASFC_SHIFT        13
#define CM_DSFC_MASK        0x00001C00    /* DAC sampling frequency */
#define CM_DSFC_SHIFT        10
#define CM_SPDF_1        0x00000200    /* SPDIF IN/OUT at channel B */
#define CM_SPDF_0        0x00000100    /* SPDIF OUT only channel A */
#define CM_SPDFLOOP        0x00000080    /* ext. SPDIIF/OUT -> IN loopback */
#define CM_SPDO2DAC        0x00000040    /* SPDIF/OUT can be heard from internal DAC */
#define CM_INTRM        0x00000020    /* master control block (MCB) interrupt enabled */
#define CM_BREQ            0x00000010    /* bus master enabled */
#define CM_VOICE_EN        0x00000008    /* legacy voice (SB16,FM) */
#define CM_UART_EN        0x00000004    /* UART */
#define CM_JYSTK_EN        0x00000002    /* joy stick */

#define CM_REG_CHFORMAT        0x08

#define CM_CHB3D5C        0x80000000    /* 5,6 channels */
#define CM_CHB3D        0x20000000    /* 4 channels */

#define CM_CHIP_MASK1        0x1f000000
#define CM_CHIP_037        0x01000000

#define CM_SPDIF_SELECT1    0x00080000    /* for model <= 037 ? */
#define CM_AC3EN1        0x00100000    /* enable AC3: model 037 */
#define CM_SPD24SEL        0x00020000    /* 24bit spdif: model 037 */

#define CM_ADCBITLEN_MASK    0x0000C000
#define CM_ADCBITLEN_16        0x00000000
#define CM_ADCBITLEN_15        0x00004000
#define CM_ADCBITLEN_14        0x00008000
#define CM_ADCBITLEN_13        0x0000C000

#define CM_ADCDACLEN_MASK    0x00003000
#define CM_ADCDACLEN_060    0x00000000
#define CM_ADCDACLEN_066    0x00001000
#define CM_ADCDACLEN_130    0x00002000 //default
#define CM_ADCDACLEN_280    0x00003000

#define CM_CH1_SRATE_176K    0x00000800
#define CM_CH1_SRATE_88K    0x00000400
#define CM_CH0_SRATE_176K    0x00000200
#define CM_CH0_SRATE_88K    0x00000100

#define CM_SPDIF_INVERSE2    0x00000080    /* model 055? */

#define CM_CH1FMT_MASK        0x0000000C
#define CM_CH1FMT_SHIFT        2
#define CM_CH0FMT_MASK        0x00000003
#define CM_CH0FMT_SHIFT        0

#define CM_REG_INT_HLDCLR    0x0C
#define CM_CHIP_MASK2        0xff000000
#define CM_CHIP_039        0x04000000
#define CM_CHIP_039_6CH        0x01000000
#define CM_TDMA_INT_EN        0x00040000
#define CM_CH1_INT_EN        0x00020000
#define CM_CH0_INT_EN        0x00010000
#define CM_INT_HOLD        0x00000002
#define CM_INT_CLEAR        0x00000001

#define CM_REG_INT_STATUS    0x10 //(R)
#define CM_INTR            0x80000000 //Interrupt reflected from any sources.
#define CM_VCO            0x08000000    /* Voice Control? CMI8738 */
#define CM_MCBINT        0x04000000    /* Master Control Block abort cond.? */
#define CM_UARTINT        0x00010000
#define CM_LTDMAINT        0x00008000
#define CM_HTDMAINT        0x00004000
#define CM_XDO46        0x00000080    /* Modell 033? Direct programming EEPROM (read data register) */
#define CM_LHBTOG        0x00000040    /* High/Low status from DMA ctrl register */
#define CM_LEG_HDMA        0x00000020    /* Legacy is in High DMA channel */
#define CM_LEG_STEREO        0x00000010    /* Legacy is in Stereo mode */
#define CM_CH1BUSY        0x00000008
#define CM_CH0BUSY        0x00000004
#define CM_CHINT1        0x00000002
#define CM_CHINT0        0x00000001
#define CM_INTR_MASK (CM_INTR|CM_UARTINT|CM_LTDMAINT|CM_HTDMAINT|CM_CHINT1|CM_CHINT0)

#define CM_REG_LEGACY_CTRL    0x14
#define CM_NXCHG        0x80000000    /* h/w multi channels? */
#define CM_VMPU_MASK        0x60000000    /* MPU401 i/o port address */
#define CM_VMPU_330        0x00000000
#define CM_VMPU_320        0x20000000
#define CM_VMPU_310        0x40000000
#define CM_VMPU_300        0x60000000
#define CM_VSBSEL_MASK        0x0C000000    /* SB16 base address */
#define CM_VSBSEL_220        0x00000000
#define CM_VSBSEL_240        0x04000000
#define CM_VSBSEL_260        0x08000000
#define CM_VSBSEL_280        0x0C000000
#define CM_FMSEL_MASK        0x03000000    /* FM OPL3 base address */
#define CM_FMSEL_388        0x00000000
#define CM_FMSEL_3C8        0x01000000
#define CM_FMSEL_3E0        0x02000000
#define CM_FMSEL_3E8        0x03000000
#define CM_ENSPDOUT        0x00800000    /* enable XPDIF/OUT to I/O interface */
#define CM_SPDCOPYRHT        0x00400000    /* set copyright spdif in/out */
#define CM_DAC2SPDO        0x00200000    /* enable wave+fm_midi -> SPDIF/OUT */
#define CM_SETRETRY        0x00010000    /* 0: legacy i/o wait (default), 1: legacy i/o bus retry */
#define CM_CHB3D6C        0x00008000    /* 5.1 channels support */
#define CM_LINE_AS_BASS        0x00006000    /* use line-in as bass */

#define CM_REG_MISC_CTRL    0x18
#define CM_PWD            0x80000000
#define CM_RESET        0x40000000
#define CM_SFIL_MASK        0x30000000
#define CM_TXVX            0x08000000
#define CM_N4SPK3D        0x04000000    /* 4ch output */
#define CM_SPDO5V        0x02000000    /* 5V spdif output (1 = 0.5v (coax)) */
#define CM_SPDIF48K        0x01000000    /* write */
#define CM_SPATUS48K        0x01000000    /* read */
#define CM_ENDBDAC        0x00800000    /* enable dual dac */
#define CM_XCHGDAC        0x00400000    /* 0: front=ch0, 1: front=ch1 */
#define CM_SPD32SEL        0x00200000    /* 0: 16bit SPDIF, 1: 32bit */
#define CM_SPDFLOOPI        0x00100000    /* int. SPDIF-IN -> int. OUT */
#define CM_FM_EN        0x00080000    /* enalbe FM */
#define CM_AC3EN2        0x00040000    /* enable AC3: model 039 */
#define CM_VIDWPDSB        0x00010000
#define CM_SPDF_AC97        0x00008000    /* 0: SPDIF/OUT 44.1K, 1: 48K */
#define CM_MASK_EN        0x00004000
#define CM_VIDWPPRT        0x00002000
#define CM_SFILENB        0x00001000
#define CM_MMODE_MASK        0x00000E00
#define CM_SPDIF_SELECT2    0x00000100    /* for model > 039 ? */
#define CM_ENCENTER        0x00000080
#define CM_FLINKON        0x00000040
#define CM_FLINKOFF        0x00000020
#define CM_MIDSMP        0x00000010
#define CM_UPDDMA_MASK        0x0000000C
#define CM_TWAIT_MASK        0x00000003

    /* byte */
#define CM_REG_MIXER0        0x20
#define CM_REG_MIXERC       0x21
#define  CM_X_SB16          0x01
#define CM_REG_SB16_DATA    0x22
#define CM_REG_SB16_ADDR    0x23

#define CM_REFFREQ_XIN        (315*1000*1000)/22    /* 14.31818 Mhz reference clock frequency pin XIN */
#define CM_ADCMULT_XIN        512            /* Guessed (487 best for 44.1kHz, not for 88/176kHz) */
#define CM_TOLERANCE_RATE    0.001            /* Tolerance sample rate pitch (1000ppm) */
#define CM_MAXIMUM_RATE        80000000        /* Note more than 80MHz */

#define CM_REG_MIXER1        0x24
#define CM_FMMUTE        0x80    /* mute FM */
#define CM_FMMUTE_SHIFT        7
#define CM_WSMUTE        0x40    /* mute PCM */
#define CM_WSMUTE_SHIFT        6
#define CM_SPK4            0x20    /* lin-in -> rear line out */
#define CM_SPK4_SHIFT        5
#define CM_REAR2FRONT        0x10    /* exchange rear/front */
#define CM_REAR2FRONT_SHIFT    4
#define CM_WAVEINL        0x08    /* digital wave rec. left chan */
#define CM_WAVEINL_SHIFT    3
#define CM_WAVEINR        0x04    /* digical wave rec. right */
#define CM_WAVEINR_SHIFT    2
#define CM_X3DEN        0x02    /* 3D surround enable */
#define CM_X3DEN_SHIFT        1
#define CM_CDPLAY        0x01    /* enable SPDIF/IN PCM -> DAC */
#define CM_CDPLAY_SHIFT        0

#define CM_REG_MIXER2        0x25
#define CM_RAUXREN        0x80    /* AUX right capture */
#define CM_RAUXREN_SHIFT    7
#define CM_RAUXLEN        0x40    /* AUX left capture */
#define CM_RAUXLEN_SHIFT    6
#define CM_VAUXRM        0x20    /* AUX right mute */
#define CM_VAUXRM_SHIFT        5
#define CM_VAUXLM        0x10    /* AUX left mute */
#define CM_VAUXLM_SHIFT        4
#define CM_VADMIC_MASK        0x0e    /* mic gain level (0-3) << 1 */
#define CM_VADMIC_SHIFT        1
#define CM_MICGAINZ        0x01    /* mic boost */
#define CM_MICGAINZ_SHIFT    0

#define CM_REG_AUX_VOL        0x26
#define CM_VAUXL_MASK        0xf0
#define CM_VAUXR_MASK        0x0f

#define CM_REG_MISC        0x27
#define CM_XGPO1        0x20
#define CM_MIC_CENTER_LFE    0x04    /* mic as center/lfe out? (model 039 or later?) */
#define CM_SPDIF_INVERSE    0x04    /* spdif input phase inverse (model 037) */
#define CM_SPDVALID        0x02    /* spdif input valid check */
#define CM_DMAUTO        0x01

#define CM_REG_AC97        0x28    /* hmmm.. do we have ac97 link? */
/*
 * For CMI-8338 (0x28 - 0x2b) .. is this valid for CMI-8738
 * or identical with AC97 codec?
 */
#define CM_REG_EXTERN_CODEC    CM_REG_AC97

/*
 * MPU401 pci port index address 0x40 - 0x4f (CMI-8738 spec ver. 0.6)
 */
#define CM_REG_MPU_PCI        0x40

/*
 * FM pci port index address 0x50 - 0x5f (CMI-8738 spec ver. 0.6)
 */
#define CM_REG_FM_PCI        0x50

#define CM_REG_EXT_MISC      0x90

/*
 * for CMI-8338 .. this is not valid for CMI-8738.
 */
#define CM_REG_EXTENT_IND    0xf0
#define CM_VPHONE_MASK        0xe0    /* Phone volume control (0-3) << 5 */
#define CM_VPHONE_SHIFT        5
#define CM_VPHOM        0x10    /* Phone mute control */
#define CM_VSPKM        0x08    /* Speaker mute control, default high */
#define CM_RLOOPREN        0x04    /* Rec. R-channel enable */
#define CM_RLOOPLEN        0x02    /* Rec. L-channel enable */

/*
 * CMI-8338 spec ver 0.5 (this is not valid for CMI-8738):
 * the 8 registers 0xf8 - 0xff are used for programming m/n counter by the PLL
 * unit (readonly?).
 */
#define CM_REG_PLL        0xf8

/*
 * extended registers
 */
#define CM_REG_CH0_FRAME1    0x80    /* base address */
#define CM_REG_CH0_FRAME2    0x84    /* 0-15: count of samples at bus master; buffer size */
                                     /* 16-31: count of samples at codec; fragment size */
#define CM_REG_CH1_FRAME1    0x88
#define CM_REG_CH1_FRAME2    0x8C

/*
 * size of i/o region
 */
#define CM_EXTENT_CODEC      0x100
#define CM_EXTENT_MIDI      0x2
#define CM_EXTENT_SYNTH      0x4

/* fixed legacy joystick address */
#define CM_JOYSTICK_ADDR    0x200


/*
 * pci ids
 */
#define PCI_DEVICE_ID_CMEDIA_CM8338A    0x0100
#define PCI_DEVICE_ID_CMEDIA_CM8338B    0x0101
#define PCI_DEVICE_ID_CMEDIA_CM8738  0x0111
#define PCI_DEVICE_ID_CMEDIA_CM8738B 0x0112

/*
 * channels for playback / capture
 */
#define CM_CH_PLAY    0
#define CM_CH_CAPT    1

/*
 * flags to check device open/close
 */
#define CM_OPEN_NONE    0
#define CM_OPEN_CH_MASK    0x01
#define CM_OPEN_DAC    0x10
#define CM_OPEN_ADC    0x20
#define CM_OPEN_SPDIF    0x40
#define CM_OPEN_MCHAN    0x80
#define CM_OPEN_PLAYBACK    (CM_CH_PLAY | CM_OPEN_DAC)
#define CM_OPEN_PLAYBACK2    (CM_CH_CAPT | CM_OPEN_DAC)
#define CM_OPEN_PLAYBACK_MULTI    (CM_CH_PLAY | CM_OPEN_DAC | CM_OPEN_MCHAN)
#define CM_OPEN_CAPTURE        (CM_CH_CAPT | CM_OPEN_ADC)
#define CM_OPEN_SPDIF_PLAYBACK    (CM_CH_PLAY | CM_OPEN_DAC | CM_OPEN_SPDIF)
#define CM_OPEN_SPDIF_CAPTURE    (CM_CH_CAPT | CM_OPEN_ADC | CM_OPEN_SPDIF)


#if CM_CH_PLAY == 1
#define CM_PLAYBACK_SRATE_176K    CM_CH1_SRATE_176K
#define CM_PLAYBACK_SPDF    CM_SPDF_1
#define CM_CAPTURE_SPDF        CM_SPDF_0
#else
#define CM_PLAYBACK_SRATE_176K CM_CH0_SRATE_176K
#define CM_PLAYBACK_SPDF    CM_SPDF_0
#define CM_CAPTURE_SPDF        CM_SPDF_1
#endif

extern struct sndcard_info_s CMI8X38_sndcard_info;

struct cmi8x38_card_s
{
 unsigned long   iobase;
 unsigned short     model;
 unsigned int    irq;
 unsigned char   chiprev;
 unsigned char   device_type;
 struct pci_config_s  pci_dev;

 struct cardmem_s dm;
 char *pcmout_buffer;
 long pcmout_bufsize;

 unsigned int ctrl;

 int chip_version;
 int max_channels;
 unsigned int can_multi_ch;

 unsigned int dma_size;          /* in frames */
 unsigned int period_size;    /* in frames */
 unsigned int fmt;          /* format bits */

 int shift;

 uint8_t uart_backlog[32];
 uint8_t uart_in, uart_out;
 uint8_t uart_delay;
};

extern unsigned int intsoundconfig,intsoundcontrol;

//-------------------------------------------------------------------------
// low level write & read

#define snd_cmipci_write_8(cm,reg,data)  do { outb(cm->iobase+reg,data); _CMI_LOG("%x: %x %x\n",reg,data,inb(cm->iobase+reg)); } while(cm->chip_version<=37 && inb(cm->iobase+reg) != (data))
#define snd_cmipci_write_8nv(cm,reg,data)  do { outb(cm->iobase+reg,data); _CMI_LOG("%x: %x %x\n",reg,data,inb(cm->iobase+reg)); } while(0)
#define snd_cmipci_write_16(cm,reg,data) do { outw(cm->iobase+reg,data); _CMI_LOG("%x: %x %x\n",reg,data,inw(cm->iobase+reg)); } while(cm->chip_version<=37 && inw(cm->iobase+reg) != (data))
#define snd_cmipci_write_16nv(cm,reg,data) do { outw(cm->iobase+reg,data); _CMI_LOG("%x: %x %x\n",reg,data,inw(cm->iobase+reg)); } while(0)
#define snd_cmipci_write_32(cm,reg,data) do { outl(cm->iobase+reg,data); _CMI_LOG("%x: %x %x\n",reg,data,inl(cm->iobase+reg)); } while(cm->chip_version<=37 && inl(cm->iobase+reg) != (data))
#define snd_cmipci_write_32m(cm,reg,data,mask) do { outl(cm->iobase+reg,data); _CMI_LOG("%x: %x %x\n",reg,data,inl(cm->iobase+reg)); } while(cm->chip_version<=37 && (inl(cm->iobase+reg)&(mask)) != ((data)&mask))
#define snd_cmipci_read_8(cm,reg)  inb(cm->iobase+reg)
#define snd_cmipci_read_16(cm,reg) inw(cm->iobase+reg)
#define snd_cmipci_read_32(cm,reg) inl(cm->iobase+reg)

static void snd_cmipci_set_bit(struct cmi8x38_card_s *cm, unsigned int cmd, unsigned int flag)
{
 unsigned int val;
 do {
 val = snd_cmipci_read_32(cm, cmd);
 _CMI_LOG("%x %x\n",cmd, val);
 } while(val == -1 && cm->chip_version <= 37);
 val|= flag;
 snd_cmipci_write_32(cm, cmd, val);
}

static void snd_cmipci_clear_bit(struct cmi8x38_card_s *cm, unsigned int cmd, unsigned int flag)
{
 unsigned int val;
 do {
 val = snd_cmipci_read_32(cm, cmd);
 _CMI_LOG("%x %x\n",cmd, val);
 } while(val == -1 && cm->chip_version <= 37);
 val&= ~flag;
 snd_cmipci_write_32(cm, cmd, val);
}

static void snd_cmipci_mixer_write(struct cmi8x38_card_s *cm, unsigned char idx, unsigned char data)
{
 snd_cmipci_write_8nv(cm, CM_REG_SB16_ADDR, idx);
 snd_cmipci_write_8nv(cm, CM_REG_SB16_DATA, data);
}

static unsigned int snd_cmipci_mixer_read(struct cmi8x38_card_s *cm, unsigned char idx)
{
 snd_cmipci_write_8nv(cm, CM_REG_SB16_ADDR, idx);
 return snd_cmipci_read_8(cm, CM_REG_SB16_DATA);
}

//-------------------------------------------------------------------------

static unsigned int cmi_rates[] = { 5512, 11025, 22050, 44100, 8000, 16000, 32000, 48000 };

static unsigned int snd_cmipci_rate_freq(unsigned int rate)
{
 unsigned int i;
 for(i = 0; i < 8; i++) {
  if(cmi_rates[i] == rate)
   return i;
 }
 return 7; // 48k
}

static void snd_cmipci_ch_reset(struct cmi8x38_card_s *cm, int ch) //reset channel ch
{
 int reset = CM_RST_CH0 << ch;
 int adcch = CM_CHADC0 << ch;
 snd_cmipci_write_32(cm, CM_REG_FUNCTRL0, adcch|reset);
 do {uint32_t x; pds_delay_10us(10); x = snd_cmipci_read_32(cm,CM_REG_FUNCTRL0);} while(!(snd_cmipci_read_32(cm,CM_REG_FUNCTRL0)&reset));
 snd_cmipci_write_32(cm, CM_REG_FUNCTRL0, adcch&(~reset));
 do {pds_delay_10us(10);} while((snd_cmipci_read_32(cm,CM_REG_FUNCTRL0)&reset));
 pds_delay_10us(500); 
}

static int set_dac_channels(struct cmi8x38_card_s *cm, int channels)
{
  if(cm->can_multi_ch){
   snd_cmipci_clear_bit(cm, CM_REG_LEGACY_CTRL, CM_NXCHG);
   snd_cmipci_clear_bit(cm, CM_REG_CHFORMAT, CM_CHB3D);
   snd_cmipci_clear_bit(cm, CM_REG_CHFORMAT, CM_CHB3D5C);
   snd_cmipci_clear_bit(cm, CM_REG_LEGACY_CTRL, CM_CHB3D6C);
   snd_cmipci_clear_bit(cm, CM_REG_MISC_CTRL, CM_ENCENTER);
  }
 return 0;
}

static void query_chip(struct cmi8x38_card_s *cm)
{
 unsigned int detect;

 /* check reg 0Ch, bit 24-31 */
 detect = snd_cmipci_read_32(cm, CM_REG_INT_HLDCLR) & CM_CHIP_MASK2;
 if(!detect){
  /* check reg 08h, bit 24-28 */
  detect = snd_cmipci_read_32(cm, CM_REG_CHFORMAT) & CM_CHIP_MASK1;
  if(!detect) {
   cm->chip_version = 33;
   cm->max_channels = 2;
  }else if(detect == CM_CHIP_037) { //CMI-8738/PCI-SX, 4 chnannel, bit 31-24 in CM_REG_CHFORMAT VER[0,7]: PCI Audio subversion for internal indentification. "01"
   cm->chip_version = 37;
   cm->max_channels = 2;
  }else{
   cm->chip_version = 39;
   cm->max_channels = 2;
  }
 }else{
  /* check reg 0Ch, bit 26 */
  if(detect&CM_CHIP_039){
   cm->chip_version = 39;
   if(detect & CM_CHIP_039_6CH)
    cm->max_channels  = 6;
   else
    cm->max_channels = 4;
   cm->can_multi_ch = 1;
  }else{
   cm->chip_version = 55; /* 4 or 6 channels */
   cm->max_channels  = 6;
   cm->can_multi_ch = 1;
  }
 }
}

static void cmi8x38_chip_init(struct cmi8x38_card_s *cm)
{
 unsigned int val;
 cm->ctrl = 0; // ch0 playback

 cm->chip_version = 0;
 cm->max_channels = 2;
 if (cm->pci_dev.device_id != PCI_DEVICE_ID_CMEDIA_CM8338A &&
        cm->pci_dev.device_id != PCI_DEVICE_ID_CMEDIA_CM8338B)
  query_chip(cm);

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "chip version: %d, multi_chan: %d", cm->chip_version, cm->can_multi_ch);

 /* initialize codec registers */
 snd_cmipci_set_bit(cm, CM_REG_MISC_CTRL, CM_RESET); //reset DSP/Bus master
 do {pds_delay_10us(10);} while(!(snd_cmipci_read_32(cm, CM_REG_MISC_CTRL)&CM_RESET));
 snd_cmipci_clear_bit(cm, CM_REG_MISC_CTRL, CM_RESET); //release reset
 do {pds_delay_10us(10);} while((snd_cmipci_read_32(cm, CM_REG_MISC_CTRL)&CM_RESET));
 pds_delay_10us(1000);

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL0: %x",  snd_cmipci_read_32(cm, CM_REG_FUNCTRL0));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL1: %x",  snd_cmipci_read_32(cm, CM_REG_FUNCTRL1));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "LEGCCTRL: %x",  snd_cmipci_read_32(cm, CM_REG_LEGACY_CTRL));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "MISCCTRL: %x",  snd_cmipci_read_32(cm, CM_REG_MISC_CTRL));

 if(cm->chip_version <= 37)
  pcibios_WriteConfig_Dword(&cm->pci_dev, 0x40, 0); //disable DMA slave

 snd_cmipci_write_32(cm, CM_REG_INT_HLDCLR, 0);    /* disable ints */
 snd_cmipci_ch_reset(cm, CM_CH_PLAY);
 snd_cmipci_ch_reset(cm, CM_CH_CAPT);
 snd_cmipci_write_32(cm, CM_REG_FUNCTRL0, 0);    /* disable channels */
 snd_cmipci_write_32(cm, CM_REG_FUNCTRL1, 0);

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL0: %x",  snd_cmipci_read_32(cm, CM_REG_FUNCTRL0));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL1: %x",  snd_cmipci_read_32(cm, CM_REG_FUNCTRL1));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "LEGCCTRL: %x",  snd_cmipci_read_32(cm, CM_REG_LEGACY_CTRL));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "MISCCTRL: %x",  snd_cmipci_read_32(cm, CM_REG_MISC_CTRL));

 snd_cmipci_write_32m(cm, CM_REG_CHFORMAT, 0, 0xFFFFFF);
 snd_cmipci_set_bit(cm, CM_REG_MISC_CTRL, CM_ENDBDAC|CM_N4SPK3D);
 snd_cmipci_clear_bit(cm, CM_REG_MISC_CTRL, CM_XCHGDAC);

 snd_cmipci_clear_bit(cm, CM_REG_MISC_CTRL, CM_SPD32SEL|CM_AC3EN2); //disable 32bit PCM/AC3

 /* Set Bus Master Request */
 snd_cmipci_set_bit(cm, CM_REG_FUNCTRL1, CM_BREQ);

 /* Assume TX and compatible chip set (Autodetection required for VX chip sets) */
 switch(cm->pci_dev.device_id) {
  case PCI_DEVICE_ID_CMEDIA_CM8738:
  case PCI_DEVICE_ID_CMEDIA_CM8738B:
       /* PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82437VX */
       if(pcibios_FindDevice(0x8086, 0x7030,NULL)!=PCI_SUCCESSFUL)
        snd_cmipci_set_bit(cm, CM_REG_MISC_CTRL, CM_TXVX); //bit set means TX
       break;
 }

 /* reset mixer */
 snd_cmipci_mixer_write(cm, 0, 0);

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL0: %x",  snd_cmipci_read_32(cm, CM_REG_FUNCTRL0));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL1: %x",  snd_cmipci_read_32(cm, CM_REG_FUNCTRL1));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "LEGCCTRL: %x",  snd_cmipci_read_32(cm, CM_REG_LEGACY_CTRL));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "MISCCTRL: %x",  snd_cmipci_read_32(cm, CM_REG_MISC_CTRL));

}

static void cmi8x38_chip_close(struct cmi8x38_card_s *cm)
{
 if(cm->iobase){
  snd_cmipci_clear_bit(cm, CM_REG_MISC_CTRL, CM_FM_EN);
  snd_cmipci_clear_bit(cm, CM_REG_LEGACY_CTRL, CM_ENSPDOUT);
  snd_cmipci_write_32(cm, CM_REG_INT_HLDCLR, 0);  /* disable ints */
  snd_cmipci_ch_reset(cm, CM_CH_PLAY);
  snd_cmipci_ch_reset(cm, CM_CH_CAPT);
  snd_cmipci_write_32(cm, CM_REG_FUNCTRL0, 0); /* disable channels */
  snd_cmipci_write_32(cm, CM_REG_FUNCTRL1, 0);

  /* reset mixer */
  snd_cmipci_mixer_write(cm, 0, 0);
 }
}

//-------------------------------------------------------------------------
static struct pci_device_s cmi_devices[]={
 {"8338A",0x13F6,0x0100},
 {"8338B",0x13F6,0x0101},
 {"8738" ,0x13F6,0x0111},
 {"8738B",0x13F6,0x0112},
 {NULL,0,0}
};

static void CMI8X38_close(struct audioout_info_s *aui);

static unsigned int snd_cmi_buffer_init(struct cmi8x38_card_s *card, struct audioout_info_s *aui)
///////////////////////////////////////////////////////////////////////////////////////////////////
{
    unsigned int bytes_per_sample = 2;
    card->pcmout_bufsize = MDma_get_max_pcmoutbufsize( aui, 0,
      aui->gvars->period_size ? aui->gvars->period_size : PCMBUFFERPAGESIZE, bytes_per_sample, 48000 );
    if (!MDma_alloc_cardmem( &card->dm, PCMBUFFERALIGNMENT + card->pcmout_bufsize)) return 0;
    /* pagetable requires 8 byte align; MDma_alloc_cardmem() returns 1kB aligned ptr */
    card->pcmout_buffer=(void *)(((uint32_t)card->dm.pMem + PCMBUFFERALIGNMENT-1)&(~(PCMBUFFERALIGNMENT-1))); // buffer begins on page (4096 bytes) boundary
    aui->card_DMABUFF = card->pcmout_buffer;
#if 0 //def SBEMU
    memset(card->pcmout_buffer, 0, card->pcmout_bufsize);
#endif
    dbgprintf(("snd_cmi_buffer init: pcmoutbuf:%X size:%d\n",(unsigned long)card->pcmout_buffer,card->pcmout_bufsize));
    return 1;
}


static int CMI8X38_adetect(struct audioout_info_s *aui)
{
 struct cmi8x38_card_s *card;

 card=(struct cmi8x38_card_s *)calloc(1,sizeof(struct cmi8x38_card_s));
 if(!card)
  return 0;
 aui->card_private_data=card;

 if(pcibios_search_devices(cmi_devices,&card->pci_dev)!=PCI_SUCCESSFUL)
  goto err_adetect;

 dbgprintf(("CMI8X38_adetect: enable PCI io and busmaster\n"));
 pcibios_enable_BM_IO(&card->pci_dev);

 card->iobase = pcibios_ReadConfig_Dword(&card->pci_dev, PCIR_NAMBAR)&0xfff0;
 if(!card->iobase)
  goto err_adetect;
 aui->card_irq = card->irq = pcibios_ReadConfig_Byte(&card->pci_dev, PCIR_INTR_LN);
 card->chiprev=pcibios_ReadConfig_Byte(&card->pci_dev, PCIR_RID);
 card->model  =pcibios_ReadConfig_Word(&card->pci_dev, PCIR_SSID);

 // alloc buffers
 if(!snd_cmi_buffer_init(card, aui))
  goto err_adetect;
 // init chip
 cmi8x38_chip_init(card);

 sprintf(cmi8x38_shortname, CMI8X38_CARD " (%u)", card->chip_version % 1000);

 card->uart_in = card->uart_out = 0;
 snd_cmipci_read_8(card, CM_REG_MPU_PCI + 1);

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "did init, IRQ: %d, iobase: %x", card->irq, card->iobase);

 return 1;

err_adetect:
 CMI8X38_close(aui);
 return 0;
}

static void CMI8X38_close(struct audioout_info_s *aui)
{
 struct cmi8x38_card_s *card=aui->card_private_data;
 if(card){
  if(card->iobase)
   cmi8x38_chip_close(card);
  MDma_free_cardmem(&card->dm);
  free(card);
  aui->card_private_data=NULL;
 }
}

static void CMI8X38_setrate(struct audioout_info_s *aui)
{
 struct cmi8x38_card_s *card=aui->card_private_data;
 unsigned int dmabufsize,val,freqnum;
 int periods;
 int dac_to_spdiff = 0;

 if(aui->freq_card<5512)
  aui->freq_card=5512;
 else{
  if(aui->freq_card>48000)
   aui->freq_card=48000;
 }

 aui->chan_card=2;
 aui->bits_card=16;
 aui->card_wave_id=WAVEID_PCM_SLE;

 dmabufsize=MDma_init_pcmoutbuf(aui, card->pcmout_bufsize,
   aui->gvars->period_size ? aui->gvars->period_size : PCMBUFFERPAGESIZE,48000);

 //format cfg
 card->fmt=0;
 card->shift=0;
 if(aui->bits_card>=16){
  card->fmt|=0x02;
  card->shift++;
 }
 if(aui->chan_card>1){
  card->fmt|=0x01;
  card->shift++;
 }
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "card->shift: %d", card->shift);

 //
 if(set_dac_channels(card,aui->chan_card)!=0)
  return;

 //buffer cfg
#if 1 //SBEMU
 periods = max(1, dmabufsize / PCMBUFFERPAGESIZE);
 card->period_size =
   (aui->gvars->period_size ? aui->gvars->period_size : dmabufsize/periods)
     >> card->shift;
 card->dma_size    = dmabufsize >> card->shift;
#endif
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "0FUNCTRL0: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL0));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL1: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL1));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "LEGCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_LEGACY_CTRL));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "MISCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_MISC_CTRL));


  // set buffer address
 snd_cmipci_write_32(card, CM_REG_CH0_FRAME1, (uint32_t) pds_cardmem_physicalptr(card->dm, card->pcmout_buffer));
 // program sample counts
 snd_cmipci_write_16nv(card, CM_REG_CH0_FRAME2    , card->dma_size - 1);
 snd_cmipci_write_16nv(card, CM_REG_CH0_FRAME2 + 2, card->period_size - 1);

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "1FUNCTRL0: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL0));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL1: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL1));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "LEGCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_LEGACY_CTRL));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "MISCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_MISC_CTRL));

 // set sample rate
 freqnum = snd_cmipci_rate_freq(aui->freq_card);
 aui->freq_card=cmi_rates[freqnum]; // if the freq-config is not standard at CMI

 // DAC
 do {
  val = snd_cmipci_read_32(card, CM_REG_FUNCTRL1);
 }while(card->chip_version <= 37 && val == -1);
 val &= ~CM_DSFC_MASK;
 val |= (freqnum << CM_DSFC_SHIFT) & CM_DSFC_MASK;
 snd_cmipci_write_32(card, CM_REG_FUNCTRL1, val);

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "2FUNCTRL0: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL0));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL1: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL1));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "LEGCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_LEGACY_CTRL));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "MISCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_MISC_CTRL));


 // set format
 do {
  val = snd_cmipci_read_32(card, CM_REG_CHFORMAT);
 }while(card->chip_version <= 37 && val == -1);

 val &= ~CM_CH0FMT_MASK;
 val |= card->fmt << CM_CH0FMT_SHIFT;
 snd_cmipci_write_32m(card, CM_REG_CHFORMAT, val, 0xFFFFFF);
 pds_delay_10us(1000);
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "3CHFORMAT: %x",  snd_cmipci_read_32(card, CM_REG_CHFORMAT));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL0: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL0));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "FUNCTRL1: %x",  snd_cmipci_read_32(card, CM_REG_FUNCTRL1));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "LEGCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_LEGACY_CTRL));
 mpxplay_debugf(CMI_DEBUG_OUTPUT, "MISCCTRL: %x",  snd_cmipci_read_32(card, CM_REG_MISC_CTRL));

 if(aui->gvars->pin == 3) {
  snd_cmipci_set_bit(card, CM_REG_FUNCTRL1, CM_PLAYBACK_SPDF);
  snd_cmipci_set_bit(card, CM_REG_LEGACY_CTRL, CM_ENSPDOUT);

  if(card->chip_version <= 37 || aui->gvars->dig_out_override) {
   snd_cmipci_set_bit(card, CM_REG_LEGACY_CTRL, CM_DAC2SPDO);
   dac_to_spdiff = 1;
  }
 } else {
  snd_cmipci_clear_bit(card, CM_REG_FUNCTRL1, CM_PLAYBACK_SPDF);
 }

 if(!aui->gvars->cd_in_disable) {
  int val;
  do {
   val = snd_cmipci_read_8(card, CM_REG_MIXER1);
   _CMI_LOG("%x %x\n", CM_REG_MIXER1, val);
  } while(val == -1 && card->chip_version <= 37);
  val|= CM_CDPLAY;
  snd_cmipci_write_8(card, CM_REG_MIXER1, val);
 }

 mpxplay_debugf(CMI_DEBUG_OUTPUT, "setrate done freqnum: %d  freq: %d", freqnum, aui->freq_card);
}

static void CMI8X38_start(struct audioout_info_s *aui)
{
 struct cmi8x38_card_s *card=aui->card_private_data;

 /* disable FM */
 if(!aui->gvars->opl3 && !aui->gvars->legacy_fm_disable)
  snd_cmipci_set_bit(card, CM_REG_MISC_CTRL, CM_FM_EN);
 else
  snd_cmipci_clear_bit(card, CM_REG_MISC_CTRL, CM_FM_EN);
 snd_cmipci_set_bit(card, CM_REG_FUNCTRL1, CM_JYSTK_EN);

 if(!aui->gvars->mpu && aui->gvars->legacy_uart_enable)
  snd_cmipci_set_bit(card, CM_REG_FUNCTRL1, CM_UART_EN);
 else
  snd_cmipci_clear_bit(card, CM_REG_FUNCTRL1, CM_UART_EN);

 if(aui->gvars->try_slower_uart)
  card->uart_delay = 40;
 else 
  card->uart_delay = 25;

#if 1 //def SBEMU
 snd_cmipci_write_32(card, CM_REG_INT_HLDCLR, CM_CH0_INT_EN);    /* enable ints */
#endif
 card->ctrl |= CM_CHEN0;
 card->ctrl &= ~CM_PAUSE0;
 card->ctrl &= ~CM_CHADC0;
 snd_cmipci_write_32(card, CM_REG_FUNCTRL0, card->ctrl);
}

static void CMI8X38_stop(struct audioout_info_s *aui)
{
 struct cmi8x38_card_s *card=aui->card_private_data;

 snd_cmipci_clear_bit(card, CM_REG_FUNCTRL1, CM_JYSTK_EN);

#if 1 //def SBEMU
 snd_cmipci_write_32(card, CM_REG_INT_HLDCLR, 0);    /* disable ints */
#endif
 card->ctrl &= ~CM_CHEN0;
 card->ctrl |= CM_PAUSE0;
 snd_cmipci_write_32(card, CM_REG_FUNCTRL0, card->ctrl);
 snd_cmipci_write_32(card, CM_REG_FUNCTRL0, card->ctrl | CM_RST_CH0);
 snd_cmipci_write_32(card, CM_REG_FUNCTRL0, card->ctrl & ~CM_RST_CH0);
}

static long CMI8X38_getbufpos(struct audioout_info_s *aui)
{
 struct cmi8x38_card_s *card=aui->card_private_data;
 unsigned long bufpos;

#if 1 //SBEMU
 // From the Linux driver
 unsigned int reg = CM_REG_CH0_FRAME2;
 unsigned int rem, tries;
 for (tries = 0; tries < 3; tries++) {
   do {rem = snd_cmipci_read_16(card, reg); //note: current sample count can be 0
   }while(rem == 0xFFFF && card->dma_size-1 != 0xFFFF);
   if (rem < card->dma_size)
     goto ok;
 }
 return aui->card_dma_lastgoodpos;
 ok:
  bufpos = (card->dma_size - (rem + 1)) << card->shift;
#endif

  if (bufpos < aui->card_dmasize)
    aui->card_dma_lastgoodpos = bufpos;
  return aui->card_dma_lastgoodpos;
}

static void CMI8X38_clearbuf(struct audioout_info_s *aui)
{
 MDma_clearbuf(aui);
}

//--------------------------------------------------------------------------
//mixer

static void CMI8X38_writeMIXER(struct audioout_info_s *aui,unsigned long reg, unsigned long val)
{
 struct cmi8x38_card_s *card=aui->card_private_data;
 snd_cmipci_mixer_write(card,reg,val);
}

static unsigned long CMI8X38_readMIXER(struct audioout_info_s *aui,unsigned long reg)
{
 struct cmi8x38_card_s *card=aui->card_private_data;
 return snd_cmipci_mixer_read(card,reg);
}

static int CMI8X38_IRQRoutine(struct audioout_info_s* aui)
{
  struct cmi8x38_card_s *card=aui->card_private_data;
  unsigned int mask = 0;
  int status = snd_cmipci_read_32(card, CM_REG_INT_STATUS); //read only reg (R)
  if (status == -1) {
    int timeout = 2000;
    do {
      status = snd_cmipci_read_32(card, CM_REG_INT_STATUS);
      if (status != -1) break;
    } while (--timeout);
  }
  if ((card->chip_version > 37 && !(status&CM_INTR)) ||
      (card->chip_version <= 37 && !(status & CM_INTR_MASK))) { //the summary bit is incorrect for PCI-SX, the interrupt be chained to other shared IRQ device with invalid interrupts
    return 0;
  }
  if (status&CM_MCBINT) //Abort conditions occur during PCI Bus Target/Master Access
  {
    //nothing we can do
  }

  if (status&CM_UARTINT)
  {
    const uint8_t backlog = (sizeof(card->uart_backlog) / sizeof(card->uart_backlog[0]));
    if(card->uart_in - card->uart_out < backlog)
    {
      card->uart_backlog[card->uart_in++%backlog] = snd_cmipci_read_8(card, CM_REG_MPU_PCI);
    }

    if(!(status & (CM_CHINT0|CM_CHINT1)))
      return -1;
  }

  if (status & CM_CHINT0)
    mask |= CM_CH0_INT_EN;
  if (status & CM_CHINT1)
    mask |= CM_CH1_INT_EN;
  snd_cmipci_clear_bit(card, CM_REG_INT_HLDCLR, mask|CM_TDMA_INT_EN); //set to 0 will disable other interrupt. or write 0 and then with full ENs will work.
  snd_cmipci_set_bit(card, CM_REG_INT_HLDCLR, mask); //re-enable hold

  return 1;
}


static int CMI8X38_write_uart(struct audioout_info_s *aui, int reg, int data)
{
  struct cmi8x38_card_s *card=aui->card_private_data;
  int timeout = 10000; // 100ms

  if(!reg) do {
    if (!(0x40 & snd_cmipci_read_8(card, 1 + CM_REG_MPU_PCI))) break;
    pds_delay_10us(1);
  } while (--timeout);

  if(card->chip_version <= 37)
   pds_delay_10us(card->uart_delay);
  snd_cmipci_write_8nv(card, reg + CM_REG_MPU_PCI, (uint8_t)data);
  if(card->chip_version <= 37)
   pds_delay_10us(card->uart_delay);
  return (uint8_t)data;
}

static int CMI8X38_read_uart(struct audioout_info_s *aui, int reg)
{
  struct cmi8x38_card_s *card=aui->card_private_data;
  if(reg || card->uart_out == card->uart_in)
  {
    return snd_cmipci_read_8(card, reg + CM_REG_MPU_PCI);
  }
  else
  {
    const uint8_t backlog = (sizeof(card->uart_backlog) / sizeof(card->uart_backlog[0]));
    return card->uart_backlog[card->uart_out++%backlog];
  }
}

static int CMI8X38_write_fm(struct audioout_info_s *aui, int reg, int data)
{
  struct cmi8x38_card_s *card=aui->card_private_data;
  if(aui->gvars->legacy_fm_disable)
    snd_cmipci_write_8nv(card, reg + CM_REG_FM_PCI, (uint8_t)data);
  else if(!aui->gvars->opl3)
    outb(0x388 + reg,data);
  else return -1;

  return (uint8_t)data;
}

static int CMI8X38_read_fm(struct audioout_info_s *aui, int reg)
{
  struct cmi8x38_card_s *card=aui->card_private_data;
  if(aui->gvars->legacy_fm_disable)
    return snd_cmipci_read_8(card, reg + CM_REG_FM_PCI);
  else if(!aui->gvars->opl3)
    return inb(0x388 + reg);
  else
    return -1;
}

//like SB16
static struct aucards_mixerchan_s cmi8x38_master_vol={AU_MIXCHAN_MASTER,AU_MIXCHANFUNC_VOLUME,2,{{0x30,5,3,0},{0x31,5,3,0}}};
static struct aucards_mixerchan_s cmi8x38_pcm_vol={AU_MIXCHAN_PCM,AU_MIXCHANFUNC_VOLUME,      2,{{0x32,5,3,0},{0x33,5,3,0}}};
static struct aucards_mixerchan_s cmi8x38_synth_vol={AU_MIXCHAN_SYNTH,AU_MIXCHANFUNC_VOLUME,  2,{{0x34,5,3,0},{0x35,5,3,0}}};
static struct aucards_mixerchan_s cmi8x38_cdin_vol={AU_MIXCHAN_CDIN,AU_MIXCHANFUNC_VOLUME,    2,{{0x36,5,3,0},{0x37,5,3,0}}};
static struct aucards_mixerchan_s cmi8x38_linein_vol={AU_MIXCHAN_LINEIN,AU_MIXCHANFUNC_VOLUME,2,{{0x38,5,3,0},{0x39,5,3,0}}};
static struct aucards_mixerchan_s cmi8x38_micin_vol={AU_MIXCHAN_MICIN,AU_MIXCHANFUNC_VOLUME,  1,{{0x3A,5,3,0}}};

const static struct aucards_mixerchan_s *cmi8x38_mixerset[]={
 &cmi8x38_master_vol,
 &cmi8x38_pcm_vol,
 &cmi8x38_synth_vol,
 &cmi8x38_cdin_vol,
 &cmi8x38_linein_vol,
 &cmi8x38_micin_vol,
 NULL
};

struct sndcard_info_s CMI8X38_sndcard_info={
 cmi8x38_shortname,
 0,

 &CMI8X38_adetect,       // only autodetect
 &CMI8X38_start,
 &CMI8X38_stop,
 &CMI8X38_close,
 &CMI8X38_setrate,

 &MDma_writedata,
 &CMI8X38_getbufpos,
 &CMI8X38_clearbuf,
 &CMI8X38_IRQRoutine,

 &CMI8X38_writeMIXER,
 &CMI8X38_readMIXER,
 cmi8x38_mixerset,

 &CMI8X38_write_uart,
 &CMI8X38_read_uart,

 &CMI8X38_write_fm,
 &CMI8X38_read_fm

};


