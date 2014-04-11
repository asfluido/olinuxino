#!/usr/src/mruby/bin/mruby
l=Led::from_pin

status=false
l.off

loop do
  readline
  status=!status
  if(status)
    l.on
  else
    l.off
  end
end
