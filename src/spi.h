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

#define CSCON_PIN "pb18"
#define IRQ1_PIN "pg4"
#define IRQ2_PIN "pb17"

typedef struct mrb_spi
{
  int spi_no[2],unit,cscon_unit,irq_units[2];
} mrb_spi_stc;

extern int spi_low_write(mrb_spi_stc *s,__u8 *b,int len,__u8 cscon);
extern int spi_low_read(mrb_spi_stc *s,__u8 *b,int len,__u8 cscon);

#endif
