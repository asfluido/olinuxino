#!/usr/src/mruby/bin/mruby

channels=ARGV.map do |v|
  v.to_i
end.sort.uniq

raise "Usage: #{$0} ch ch ch ch" if(channels.length<=0)

channels.each do |c|
  raise "Bad channel #{c}!" if(c<0 || c>=Mrf89::N_CHANNELS)
end

spi=Spi::new(2,0)
m=Mrf89::new(spi)
m.prepare_for_transmit

l=Led::new

sendout='The quick brown fox jumps over the lazy dog %2.2d %8.8d'

cnt=0

loop do
  channels.each do |ch|
    m.set_channel(ch)
    cnt+=1
    msg=sprintf(sendout,ch,cnt)
    loggo(msg)

    l.on
    m.transmit(msg)
    l.off
    
    msleep(10)
  end
end


