/* mrf89.c */

/*
 * 26/11/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * Talks to an Olimex MOD-MRF89-868 module
 */

#include "oxino.h"
#include "mrf89.h"

#define RESET_PIN "pg3"
#define RESET_SLEEP1 100 
#define RESET_SLEEP2 5000
#define TSHOP 350
#define TSTWF 500

typedef struct mrb_mrf89
{
  mrb_spi_stc *spi;
  __u8 pvalues[CHANNEL_NUM],svalues[CHANNEL_NUM];
} mrb_mrf89_stc;

static void mrf89_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_mrf89_type={"Mrf89",mrf89_free};

mrb_value mrb_mrf89_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=(mrb_mrf89_stc *)mrb_malloc(mrb,sizeof(mrb_mrf89_stc));
  mrb_value v;

  mrb_get_args(mrb,"o",&v);
  s->spi=DATA_PTR(v);

  int i,freq,compare;
  
  for(i=0;i<CHANNEL_NUM;i++)
  {
    freq=START_FREQUENCY+(i+1)*CHANNEL_SPACING;
    compare=(int)((freq*808)/(FXTAL*9000));
    s->pvalues[i]=(__u8)((compare-75)/76+1);
    s->svalues[i]=(__u8)(compare-75*(s->pvalues[i]+1));
  }

/*
 * Reset & initialize
 */

  char bfr[256];

  fprintf(stderr,"Opened mrf89 on spi %d/%d\nResetting...",s->spi->spi_no[0],s->spi->spi_no[1]);
  sprintf(bfr,"/sys/devices/virtual/misc/sun4i-gpio/pin/%s",RESET_PIN);
  
  int r_unit=open(bfr,O_WRONLY|O_SYNC);
  __u8 c[2*N_REG],*ptr;  
  
  if(r_unit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",mrb_str_new_cstr(mrb,bfr),mrb_str_new_cstr(mrb,strerror(errno)));

  write(r_unit,&one,1);
  usleep(RESET_SLEEP1);
  write(r_unit,&zero,1);
  usleep(RESET_SLEEP2);
  close(r_unit);

  fputs("Done\nReading configuration...",stderr);

  write(s->spi->cscon_unit,&zero,1); // must be high, except when sending
  for(i=0;i<N_REG;i++)
  {
    c[0]=(i<<1)|0x40;
    write(s->spi->unit,c,1);
    read(s->spi->unit,c,1);
    fprintf(stderr,"%2.2x -> %2.2x ",i,c[0]);
  }
  write(s->spi->cscon_unit,&one,1); // must be high, except when sending

  fputs("Done\nWriting configuration...",stderr);
  
  for(ptr=c,i=0;i<N_REG;i++)
  {
    *ptr++=i<<1;
    *ptr++=base_confvals[i];
    fprintf(stderr,"%d (%.2x/%.2x) ",i,ptr[-2],ptr[-1]);
  }
  i=spi_low_write(s->spi,c,2*N_REG,1);

  fprintf(stderr," Done.\nWr res %d\n",i);

  DATA_TYPE(self)=&mrb_mrf89_type;
  DATA_PTR(self)=s;
  return self;
}

mrb_value mrb_mrf89_set_para(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=DATA_PTR(self);
  int paraid,paraval;
  
  mrb_get_args(mrb,"ii",&paraid,&paraval);

  if(paraid<0 || paraid>=N_REG)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad para id (%S)!\n",mrb_str_new_cstr(mrb,__func__),mrb_fixnum_value(paraid));
  if(paraval<0 || paraval>0xff)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad para val (%S)!\n",mrb_str_new_cstr(mrb,__func__),mrb_fixnum_value(paraval));
  
  __u8 c[2];

  c[0]=paraid<<1;
  c[1]=paraval;
  
  write(s->spi->cscon_unit,&zero,1); // must be high, except when sending
  write(s->spi->unit,c,2);
  write(s->spi->cscon_unit,&one,1); // must be high, except when sending

  return self;
}

mrb_value mrb_mrf89_set_channel(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=DATA_PTR(self);
  int chno;
  
  mrb_get_args(mrb,"i",&chno);
  if(chno<0 || chno>=CHANNEL_NUM)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad channel (%S)!\n",mrb_str_new_cstr(mrb,__func__),mrb_fixnum_value(chno));

  write(s->spi->cscon_unit,&zero,1); // must be high, except when sending
  
  __u8 c[18]=
    {
      R1CREG<<1,RVALUE,
      P1CREG<<1,s->pvalues[chno],
      S1CREG<<1,s->svalues[chno],
      GCONREG<<1,CHIPMODE_STBYMODE|FREQ_BAND|VCO_TRIM_11,
      GCONREG<<1,CHIPMODE_FSMODE|FREQ_BAND|VCO_TRIM_11,
      GCONREG<<1,CHIPMODE_FSMODE|FREQ_BAND|VCO_TRIM_11, // 2 times?!?
      FTPRIREG<<1,0xf,
      GCONREG<<1,CHIPMODE_STBYMODE|FREQ_BAND|VCO_TRIM_11,
      GCONREG<<1,CHIPMODE_RX|FREQ_BAND|VCO_TRIM_11,
    };
  
  write(s->spi->unit,c,18);
  
  write(s->spi->cscon_unit,&one,1); // must be high, except when sending
  
  usleep(TSHOP);  
  return self;
}

