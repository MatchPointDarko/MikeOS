#ifndef MIKE_OS_PORT_IO_H
#define MIKE_OS_PORT_IO_H

/* Function: write_port
 * ------------------
 * write a byte to an I/O port.
 *
 * port: the port to write to.
 * value: the value to write.
 */
void write_port(unsigned int port, unsigned char value);

/* Function: read_port
 * ------------------
 * read a byte from an I/O port.
 *
 * port: the port to read from.
 *
 * returns: the value that was read from the port.
 */
unsigned char read_port(unsigned int port);

/* Function: write_word_port
 * ------------------
 * write a word to an I/O port.
 *
 * port: the port to write to.
 * value: the value to write.
 */
void write_word_port(unsigned int port, unsigned short value);

/* Function: read_port
 * ------------------
 * read a word from an I/O port.
 *
 * port: the port to read from.
 *
 * returns: the value that was read from the port.
 */
unsigned short read_word_port(unsigned int);

#endif
