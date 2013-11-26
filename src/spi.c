/* spi.c */

/*
 * 26/11/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * Talk with a SPI bus
 */

#include "oxino.h"
#include "spi.h"

unsigned char one='1';
unsigned char zero='0';

static void spi_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_spi_type={"Spi",spi_free};

mrb_value mrb_spi_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_spi_stc *s=(mrb_spi_stc *)mrb_malloc(mrb,sizeof(mrb_spi_stc));
  char bfr[256];

  mrb_get_args(mrb,"ii",&s->spi_no[0],&s->spi_no[1]);

  sprintf(bfr,"/dev/spidev%d.%d",s->spi_no[0],s->spi_no[1]);
  s->unit=open(bfr,O_RDWR|O_SYNC);
  if(s->unit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",mrb_str_new_cstr(mrb,bfr),mrb_str_new_cstr(mrb,strerror(errno)));

  sprintf(bfr,"/sys/devices/virtual/misc/sun4i-gpio/pin/%s",CSCON_PIN);
  s->cscon_unit=open(bfr,O_WRONLY|O_SYNC);
  if(s->cscon_unit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",mrb_str_new_cstr(mrb,bfr),mrb_str_new_cstr(mrb,strerror(errno)));
  write(s->cscon_unit,&one,1); // must be high, except when sending  

  sprintf(bfr,"/sys/devices/virtual/misc/sun4i-gpio/pin/%s",IRQ1_PIN);
  s->irq_units[0]=open(bfr,O_WRONLY|O_SYNC);
  if(s->irq_units[0]<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",mrb_str_new_cstr(mrb,bfr),mrb_str_new_cstr(mrb,strerror(errno)));
  sprintf(bfr,"/sys/devices/virtual/misc/sun4i-gpio/pin/%s",IRQ2_PIN);
  s->irq_units[1]=open(bfr,O_WRONLY|O_SYNC);
  if(s->irq_units[1]<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",mrb_str_new_cstr(mrb,bfr),mrb_str_new_cstr(mrb,strerror(errno)));

  DATA_TYPE(self)=&mrb_spi_type;
  DATA_PTR(self)=s;
  return self;
}

static void spi_free(mrb_state *mrb, void *p)
{
  mrb_spi_stc *s=(mrb_spi_stc *)p;
  
  close(s->unit);
  
  write(s->cscon_unit,&one,1);
  close(s->cscon_unit);
  
  close(s->irq_units[0]);
  close(s->irq_units[1]);
  
  mrb_free(mrb,p);
}
