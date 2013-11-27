#!/usr/src/mruby/bin/mruby

spi=Spi::new(2,0)
m=Mrf89::new(spi)
m.prepare_for_receive

def hexp(b)
  s=''
  a=b.bytes.to_a
  a.each do |c|
    c=c.chr
    s+=(/[A-Za-z0-9]/.match(c) ? c : '.')
  end
  s+='| '
  a.each do |c|    
    s+=sprintf('%2.2x ',c.ord)
  end
  loggo(s)
end

m.set_channel(4)

loop do
  hexp(m.receive()) #if(iv>100)
end

