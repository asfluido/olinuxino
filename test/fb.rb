#!/usr/src/mruby/bin/mruby

f=Fb::new('/dev/fb0','/dev/input/by-path/platform-sun4i-ts-event')
sx,sy=f.size
loggo("Ueppolo: #{sx}x#{sy}")
touch=nil
loop do
  f.line(rand(sx),rand(sy),rand(sx),rand(sy),rand(0xffffff))
  nt,px,py=f.status
  unless(nt==touch)
    touch=nt
    loggo("#{touch ? 'CLICK' : 'release'} at #{px},#{py}")
  end
end
