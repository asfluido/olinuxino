#!/usr/src/mruby/bin/mruby

RECT=5

f=Fb::new('/dev/fb0','/dev/input/by-path/platform-sun4i-ts-event')

cd=f.calibdata
unless(cd)
  f.calibrate 
  cd=f.calibdata
end

sx,sy=f.size
loggo("Ueppolo: #{sx}x#{sy}")
touch=nil
loop do
  vs=[rand(sx),rand(sy),rand(sx),rand(sy),rand(0xffffff)]

  if(rand(100)==0)
    f.rect(*vs)
  else
    f.line(*vs)
  end
  
  nt,px,py=f.status
  #  unless(nt==touch)
  #    touch=nt
  #    loggo("#{touch ? 'CLICK' : 'release'} at #{px},#{py}")
  #  end
  if(nt)
    cx=((px-cd[0])*cd[1]).to_i
    cy=((py-cd[2])*cd[3]).to_i
    a=[cx-RECT,cy-RECT,cx+RECT,cy+RECT,rand(0xffffff)]
    p a
    f.rect(*a)
  end
end
