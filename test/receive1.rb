#!/usr/src/mruby/bin/mruby

spi=Spi::new(2,0)
m=Mrf89::new(spi)
m.prepare_for_receive

maxv=Array::new(Mrf89::N_CHANNELS,0)
minv=Array::new(Mrf89::N_CHANNELS,255)

loop do
  Mrf89::N_CHANNELS.times do |ch|
    m.set_channel(ch)
    msleep(20)
    
    10.times do
      iv=m.req_para(0x14)
      next if(iv<20)
      c=false
      if(maxv[ch]<iv)
        maxv[ch]=iv
        c=true
      end

      minv[ch]=iv if(minv[ch]>iv)
      next unless(c)
      s=sprintf("Ch%2d: %3d ->",ch,iv)
      maxv.each_with_index do |v,ix|
        s+=" |" if((ix%8)==0)
        s+=sprintf(" %2.2x",v)
      end
      loggo(s)
      i=m.get_irqs
      unless(i[0]==48 && i[1]==48)
        r=m.read(1,false)
        loggo(sprintf("[%2.2x] (%c%c)",r.bytes.to_a[0],i[0].chr,i[1].chr))
      end
    end
  end
end


