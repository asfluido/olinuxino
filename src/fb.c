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
  __u32 vbits,absbits;

  mrb_get_args(mrb,"SS",&v1,&v2);

  s->fbunit=open(RSTRING_PTR(v1),O_RDWR);
  if(s->fbunit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v1,mrb_str_new_cstr(mrb,strerror(errno)));
  
  s->tsunit=open(RSTRING_PTR(v2),O_RDWR|O_NONBLOCK);
  if(s->tsunit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(ioctl(s->tsunit,EVIOCGVERSION,&version)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad file (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(ioctl(s->tsunit,EVIOCGBIT(0,4),&vbits)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad EVIOCGBIT (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(!((vbits>>EV_ABS)&1) || !((vbits>>EV_KEY)&1))
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: does not support ABS/KEY\n",v2);
  
  if(ioctl(s->tsunit,EVIOCGBIT(EV_ABS,4),&absbits)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad EVIOCGBIT/2 (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(!((absbits>>ABS_X)&1) || !((absbits>>ABS_Y)&1))
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: does not support ABS/KEY\n",v2);
  
  fprintf(stderr,"EV version <%d> bits <%4.4x> absbits <%4.4x>\n",version,vbits,absbits);
     
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


