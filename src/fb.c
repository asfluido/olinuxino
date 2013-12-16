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
#include <pthread.h>
#include <poll.h>

#define ISBIT(data,pos) ((data[(pos>>3)]>>(pos&7))&1)
#define POLL_TIMEOUT 250

typedef struct mrb_fb
{
  int fbunit,tsunit;
  __u8 exit_thread;
  pthread_t ts_thr;
  mrb_bool touching;
  __s32 p_x,p_y;
} mrb_fb_stc;

static void fb_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_fb_type={"Fb",fb_free};
static void *ts(void *arg);

mrb_value mrb_fb_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=(mrb_fb_stc *)mrb_malloc(mrb,sizeof(mrb_fb_stc));
  mrb_value v1,v2;
  int version;
  __u8 vbits[EV_CNT>>3],absbits[ABS_CNT>>3],keybits[KEY_CNT>>3];

  mrb_get_args(mrb,"SS",&v1,&v2);

  s->fbunit=open(RSTRING_PTR(v1),O_RDWR);
  if(s->fbunit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v1,mrb_str_new_cstr(mrb,strerror(errno)));
  
  s->tsunit=open(RSTRING_PTR(v2),O_RDWR|O_NONBLOCK);
  if(s->tsunit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(ioctl(s->tsunit,EVIOCGVERSION,&version)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad file (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(ioctl(s->tsunit,EVIOCGBIT(0,EV_CNT>>3),vbits)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad EVIOCGBIT (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(!(ISBIT(vbits,EV_ABS)) || !(ISBIT(vbits,EV_KEY)))
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: does not support ABS/KEY\n",v2);
  
  if(ioctl(s->tsunit,EVIOCGBIT(EV_ABS,ABS_CNT>>3),absbits)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad EVIOCGBIT/2 (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));
  
  if(!(ISBIT(absbits,ABS_MT_POSITION_X)) || !(ISBIT(absbits,ABS_MT_POSITION_Y)))
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: does not support ABS X/Y\n",v2);  

  if(ioctl(s->tsunit,EVIOCGBIT(EV_KEY,KEY_CNT>>3),keybits)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad EVIOCGBIT/3 (%S)\n",v2,mrb_str_new_cstr(mrb,strerror(errno)));

  if(!(ISBIT(keybits,BTN_TOUCH)))
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: does not support STYLUS\n",v2);  

  fprintf(stderr,"EV version <%x>\n",version);

  s->exit_thread=0;
  int ret=pthread_create(&s->ts_thr,NULL,ts,s);
  if(ret)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error starting ts thread (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));

  DATA_TYPE(self)=&mrb_fb_type;
  DATA_PTR(self)=s;
  return self;
}

mrb_value mrb_fb_status(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);
  mrb_value to_ret=mrb_ary_new(mrb);

  mrb_ary_push(mrb,to_ret,mrb_bool_value(s->touching));
  mrb_ary_push(mrb,to_ret,mrb_fixnum_value(s->p_x));
  mrb_ary_push(mrb,to_ret,mrb_fixnum_value(s->p_y));

  return to_ret;
}

static void fb_free(mrb_state *mrb, void *p)
{
  mrb_fb_stc *s=(mrb_fb_stc *)p;

  s->exit_thread=1;
  pthread_join(s->ts_thr,NULL);
  
  close(s->fbunit);
  close(s->tsunit);
  
  mrb_free(mrb,p);
}

static void *ts(void *arg)
{
  mrb_fb_stc *s=(mrb_fb_stc *)arg;
  struct pollfd pfd={s->tsunit,POLLIN|POLLERR|POLLHUP,0};
  struct input_event ev;

  while(!s->exit_thread)
  {
    if(poll(&pfd,1,POLL_TIMEOUT)<=0)
      continue;
    read(s->tsunit,&ev,sizeof(struct input_event));
//    fprintf(stderr,"!! type %2.2x code %2.2x value %2.2x\n",ev.type,ev.code,ev.value);
    if(ev.type==EV_ABS)
    {
      if(ev.code==ABS_MT_TOUCH_MAJOR)
      {
	if(ev.value>0)
	{
	  s->touching=TRUE;
	}
	else
	{
	  s->touching=FALSE;
	}
      }
      else if(ev.code==ABS_MT_POSITION_X)
	s->p_x=ev.value;
      else if(ev.code==ABS_MT_POSITION_Y)
	s->p_y=ev.value;
    }
  }

  return NULL;
}
