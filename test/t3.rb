#!/usr/src/mruby/bin/mruby

spi=Spi::new(2,0)
m=Mrf89::new(spi)
m.prepare_for_receive

RE=HsRegexp::new('[A-Za-z0-9]')

def hexp(b)
  s=''
  a=b.bytes.to_a
  a.each do |c|
    if(c==0)
      s+='.'
    else
      c=c.chr
      s+=(RE.match(c)) ? c : '.'
    end
  end
  s+='| '
  a.each do |c|    
    s+=sprintf('%2.2x ',c)
  end
  loggo(s)
end

m.set_channel(4)

loop do
  hexp(m.receive()) #if(iv>100)
end

