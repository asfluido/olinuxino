/* fb.c */

/*
 * 12/12/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * Framebuffer + touchscreen
 */

#include "oxino.h"

#include <linux/input.h>
#include <linux/fb.h>

typedef struct mrb_fb
{
  int fbunit,tsunit;
} mrb_fb_stc;

static void fb_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_fb_type={"Fb",fb_free};

mrb_value mrb_fb_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=(mrb_fb_stc *)mrb_malloc(mrb,sizeof(mrb_fb_stc));
  mrb_value v1,v2;
  int version;

  mrb_get_args(mrb,"SS",&v1,&v2);

  s->fbunit=open(RSTRING_PTR(v1),O_RDWR);
  if(s->fbunit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v1,mrb_str_new_cstr(mrb,strerror(errno)));
  
  s->tsunit=open(RSTRING_PTR(v2),O_RDWR|O_NONBLOCK);
  if(s->tsunit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));

  if(ioctl(s->tsunit,EVIOCGVERSION,&version)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad file (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));

  fprintf(stderr,"EV version <%d>\n",version);

  DATA_TYPE(self)=&mrb_fb_type;
  DATA_PTR(self)=s;
  return self;
}

static void fb_free(mrb_state *mrb, void *p)
{
  mrb_fb_stc *s=(mrb_fb_stc *)p;
  
  close(s->fbunit);
  close(s->tsunit);
  
  mrb_free(mrb,p);
}


