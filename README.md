# olinuxino

### A mruby gem to dabble with an olinuxino A13 board

This repository contains a little bit of software targeted for the 
[Olimex Olinuxino-A13](https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
computer board. Whatever is included may work for other
Allwinner-based Olimex boards, but I only have these boards here, so I
cannot test on anything else.

The software is packaged as a [Mruby](https://github.com/mruby/mruby)
gem. Mruby is a small-footprint implementation of the
[Ruby](http://www.ruby-lang.org) language written by the Ruby creator
targeted to small harware and embedded application.

Currently, the code focuses on:

* talking to the **SPI** serial interface
* talking to an
  [Olimex MOD-MRF89-868](https://www.olimex.com/Products/Modules/RF/MOD-MRF89-868/)
  board via the **UEXT** interface of the A13 board.

The **MRF89** board is a radio receiver/transmitter board for the
[868MHz (SRD)](http://en.wikipedia.org/wiki/Short_Range_Devices)
frequency band.

Around April 2013 I saw the possibility to develop a work that could
make use of this hardware. I thus purchased three boards, and saw if I
could make sense of the mechanism.

When I started working on this, I found out that there was practically
no available information about how to use **SPI** with the Olinuxino
boards. I also had very little experience on this.

One month, and much sweat later, I was able to have one board sending
a data packet, and a second board reliably receiving it. Soon after,
the project was canceled and I had to deal with other things.

Six months later, I found myself with a couple of weeks' time and with
the desire to learn a bit about Mruby. I thus decided to port the old
code (which I had written in Ruby/C) to Mruby, and to publish it on
Github, in the hope that it may be useful to others.

**This code may not be exactly what you need. You are free to contact
  [me](mailto:ghub@fluido.as) if you are facing similar problems, but
  please remember that I do not have loads of experience on this.**

### Some pointers

#### How to compile Mruby

My way is to run Debian on my boards. I use Debian since it was
created (I am *that* old), and I am at home there. Of course, the A13
is not very quick in compiling, but I found it much preferrable to
do my compilations onboard.

I talk to my boards via Ethernet, using a USB-to-USB cable from my
main PC. All remote operations are performed from a SSH-connected
session. I make sure my main machine offers IP masquerading. This is
not the place to describe how I do that. The important thing is that,
once logged in to the A13, I have full internet connectivity.

Log in to your board. Make sure you have the normal compilation
environment. *Make sure you install a version of Ruby!* It is needed
for compiling Mruby. Install Git. Then go to a suitable directory and
type

	git clone https://github.com/mruby/mruby.git

Before compiling, you must add a couple of lines to file
`build_config.rb`:

```ruby
conf.gem(:github=>"asfluido/olinuxino",:branch=>"master")
conf.gem(:github=>"mattn/mruby-onig-regexp",:branch=>"master")
```

