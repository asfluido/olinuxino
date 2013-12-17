class Fb
  MARG=30
  CROSS_ARM=20
  CROSS_THICK=3
  CROSS_COL=0xff4036
  
  def calibrate
    sx,sy=size()

    [[MARG,MARG],[sx-MARG,MARG],[sx-MARG,sy-MARG],[MARG,sy-MARG]].each do |x,y|
      fx,fy=getfinger(x,y)
      loggo("#{[x,x]} -> #{[fx,fy]}")
    end

  end

  def getfinger(x,y)
    fill(0)
    rect(x-CROSS_ARM,y-CROSS_THICK,x+CROSS_ARM,y+CROSS_THICK,CROSS_COL)
    rect(x-CROSS_THICK,y-CROSS_ARM,x+CROSS_THICK,y+CROSS_ARM,CROSS_COL)
    click=false
    px=nil
    py=nil
    loop do
      t,px,py=status()
      if(!click)
        click=true if(t)
        next
      end
      break unless(t)
    end
    [px,py]
  end    
end
