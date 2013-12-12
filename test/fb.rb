#!/usr/src/mruby/bin/mruby

f=Fb::new('/dev/fb0','/dev/input/by-path/platform-sun4i-ts-event')
p f

sleep(2)
