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

* controlling the onboard LED
* talking to the **SPI** serial interface
* talking to an
  [Olimex MOD-MRF89-868](https://www.olimex.com/Products/Modules/RF/MOD-MRF89-868/)
  board via the **UEXT** interface of the A13 board.
* a beginning of a library for managing the framebuffer and
  touchscreen of an
  [Olimex A13-LCD43TS](https://www.olimex.com/Products/OLinuXino/A13/A13-LCD43TS) 4.3inch
  touch-screen.

### The LED

The A13 board has an on-board green LED that can be controlled via
GPIO pin **PG09**. Once the pin is appropriately configured (see
below), you gain control of it with:

```ruby
l=Led::new
```

and you switch it on and of as follows:

```ruby
l.on
l.off
```

### MRF89

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

### Some pointers

#### Debian

My way is to run Debian on my boards. I use Debian since it was
created (I am *that* old), and I am at home there. Of course, the A13
is not very quick in compiling, but I found it much preferrable to
do my compilations onboard. You find
[here](https://github.com/OLIMEX/OLINUXINO/blob/master/SOFTWARE/A13/olinuxino-debian.pdf)
a PDF that describes how you can end up with a bootable SD-card for
your board, that includes your self-compiled kernel. 

I talk to my boards via Ethernet, using a USB-to-USB cable from my
main PC. All remote operations are performed from a SSH-connected
session. I make sure my main machine offers IP masquerading. This is
not the place to describe how I do that. The important thing is that,
once logged in to the A13, I have full internet connectivity.

#### Talking to **SPI**

First thing, you will need to include the appropriate support for SPI
in your kernel. I have the following selected:

* `CONFIG_SPI_SUN5I`
* `CONFIG_SUN5I_SPI_NDMA`
* `CONFIG_SPI_SPIDEV`

You also have to have GPIO active:

* `GPIO_SUNXI`

The second important thing to do is to open the A13 pins that are used
to talk to SPI#2 (the interface that is wired to the UEXT socket), and
to configure them to be used for SPI. For this, you must modify the
`script.bin` file that you will include in your SD-card. 

The `script.bin` file is a 'compiled' version of a text file including
a series of parameters that are accessible by the `sunxi` modules
included in the kernel.

Here is what you should do.

1. You should already have downloaded the `A13_script_files.zip` file
   [here](https://docs.google.com/file/d/0B-bAEPML8fwlNElERXRUZURTTUU/).
1. Inside that file, you will file another archive, called
   `fex2bin_bin_fex_tools.tar.gz`. Take it out, extract it to some
   directory, and, if you are as paranoid as me, do a `make clean &&
   make`. As a result, you will have an executable called `fexc`, and
   two links to it, called `fex2bin` and `bin2fex`
1. Pick your preferred `script.bin`, copy it to that directory, call
   it `oldscript.bin` and run:
	   `./bin2fex oldscript.bin > script.fex	`
1. Edit the resulting `script.fex` file:

	1. Comment out this block:

			[uart_para]
			uart_debug_port = 1
			uart_debug_tx = port:PG03<4><1><default><default>
			uart_debug_rx = port:PG04<4><1><default><default>
	
		by prepending a `;` to each line (pin PG04 is used by SPI).

	1. Change line

			twi2_used = 1

		to

			twi2_used = 0

	    (pins PB17 and PB18 are used by SPI).
	
	1. Search the block beginning with `[spi2_para]`. Change it so
	    that it reads: 

			[spi2_para]
			spi_used = 1
			spi_cs_bitmap = 1
			spi_cs0 = port:PE00<4><default><default><default>
			spi_sclk = port:PE01<4><default><default><default>
			spi_mosi = port:PE02<4><default><default><default>
			spi_miso = port:PE03<4><default><default><default>

	1. Add these two blocks:

			[spi_devices]
			spi_dev_num = 1

			[spi_board0]
			modalias = "spidev"
			max_speed_hz = 1000000
			bus_num = 2
			chip_select = 0
			mode = 0
			full_duplex = 0
			manual_cs = 0
			

	    All this should make sure that the port connected to SPI bus#2 of
		your A13 (the one wired to the UEXT socket) is seen at boot, and
		managed, within the kernel, by `spidev` (see
		[here](https://www.kernel.org/doc/Documentation/spi/spidev) to
		learn more about spidev).

	1. Make sure you can access the GPIO pins you need to access, by
	    adding this other block: 

			[gpio_para]
			gpio_used = 1
			gpio_num = 5
			gpio_pin_1 = port:PG03<1><default><default><default>
			gpio_pin_2 = port:PG04<0><default><default><default>
			gpio_pin_3 = port:PB17<0><default><default><default>
			gpio_pin_4 = port:PB18<1><default><default><default>
			gpio_pin_5 = port:PG09<1><default><default><default>

	    (you can see all IO ports of the A13
        [here](http://linux-sunxi.org/A13/PIO)). Note that I am also
        adding pin PG09, which is used by the LED module (see above).
	
1. You should then create your own `script.bin` file . 
   run:

		./fex2bin script.fex > script.bin`
	   
   You should make sure that you put **this** file into your SD card.

With the above kernel and `script.bin`, you should be able to see file
`spidev2.0` in your `/dev/` tree. This means that the SPI port is
recognized.

### The touchscreen

The library is in communications both with the framebuffer surface
(mmapped) and with the output of the touchscreen (`/dev/input`
file). As of now, it is capable of:

* returning the current touchscreen position, and whether the screen
  is currently being touched or not.
* filling the screen
* drawing lines
* drawing rectangles
* performing a simple calibration procedure. The calibration results
  are saved in a file in the user's home directory.

It goes without saying that I have no idea whether all of this works
for any of the other screens that Olimex produces. 

### How to compile Mruby

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

This makes sure that my gem, as well as the regular expression gem,
are included into your version of the mruby executable.

It is then sufficient to run `make`, and you will be able to run my
test programs. `./minirake -p` connects to the remote gem directories
and syncronizes with them if needed. 

### My test code

The `/test` folder includes a few scripts I wrote for testing my code.

#### Led

By running `led.rb`, you will invoke a very simple script that toggles
the onboard LED on/off every time you press return.

#### Mrf89

In order to test my code you need to have two boards. The first one
will be transmitting, the second one will be receiving.

Say that you have your `mruby` on `/usr/src/mruby`. Start the
transmitter by running:

	/usr/src/mruby/bin/mruby /usr/src/mruby/build/olinuxino/test/transmit.rb 1 2 3 4 5 6 7 8

The digits represent the channels that will be transmitted on. 32
channels are available. As soon as the program is started, it will
start sending a test message to each of the channels that you specify
on the command line, endlessly cycling.

On the second board, start the receiver by running:

	/usr/src/mruby/bin/mruby /usr/src/mruby/build/olinuxino/test/receive2.rb 4

Whenever a message is received on channel 4, it will be printed on
your screen.

#### Framebuffer

Script `fb.rb` exercises the current capabilities of my library.

The first time you run it, you must calibrate the screen - just touch
on the four crosses in sequence.

Then, multicoloured segments and rectangles are drawn very quickly on
the screen. If you touch the screen, a coloured square will follow your
finger around.
