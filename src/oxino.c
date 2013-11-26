/* oxino.c */

/*
 * 26/11/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * General gem configuration
 */

#include "oxino.h"

extern mrb_value mrb_spi_initialize(mrb_state *mrb,mrb_value self);

extern mrb_value mrb_mrf89_initialize(mrb_state *mrb,mrb_value self);
extern mrb_value mrb_mrf89_set_para(mrb_state *mrb,mrb_value self);
extern mrb_value mrb_mrf89_set_channel(mrb_state *mrb,mrb_value self);
extern mrb_value mrb_mrf89_transmit(mrb_state *mrb,mrb_value self);

static mrb_value mrb_msleep(mrb_state *mrb,mrb_value self)
{
  __u32 msecs;
  
  mrb_get_args(mrb,"i",&msecs);

  usleep(msecs*1000L);
  return self;
}

static mrb_value mrb_loggo(mrb_state *mrb,mrb_value self)
{
  char *s;
  int len;
  time_t t=time(NULL);
  struct tm *tms=localtime(&t);

  len=strftime(s,256,"[%y%m%d.%H%M%S]: ",tms);
  write(2,s,len);  

  mrb_get_args(mrb,"s",&s,&len);
  write(2,s,len);  
  write(2,"\n",1);  
  
  return self;
}

void mrb_olinuxino_gem_init(mrb_state* mrb)
{
  mrb_define_method(mrb,mrb->object_class,"msleep",mrb_msleep,MRB_ARGS_REQ(1));
  mrb_define_method(mrb,mrb->object_class,"loggo",mrb_loggo,MRB_ARGS_REQ(1));
  
  struct RClass *c=mrb_define_class(mrb,"Spi",mrb->object_class);
  MRB_SET_INSTANCE_TT(c,MRB_TT_DATA);

  mrb_define_method(mrb,c,"initialize",mrb_spi_initialize,MRB_ARGS_REQ(2));

  c=mrb_define_class(mrb,"Mrf89",mrb->object_class);
  MRB_SET_INSTANCE_TT(c,MRB_TT_DATA);

  mrb_define_method(mrb,c,"initialize",mrb_mrf89_initialize,MRB_ARGS_REQ(1));
  mrb_define_method(mrb,c,"set_para",mrb_mrf89_set_para,MRB_ARGS_REQ(2));
  mrb_define_method(mrb,c,"set_channel",mrb_mrf89_set_channel,MRB_ARGS_REQ(1));
  mrb_define_method(mrb,c,"transmit",mrb_mrf89_transmit,MRB_ARGS_REQ(1));
}

void mrb_olinuxino_gem_final(mrb_state* mrb)
{
}
