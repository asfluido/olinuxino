#!/usr/src/mruby/bin/mruby

l=Led::new

status=false
l.off

loop do
  getch
  status=!status
  if(status)
    l.on
  else
    l.off
  end
end
