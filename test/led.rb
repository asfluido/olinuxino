#!/usr/src/mruby/bin/mruby

class Led
  LED_PIN='pg9'
  GPIO_BASE='/sys/class/gpio/'

  def Led::path
    base=nil
    Dir::foreach(GPIO_BASE) do |en|
      r=/gpio(.*)_(.*)/.match(en)
      if(r && r[3]==LED_PIN)
        base=GPIO_BASE+en+'/'
        break
      end
    end

    raise "Could not find gpio pin #{LED_PIN}" unless(base)
    
    File::open(base+'direction','wb') do |f|
      f.puts('out')
    end

    base+'value'
  end

  def Led::from_pin
    Led::new(Led::path)
  end
end

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
