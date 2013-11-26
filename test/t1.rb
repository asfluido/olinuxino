#!/usr/src/mruby/bin/mruby

spi=Spi::new(2,0)
m=Mrf89::new(spi)
m.prepare_for_transmit

sendout='The quick brown fox %2d %8d'

nch=5

loop do
  nch.times do |ch|
    s.set_channel(ch)
    cnt+=1
    msg=sprintf(sendout,ch,cnt)
    STDERR.printf("\r%s",msg)
    s.transmit(msg)
    sleep(0.1)
  end
end


