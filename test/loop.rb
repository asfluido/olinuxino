#!/usr/src/mruby/bin/mruby

s=Osock::new('/tmp/oso')
l=Mainloop::new(300)

p [s,l,s.unit]

l.add_unit(s,s.unit,"ckd")

msleep(300000)

