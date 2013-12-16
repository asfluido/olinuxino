#!/usr/src/mruby/bin/mruby

f=Fb::new('/dev/fb0','/dev/input/by-path/platform-sun4i-ts-event')

touch=nil
loop do
  nt,px,py=f.status
  if(nt!=touch)
    p [nt,px,py]
    touch=nt
    loggo("#{touch ? 'CLICK' : 'release'} at #{px},#{py}")
  end
end