mrb_value mrb_mrf89_transmit(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=DATA_PTR(self);
  char *stuff;
  int slen;

  mrb_get_args(mrb,"s",&stuff,&slen);
  
  __u8 b[slen+1>6 ? slen+1 : 6];

  b[0]=GCONREG<<1;
  b[1]=CHIPMODE_STBYMODE|FREQ_BAND|VCO_TRIM_11;
  b[2]=FTXRXIREG<<1;
  b[3]=0x9;
  b[4]=FCRCREG<<1;
  b[5]=0;

  write(s->spi->cscon_unit,&zero,1);
  write(s->spi->unit,b,6);
  write(s->spi->cscon_unit,&one,1);
  
  write(s->spi->irq_units[0],&zero,1);
  
  int i;

  b[0]=slen;
  memcpy(b+1,stuff,slen);
  for(i=0;i<=slen;i++)
    write(s->spi->unit,b+i,1);  

  b[0]=GCONREG<<1;
  b[1]=CHIPMODE_TX|FREQ_BAND|VCO_TRIM_11;

  write(s->spi->cscon_unit,&zero,1);
  write(s->spi->unit,b,2);
  write(s->spi->cscon_unit,&one,1);

  while(1)
  {
    lseek(s->spi->irq_units[0],0,SEEK_SET);
    read(s->spi->irq_units[0],b,1);
    if(b[0]=='1')
      break;
    fprintf(stderr,"[%2.2x]",b[0]);
    usleep(5000);
  }

  b[0]=GCONREG<<1;
  b[1]=CHIPMODE_STBYMODE|FREQ_BAND|VCO_TRIM_11;
  write(s->spi->cscon_unit,&zero,1);
  write(s->spi->unit,b,2);
  write(s->spi->cscon_unit,&one,1);
  
  return self;
}

mrb_value mrb_mrf89_receive(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=DATA_PTR(self);
    __u8 b[64];

  write(s->spi->irq_units[0],&zero,1);
  write(s->spi->irq_units[1],&zero,1);

  b[0]=GCONREG<<1;
  b[1]=CHIPMODE_STBYMODE|FREQ_BAND|VCO_TRIM_11;
  b[2]=FTXRXIREG<<1;
  b[3]=0x80;
  b[4]=FTPRIREG<<1;
  b[5]=1;
  b[6]=GCONREG<<1;
  b[7]=CHIPMODE_RX|FREQ_BAND|VCO_TRIM_11;
  
  write(s->spi->cscon_unit,&zero,1);
  write(s->spi->unit,b,8);
  write(s->spi->cscon_unit,&one,1);
  
  usleep(TSTWF);

  b[0]=FTPRIREG<<1;
  b[1]=0xcf;
  
  write(s->spi->cscon_unit,&zero,1);
  write(s->spi->unit,b,2);
  write(s->spi->cscon_unit,&one,1);

  while(1)
  {
    lseek(s->spi->irq_units[0],0,SEEK_SET);
    read(s->spi->irq_units[0],b,1);
    lseek(s->spi->irq_units[1],0,SEEK_SET);
    read(s->spi->irq_units[1],b+1,1);
    if(b[1]=='1')
      break;
//    fprintf(stderr,"[%2.2x %2.2x]",b[0],b[1]);
    usleep(1000);
  }

  int i;

  for(i=0;i<64;i++)
    read(s->spi->unit,b+i,1);
  
//  low_read(s,b,64,0);

  return mrb_str_new(mrb,(char *)b,64);
}

mrb_value mrb_mrf89_read(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=DATA_PTR(self);
  int nch;
  mrb_value cscon;

  mrb_get_args(mrb,"io",&nch,&cscon);

  __u8 b[nch];
  int ret=spi_low_read(s->spi,b,nch,mrb_bool(cscon) ? 1 : 0);
  if(ret!=nch)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: read %S instead of %S! (%S)",mrb_str_new_cstr(mrb,__func__),
	       mrb_fixnum_value(ret),mrb_fixnum_value(nch),mrb_str_new_cstr(mrb,strerror(errno)));

  return mrb_str_new(mrb,(char *)b,nch);
}

mrb_value mrb_mrf89_req_para(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=DATA_PTR(self);
  int paraid;

  mrb_get_args(mrb,"i",&paraid);

  if(paraid<0 || paraid>=N_REG)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad para id! (%S)",mrb_str_new_cstr(mrb,__func__),
	       mrb_fixnum_value(paraid));
  
  __u8 c=(paraid<<1)|0x40,c2;
  write(s->spi->cscon_unit,&zero,1);
  write(s->spi->unit,&c,1);
  read(s->spi->unit,&c2,1);
  write(s->spi->cscon_unit,&one,1);
  
  return mrb_fixnum_value(c2);
}

mrb_value mrb_mrf89_get_irqs(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=DATA_PTR(self);
  mrb_value to_ret=mrb_ary_new(mrb);
  unsigned char c;
  int i;
  
  for(i=0;i<2;i++)
  {
    lseek(s->spi->irq_units[i],0,SEEK_SET);
    read(s->spi->irq_units[i],&c,1);
    mrb_ary_push(mrb,to_ret,mrb_fixnum_value(c));
  }
  
  return to_ret;
}
  
static void mrf89_free(mrb_state *mrb, void *p)
{
//  mrb_mrf89_stc *s=(mrb_mrf89_stc *)p;
  
  mrb_free(mrb,p);
}
