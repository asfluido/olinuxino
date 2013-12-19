/* led.c */

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
#include <pthread.h>

typedef struct mrb_mainloop
{
  mrb_state *mrb_orig;
  int max_events,timeout;
  __u8 exit_thread;
  pthread_t ml_thr;
} mrb_mainloop_stc;

static void mainloop_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_mainloop_type={"Mainloop",mainloop_free};
static void *ml(void *arg);

mrb_value mrb_mainloop_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_mainloop_stc *s=(mrb_mainloop_stc *)mrb_malloc(mrb,sizeof(mrb_mainloop_stc));

  s->mrb_orig=mrb;
  
  mrb_get_args(mrb,"ii",&s->max_events,&s->timeout);

  s->exit_thread=0;
  int ret=pthread_create(&s->ml_thr,NULL,ml,s);
  if(ret)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error starting main loop thread (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));

  DATA_TYPE(self)=&mrb_mainloop_type;
  DATA_PTR(self)=s;
  return self;
}

static void mainloop_free(mrb_state *mrb, void *p)
{
  mrb_mainloop_stc *s=(mrb_mainloop_stc *)p;

  s->exit_thread=1;
  pthread_join(s->ml_thr,NULL);

  mrb_free(mrb,p);
}

static void *ml(void *arg)
{
  mrb_mainloop_stc *s=(mrb_mainloop_stc *)arg;
  mrb_state *mrb=s->mrb_orig;
//  struct epoll_event events[s->max_events];
  int epollfd=epoll_create(s->max_events);

  if(epollfd<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error creating epollfd (%S)\n",mrb_str_new_cstr(mrb,strerror(errno)));

  while(!s->exit_thread)
  {
    usleep(s->timeout);
  }

  return NULL;
}
