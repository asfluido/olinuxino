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

#include <sys/mman.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <pthread.h>
#include <poll.h>

#define ISBIT(data,pos) ((data[(pos>>3)]>>(pos&7))&1)
#define POLL_TIMEOUT 250

#define CALIBDATA_FILE "/.fb_calibdata"

typedef struct mrb_fb
{
  int fbunit,tsunit;
  __u8 exit_thread;
  pthread_t ts_thr;
  mrb_bool touching;
  __s32 p_x,p_y;
  struct fb_fix_screeninfo fix;
  struct fb_var_screeninfo var;
  __u32 *fb,**lines,screen_size;
  char *calibdata_path;
  mrb_float *calibdata;
} mrb_fb_stc;

static void fb_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_fb_type={"Fb",fb_free};
static inline void paint_pixel(mrb_fb_stc *s,int x,int y,__u32 col);
static inline void fill(mrb_fb_stc *s,__u32 col);
static void *ts(void *arg);

mrb_value mrb_fb_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=(mrb_fb_stc *)mrb_malloc(mrb,sizeof(mrb_fb_stc));
  mrb_value v1,v2;
  int version,i;
  __u8 vbits[EV_CNT>>3],absbits[ABS_CNT>>3],keybits[KEY_CNT>>3];

  mrb_get_args(mrb,"SS",&v1,&v2);
  
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

  s->fbunit=open(RSTRING_PTR(v1),O_RDWR);
  if(s->fbunit<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error opening %S (%S)\n",v1,mrb_str_new_cstr(mrb,strerror(errno)));

  if(ioctl(s->fbunit,FBIOGET_FSCREENINFO,&s->fix)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad FBIOGET_FSCREENINFO (%S)\n",v1,mrb_str_new_cstr(mrb,strerror(errno)));
  if(ioctl(s->fbunit,FBIOGET_VSCREENINFO,&s->var)<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: bad FBIOGET_VSCREENINFO (%S)\n",v1,mrb_str_new_cstr(mrb,strerror(errno)));

  if(s->var.bits_per_pixel!=32)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: only support 32bpp (found %S)\n",v1,mrb_fixnum_value(s->var.bits_per_pixel));

  s->screen_size=s->var.xres*s->var.yres;
  
  s->fb=mmap(NULL,s->fix.smem_len,PROT_READ|PROT_WRITE,MAP_FILE|MAP_SHARED,s->fbunit,0);
  if(s->fb==(__u32 *)-1)
    mrb_raisef(mrb,E_TYPE_ERROR,"%S: error in fb mmap (%S)\n",v1,mrb_str_new_cstr(mrb,strerror(errno)));

  s->lines=malloc(sizeof(__u32 *)*s->var.yres_virtual);
  for(i=0;i<s->var.yres_virtual;i++)
    s->lines[i]=s->fb+s->var.xres_virtual*i;
  
  fprintf(stderr,"FB: [%s] (%dx%d, %d bpp)\n",s->fix.id,s->var.xres,s->var.yres,s->var.bits_per_pixel);

/*
 * load calibdata if present
 */

  s->calibdata_path=strdup(strcat(getenv("HOME"),CALIBDATA_FILE));
  
  i=open(s->calibdata_path,O_RDONLY);
  if(i>0)
  {
    s->calibdata=malloc(sizeof(mrb_float)*4);
    read(i,s->calibdata,sizeof(mrb_float)*4);
    close(i);
  }
  else
    s->calibdata=NULL;

/*
 * Disable cursor blink
 */

  i=open("/sys/class/graphics/fbcon/cursor_blink",O_WRONLY);
  write(i,"0",1);
  close(i);

  fill(s,0);
  
  s->exit_thread=0;
  int ret=pthread_create(&s->ts_thr,NULL,ts,s);
  if(ret)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error starting ts thread (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));

  DATA_TYPE(self)=&mrb_fb_type;
  DATA_PTR(self)=s;
  return self;
}

mrb_value mrb_fb_size(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);
  mrb_value to_ret=mrb_ary_new(mrb);

  mrb_ary_push(mrb,to_ret,mrb_fixnum_value(s->var.xres));
  mrb_ary_push(mrb,to_ret,mrb_fixnum_value(s->var.yres));

  return to_ret;
}
  
mrb_value mrb_fb_status(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);
  mrb_value to_ret=mrb_ary_new(mrb);
  mrb_bool converted;
  
  mrb_get_args(mrb,"b",&converted);

  if(converted && !s->calibdata)
    return mrb_false_value();  

  mrb_ary_push(mrb,to_ret,mrb_bool_value(s->touching));
  mrb_ary_push(mrb,to_ret,mrb_fixnum_value(converted ? (int)((s->p_x-s->calibdata[0])*s->calibdata[1]) : s->p_x));
  mrb_ary_push(mrb,to_ret,mrb_fixnum_value(converted ? (int)((s->p_y-s->calibdata[2])*s->calibdata[3]) : s->p_y));

  return to_ret;
}

