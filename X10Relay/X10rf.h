/************************************************************************/
/* X10 RF receiver library, v1.2.    (jjg SB v1.3)                                   */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet thomas@mittet.nu October 2010.              */
/************************************************************************/
/* NOTES COLLECTED BY JJG RE TIMING - USE TO TWEEK DEFULT SETTINGS
 *  *  from http://davehouston.org/rf.htm
 * Each code starts with a ~9000us burst of carrier followed by a 4500us silence.
 * binary 1 represented by 2250us between rising edges (560us mark + 1690us space)
 * binary 0 represented by 1120us between rising edges (560us mark + 560us space)
 * The overall time to send one code, including the approximate 40ms silence, is about 108ms. 
 * Timings vary by transmitter. (seen a range from 95ms to 116ms for various transmitters)
  *   ** http://www.printcapture.com/files/X10_RF_Receiver.pdf
 * When you add in a fudge factor or +/- 30% to the timing, the values become:
 * 1. gap - 40,000 +/- 30% = 28,000 to 52,000us
 * 2. header - 13,500 +/- 30% = 9,450 to 17,550us
 * 3. Bit 0 - 1,120 +/- 30% = 787 to 1,463us
 * 4. Bit 1 - 2,250 +/- 30% = 1,575 to 2,925us
 */


#ifndef X10rf_h
#define X10rf_h

#include <inttypes.h>
// RF initial start burst min length
#define X10_RF_SB_MIN            9450  //was 1200 **jjg 9450
// RF start burst max length
#define X10_RF_SB_MAX           17550  //was 15000 **jjg 17550
// RF repeat start burst min length
// If you get choppy dimming, lower this threshold
#define X10_RF_RSB_MIN           7000
// RF bit 0 min length

#define X10_RF_BIT0_MIN          787  //was 1000 **jjg 787
// RF bit 0 max length
#define X10_RF_BIT0_MAX          1463 //was 1200 **jjg 1463
// RF bit 1 min length
#define X10_RF_BIT1_MIN          1575 //was 2100 **jjg 1575
// RF bit 1 max length
#define X10_RF_BIT1_MAX          2925 //was 2300 **jjg 2925
// When repeated start burst is detected within this millisecond threshold
// of the last command received, it is assumed that the following command
// is the same and that it does not need to be parsed
#define X10_RF_REPEAT_THRESHOLD   500 //was 200ms **jjg using 500 stops duplicates for RF

#define CMD_ON      B0010
#define CMD_OFF     B0011
#define CMD_DIM     B0100
#define CMD_BRIGHT  B0101

class X10rf
{

public:
  typedef void (*rfReceiveCallback_t)(char, uint8_t, uint8_t, bool);
  X10rf(uint8_t receiveInt, uint8_t receivePin, rfReceiveCallback_t rfReceiveCallback);
  // Public methods
  void begin();
  void receive();
  
private:
  static const uint8_t HOUSE_CODE[16];
  // Set in constructor
  uint8_t receiveInt, receivePin;
  rfReceiveCallback_t rfReceiveCallback;
  // Used by interrupt triggered methods
  uint32_t riseUs, receiveEnded;
  char house;
  uint8_t unit, command;
  int8_t receivedCount;
  uint32_t receiveBuffer;
  // Private methods
  void handleCommand(uint8_t byte1, uint8_t byte2);
  char parseHouseCode(uint8_t data);
};

#endif
