/* mainloop.c */

/*
 * 19/12/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * A general-usage main loop
 */

#include "oxino.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_EVENTS_AT_A_TIME 10

typedef struct
{
  mrb_value *obj;
  int unit;
  char *cback;
} poll_unit_stc;

typedef struct mrb_mainloop
{
  mrb_state *mrb_orig;
  int timeout,n_poll_units;
  __u8 exit_thread;
  pthread_t ml_thr;
  poll_unit_stc *poll_units;
} mrb_mainloop_stc;

static void mainloop_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_mainloop_type={"Mainloop",mainloop_free};
static void *ml(void *arg);

mrb_value mrb_mainloop_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_mainloop_stc *s=(mrb_mainloop_stc *)mrb_malloc(mrb,sizeof(mrb_mainloop_stc));

  s->mrb_orig=mrb;
  
  mrb_get_args(mrb,"i",&s->timeout);

  s->exit_thread=0;
  int ret=pthread_create(&s->ml_thr,NULL,ml,s);
  if(ret)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error starting main loop thread (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));

  s->n_poll_units=0;
  s->poll_units=NULL;
  
  DATA_TYPE(self)=&mrb_mainloop_type;
  DATA_PTR(self)=s;
  return self;
}

mrb_value mrb_mainloop_add_unit(mrb_state *mrb,mrb_value self)
{
  mrb_mainloop_stc *s=DATA_PTR(self);

  s->poll_units=realloc(s->poll_units,sizeof(poll_unit_stc)*(s->n_poll_units+1));
  poll_unit_stc *pu=s->poll_units+s->n_poll_units;
  s->n_poll_units++;

  mrb_value v;
  
  mrb_get_args(mrb,"oiS",&pu->obj,&pu->unit,&v);

  pu->cback=strdup(RSTRING_PTR(v));
  
  return self;
}

static void mainloop_free(mrb_state *mrb, void *p)
{
  mrb_mainloop_stc *s=(mrb_mainloop_stc *)p;
  int i;
  
  s->exit_thread=1;
  pthread_join(s->ml_thr,NULL);

  for(i=0;i<s->n_poll_units;i++)
    free(s->poll_units[i].cback);
  
  free(s->poll_units);
  
  mrb_free(mrb,p);
}

static void *ml(void *arg)
{
  mrb_mainloop_stc *s=(mrb_mainloop_stc *)arg;
  mrb_state *mrb=s->mrb_orig;
  struct epoll_event ev,events[MAX_EVENTS_AT_A_TIME];
  int epollfd=epoll_create1(0),res,res2,i;

  if(epollfd<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error creating epollfd (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));

  int cur_n_poll_units=0;
  poll_unit_stc *pu=NULL;
  __u8 bfr[256];
  
  while(!s->exit_thread)
  {
    if(cur_n_poll_units!=s->n_poll_units)
      for(;cur_n_poll_units<s->n_poll_units;cur_n_poll_units++)
      {
	pu=s->poll_units+cur_n_poll_units;
	fprintf(stderr,"Adding %d/%d\n",cur_n_poll_units,pu->unit);
	ev.events=EPOLLIN;
	ev.data.u32=cur_n_poll_units;
	res=epoll_ctl(epollfd,EPOLL_CTL_ADD,pu->unit,&ev);
	if(res<0)
	  mrb_raisef(mrb,E_TYPE_ERROR,"Error in epoll_ctl (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));
      }
    
    res=epoll_wait(epollfd,events,MAX_EVENTS_AT_A_TIME,s->timeout);
    if(res==0)
      continue;
    if(res<0)
      mrb_raisef(mrb,E_TYPE_ERROR,"Error in epoll_wait (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));
    
//    fprintf(stderr,"res -> %d (%s)\n",res,strerror(errno));
    for(i=0;i<res;i++)
    {
      pu=s->poll_units+events[i].data.u32;
      res2=recv(pu->unit,bfr,256,0);
      if(res<0)
	mrb_raisef(mrb,E_TYPE_ERROR,"Error in recv (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));
      mrb_funcall(mrb,*(pu->obj),(char *)pu->cback,1,mrb_str_new(mrb,(char *)bfr,res));
    }
  }

  return NULL;
}
