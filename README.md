# olinuxino

## A mruby gem to dabble with an olinuxino A13 board

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

* talking to the *SPI* interface
* talking to an
  [Olimex MOD-MRF89-868](https://www.olimex.com/Products/Modules/RF/MOD-MRF89-868/)
  board via the *UEXT* interface of the A13 board.




