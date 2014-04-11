/* led.c */

/*
 * 12/12/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * Manage onboard led
 */

#include "oxino.h"

typedef struct mrb_led
{
  int unit;
} mrb_led_stc;

static void led_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_led_type={"Led",led_free};

mrb_value mrb_led_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_led_stc *s=(mrb_led_stc *)mrb_malloc(mrb,sizeof(mrb_led_stc));
  mrb_value v;
  
  mrb_get_args(mrb,"S",&v);
  
  s->unit=open(RSTRING_PTR(v),O_WRONLY|O_SYNC);
  if(s->unit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v,mrb_str_new_cstr(mrb,strerror(errno)));
  
  DATA_TYPE(self)=&mrb_led_type;
  DATA_PTR(self)=s;
  return self;
}

mrb_value mrb_led_on(mrb_state *mrb,mrb_value self)
{
  mrb_led_stc *s=DATA_PTR(self);

  write(s->unit,&one,1);

  return self;
}

mrb_value mrb_led_off(mrb_state *mrb,mrb_value self)
{
  mrb_led_stc *s=DATA_PTR(self);

  write(s->unit,&zero,1);

  return self;
}

static void led_free(mrb_state *mrb, void *p)
{
  mrb_led_stc *s=(mrb_led_stc *)p;

  close(s->unit);
  
  mrb_free(mrb,p);
}
