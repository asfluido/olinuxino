# mrf89_transmit.rb

class Mrf89
  def prepare_for_transmit
    set_para(1,0x8c) # packet mode
    set_para(0xd,0x8) # FTXRXIREG
    set_para(0xe,0x1d) # FTPRIREG
  end
end
