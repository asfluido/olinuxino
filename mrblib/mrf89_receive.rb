class Mrf89
  def prepare_for_receive
    set_para(1,0xa8) # buffered mode
    set_para(0xd,0xb) # interrupts
  end
end

