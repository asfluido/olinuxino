/* osock.c */

/*
 * 19/12/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * One named unix socket
 */

#include "oxino.h"
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct mrb_osock
{
  char *name;
  int unit;
} mrb_osock_stc;

static void osock_free(mrb_state *mrb, void *p);
static struct mrb_data_type mrb_osock_type={"Osock",osock_free};

mrb_value mrb_osock_initialize(mrb_state *mrb,mrb_value self)
{
  mrb_osock_stc *s=(mrb_osock_stc *)mrb_malloc(mrb,sizeof(mrb_osock_stc));
  mrb_value v;
  struct sockaddr_un addr;
  
  mrb_get_args(mrb,"S",&v);

  s->name=strdup(RSTRING_PTR(v));
  s->unit=socket(AF_UNIX,SOCK_DGRAM,0);

  bzero(&addr,sizeof(struct sockaddr_un));
  addr.sun_family=AF_UNIX;
  strncpy(addr.sun_path,s->name,sizeof(addr.sun_path)-1);

  int res=bind(s->unit,(struct sockaddr *)&addr,sizeof(struct sockaddr_un));
  if(res<0)
    mrb_raisef(mrb,E_TYPE_ERROR,"Error binding %S (%S)\n",v,mrb_str_new_cstr(mrb,strerror(errno)));
  
  DATA_TYPE(self)=&mrb_osock_type;
  DATA_PTR(self)=s;
  return self;
}

mrb_value mrb_osock_unit(mrb_state *mrb,mrb_value self)
{
  mrb_osock_stc *s=DATA_PTR(self);

  return mrb_fixnum_value(s->unit);
}

static void osock_free(mrb_state *mrb, void *p)
{
  mrb_osock_stc *s=(mrb_osock_stc *)p;

  close(s->unit);
  unlink(s->name);
  free(s->name);
  
  mrb_free(mrb,p);
}

