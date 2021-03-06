#!/usr/src/mruby/bin/mruby

RE=/[[:print:]]/

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

raise "Usage: #{$0} chn" unless(ARGV.length==1)
ch=ARGV[0].to_i

spi=Spi::new(2,0)
m=Mrf89::new(spi)
m.prepare_for_receive

m.set_channel(ch)

loop do
  hexp(m.receive())
end

