/***********************************************************************/
/*                                                                     */
/*  Copyright Michael Berens 14.2.2014                                 */
/***********************************************************************/
/*                                                                     */
/*  LPC111X.H:  Header file for NXP LPC1114/301                        */
/*                                                                     */
/***********************************************************************/

#ifndef LPC111x_H
#define LPC111x_H

/* General Purpose Input/Output (GPIO) */
#define GPIO2DIR	(*((volatile unsigned long*) 0x50028000))
#define GPIO2DATA	(*((volatile unsigned long*) 0x50023FFC))
#define GPIO3DIR	(*((volatile unsigned long*) 0x50038000))
#define GPIO3DATA	(*((volatile unsigned long*) 0x50033FFC))

#endif  // LPC111x_H
