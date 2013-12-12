/* oxino.h */

/*
 * 26/11/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * General includes
 */

#ifndef OXINO_H
#define OXINO_H

#define __EXPORTED_HEADERS__

#include <mruby.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <linux/types.h>
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"

extern unsigned char one,zero;

#include "mrf89.h"

#endif