mrb_value mrb_fb_fill(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);
  __u32 col;

  mrb_get_args(mrb,"i",&col);

  fill(s,col);
  
  return self;
}

mrb_value mrb_fb_line(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);
  int xf,yf,xt,yt,i,xd,yd;
  __u32 col;
  float fact;

  mrb_get_args(mrb,"iiiii",&xf,&yf,&xt,&yt,&col);
  
  col|=0xff000000;  

  xd=abs(xt-xf);
  yd=abs(yt-yf);

  if(xd>yd)
  {
    fact=yd/(float)xd;
    if(xt>xf)
    {
      for(i=xf;i<=xt;i++)
	paint_pixel(s,i,(int)(yf+fact*i),col);
    }
    else
    {
      for(i=xt;i<=xf;i++)
	paint_pixel(s,i,(int)(yt-fact*i),col);
    }
  }
  else
  {
    fact=xd/(float)yd;
    if(yt>yf)
    {
      for(i=yf;i<=yt;i++)
	paint_pixel(s,(int)(xf+fact*i),i,col);
    }
    else
    {
      for(i=yt;i<=yf;i++)
	paint_pixel(s,(int)(xt-fact*i),i,col);
    }
  }

  return self;
}

mrb_value mrb_fb_rect(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);
  int xf,yf,xt,yt,i,x,y;
  __u32 col,*ptr;

  mrb_get_args(mrb,"iiiii",&xf,&yf,&xt,&yt,&col);

  if(xf==xt && yf==yt)
    return self;  

  if(xf>xt)
  {
    i=xf;
    xf=xt;
    xt=i;
  }
  if(yf>yt)
  {
    i=yf;
    yf=yt;
    yt=i;
  }

  if(xf<0)
    xf=0;
  if(xf>=s->var.xres)
    xf=s->var.xres-1;
  if(yf<0)
    yf=0;
  if(yf>=s->var.yres)
    yf=s->var.yres-1;

  for(y=yf;y<yt;y++)
  {
    ptr=s->lines[y]+xf;
    for(x=xf;x<xt;x++)
      *ptr++=col;
  }
  
  return self;
}    

mrb_value mrb_fb_save_calibdata(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);
  mrb_float v[4];
  
  mrb_get_args(mrb,"ffff",&v[0],&v[1],&v[2],&v[3]);
  
  int unit=open(s->calibdata_path,O_WRONLY|O_TRUNC|O_CREAT,0600);
  
  fprintf(stderr,"%s opened to unit %d (%s)\n",s->calibdata_path,unit,strerror(errno));
  
  write(unit,v,sizeof(mrb_float)*4);
  close(unit);

  s->calibdata=realloc(s->calibdata,sizeof(mrb_float)*4);
  memcpy(s->calibdata,v,sizeof(mrb_float)*4);

  return self;
}    

mrb_value mrb_fb_calibdata(mrb_state *mrb,mrb_value self)
{
  mrb_fb_stc *s=DATA_PTR(self);

  if(s->calibdata==NULL)
    return mrb_false_value();  
    
  mrb_value to_ret=mrb_ary_new(mrb);
  mrb_ary_push(mrb,to_ret,mrb_float_value(mrb,s->calibdata[0]));
  mrb_ary_push(mrb,to_ret,mrb_float_value(mrb,s->calibdata[1]));
  mrb_ary_push(mrb,to_ret,mrb_float_value(mrb,s->calibdata[2]));
  mrb_ary_push(mrb,to_ret,mrb_float_value(mrb,s->calibdata[3]));  

  return to_ret;
}

static void fb_free(mrb_state *mrb, void *p)
{
  mrb_fb_stc *s=(mrb_fb_stc *)p;

  s->exit_thread=1;
  pthread_join(s->ts_thr,NULL);

  fill(s,0);

/*
 * Enable cursor blink
 */

  int i=open("/sys/class/graphics/fbcon/cursor_blink",O_WRONLY);
  write(i,"1",1);
  close(i);

  munmap(s->fb,s->fix.smem_len);
  free(s->lines);

  close(s->fbunit);
  close(s->tsunit);

  if(s->calibdata)
    free(s->calibdata);
  
  mrb_free(mrb,p);
}

static inline void paint_pixel(mrb_fb_stc *s,int x,int y,__u32 col)
{
//  fprintf(stderr,"[%d,%d,%x]",x,y,col);
  
  if(x<0 || x>=s->var.xres || y<0 || y>=s->var.yres)
    return;

  s->lines[y][x]=col;  
}

static inline void fill(mrb_fb_stc *s,__u32 col)
{
  __u32 *ptr=s->fb;
  int i;

  for(i=s->screen_size;i>0;i--)
    *ptr++=col;
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
