class Fb
  MARG=30
  CROSS_ARM=20
  CROSS_THICK=3
  CROSS_COL=0xff4036
  
  def calibrate
    sx,sy=size()

    res=[[MARG,MARG],[sx-MARG,MARG],[sx-MARG,sy-MARG],[MARG,sy-MARG]].map do |x,y|
      getfinger(x,y)      
    end
    xpclow=(res[0][0]+res[3][0])/2.0
    xpchigh=(res[1][0]+res[2][0])/2.0
    xperdig=(sx-MARG*2)/(xpchigh-xpclow)
    ypclow=(res[0][1]+res[1][1])/2.0
    ypchigh=(res[2][1]+res[3][1])/2.0
    yperdig=(sy-MARG*2)/(ypchigh-ypclow)
    
    p [xperdig,yperdig,RES[0][0]*xperdig,RES[1][0]*yperdig]
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
