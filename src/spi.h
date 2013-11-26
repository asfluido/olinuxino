/* spi.h */

/*
 * 26/11/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * Includes for SPI
 */

#ifndef SPI_H
#define SPI_H

typedef struct mrb_spi
{
  int spi_no[2],unit,cscon_unit,irq_units[2];
} mrb_spi_stc;

#define CSCON_PIN "pb18"
#define IRQ1_PIN "pg4"
#define IRQ2_PIN "pb17"

#endif
