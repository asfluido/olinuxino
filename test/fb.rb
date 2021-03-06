#!/usr/src/mruby/bin/mruby

RECT=35

f=Fb::new('/dev/fb0','/dev/input/by-path/platform-sun4i-ts-event')

cd=f.calibdata
unless(cd)
  f.perf_calibration
  cd=f.calibdata
end

sx,sy=f.size

loop do
  vs=[rand(sx),rand(sy),rand(sx),rand(sy),rand(0xffffff)]

  if(rand(100)==0)
    f.rect(*vs)
  else
    f.line(*vs)
  end
  
  nt,px,py=f.status(true)
  f.rect(px-RECT,py-RECT,px+RECT,py+RECT,rand(0xffffff)) if(nt)
end
