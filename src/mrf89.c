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
#include "spi.h"

#define RESET_PIN "pg3"

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
  fprintf(stderr,"Opened mrf89 on spi %d%d\n",s->spi->spi_no[0],s->spi->spi_no[1]);

  DATA_TYPE(self)=&mrb_mrf89_type;
  DATA_PTR(self)=s;
  return self;
}

static void mrf89_free(mrb_state *mrb, void *p)
{
//  mrb_mrf89_stc *s=(mrb_mrf89_stc *)p;
  
  mrb_free(mrb,p);
}
