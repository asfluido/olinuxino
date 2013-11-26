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

typedef struct mrb_mrf89
{
  mrb_spi_stc *spi;
} mrb_mrf89_stc;

static void mrf89_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_mrf89_type={"Mrf89",mrf89_free};

mrb_value mrb_mrf89_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_mrf89_stc *s=(mrb_mrf89_stc *)mrb_malloc(mrb,sizeof(mrb_mrf89_stc));
  mrb_value v;

  mrb_get_args(mrb,"o",&v);
  s->spi=DATA_PTR(v);

/*
 * Reset & initialize
 */

  char bfr[256];

  fprintf(stderr,"Opened mrf89 on spi %d/%d\nResetting...",s->spi->spi_no[0],s->spi->spi_no[1]);
  sprintf(bfr,"/sys/devices/virtual/misc/sun4i-gpio/pin/%s",RESET_PIN);
  
  int r_unit=open(bfr,O_WRONLY|O_SYNC);
  int i;
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

  fprintf(stderr,"\nWr res %d\n",i);

  DATA_TYPE(self)=&mrb_mrf89_type;
  DATA_PTR(self)=s;
  return self;
}

static void mrf89_free(mrb_state *mrb, void *p)
{
//  mrb_mrf89_stc *s=(mrb_mrf89_stc *)p;
  
  mrb_free(mrb,p);
}
