class Led
  LED_PIN='pg9'
  GPIO_BASE='/sys/class/gpio/'

  def Led::path
    base=nil
    Dir::foreach(GPIO_BASE) do |en|
      r=/gpio(.*)_(.*)/.match(en)
      if(r && r[2]==LED_PIN)
        base=GPIO_BASE+en+'/'
        break
      end
    end

    raise "Could not find gpio pin #{LED_PIN}" unless(base)

    begin
      File::open(base+'direction','wb') do |f|
        f.puts('out')
      end
    rescue
    end
    
    base+'value'
  end

  def Led::from_pin
    Led::new(Led::path)
  end
end
