// $Id: mbfm_tables.h $
/*
 * MBHP_MBFM MIDIbox FM V2.0 lookup tables
 *
 * ==========================================================================
 *
 *  Copyright (C) 2014 Sauraen (sauraen@gmail.com)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#ifndef MBFM_TABLES_H
#define MBFM_TABLES_H

#include <mios32.h>

/*
  t(p), Mapping for time-parameter values to number of msec per event
  Parameters: t(0) = 0 //Lowest value is 0
            t(255) = 10000 //Highest value is 10000
             t'(0) = 1 //Increment at low end is 1
  Formula: t(p) = e^((ln(10000+1-255)/255)p) + p - 1
  =ROUND(EXP(0.0360180872*A1)+A1-1,0)
*/
extern const u16 TIME_MAPPING[256] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 28, 29, 30, 31, 32, 33, 34, 35, 36, 38, 39, 40, 41, 42, 43, 44, 46, 47, 48, 49, 50, 51, 53, 54, 55, 56, 58, 59, 60, 61, 63, 64, 65, 66, 68, 69, 70, 72, 73, 74, 76, 77, 79, 80, 81, 83, 84, 86, 87, 89, 90, 92, 94, 95, 97, 98, 100, 102, 104, 105, 107, 109, 111, 113, 115, 117, 118, 120, 123, 125, 127, 129, 131, 133, 136, 138, 140, 143, 145, 148, 151, 153, 156, 159, 162, 164, 167, 171, 174, 177, 180, 184, 187, 191, 194, 198, 202, 206, 210, 214, 219, 223, 228, 232, 237, 242, 247, 252, 258, 263, 269, 275, 281, 287, 294, 301, 307, 315, 322, 329, 337, 345, 354, 362, 371, 380, 390, 399, 409, 420, 431, 442, 453, 465, 477, 490, 503, 517, 531, 545, 560, 576, 592, 608, 625, 643, 661, 680, 700, 720, 741, 763, 786, 809, 833, 858, 884, 911, 938, 967, 997, 1028, 1060, 1093, 1127, 1162, 1199, 1237, 1276, 1317, 1359, 1403, 1448, 1495, 1543, 1594, 1646, 1700, 1756, 1814, 1874, 1936, 2000, 2067, 2136, 2208, 2282, 2359, 2439, 2521, 2607, 2696, 2788, 2883, 2982, 3084, 3190, 3300, 3414, 3532, 3654, 3781, 3912, 4049, 4190, 4336, 4487, 4645, 4807, 4976, 5151, 5332, 5520, 5715, 5917, 6126, 6343, 6568, 6801, 7042, 7293, 7552, 7821, 8100, 8389, 8688, 8999, 9321, 9654, 10000
};

/*
  A simple 1024-step-long sine table. Range -127 to 127.
*/
extern const s8 SINE_TABLE[1024] = {
  0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 22, 22, 23, 24, 25, 26, 26, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38, 38, 39, 40, 41, 41, 42, 43, 44, 44, 45, 46, 46, 47, 48, 49, 49, 50, 51, 51, 52, 53, 54, 54, 55, 56, 56, 57, 58, 58, 59, 60, 61, 61, 62, 63, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 71, 71, 72, 72, 73, 74, 74, 75, 76, 76, 77, 78, 78, 79, 79, 80, 81, 81, 82, 82, 83, 84, 84, 85, 85, 86, 86, 87, 88, 88, 89, 89, 90, 90, 91, 91, 92, 93, 93, 94, 94, 95, 95, 96, 96, 97, 97, 98, 98, 99, 99, 100, 100, 101, 101, 102, 102, 102, 103, 103, 104, 104, 105, 105, 106, 106, 106, 107, 107, 108, 108, 109, 109, 109, 110, 110, 111, 111, 111, 112, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 116, 116, 116, 117, 117, 117, 118, 118, 118, 118, 119, 119, 119, 120, 120, 120, 120, 121, 121, 121, 121, 122, 122, 122, 122, 122, 123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 124, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 125, 125, 125, 125, 125, 125, 125, 124, 124, 124, 124, 124, 124, 123, 123, 123, 123, 123, 122, 122, 122, 122, 122, 121, 121, 121, 121, 120, 120, 120, 120, 119, 119, 119, 118, 118, 118, 118, 117, 117, 117, 116, 116, 116, 115, 115, 115, 114, 114, 114, 113, 113, 113, 112, 112, 112, 111, 111, 111, 110, 110, 109, 109, 109, 108, 108, 107, 107, 106, 106, 106, 105, 105, 104, 104, 103, 103, 102, 102, 102, 101, 101, 100, 100, 99, 99, 98, 98, 97, 97, 96, 96, 95, 95, 94, 94, 93, 93, 92, 91, 91, 90, 90, 89, 89, 88, 88, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 80, 79, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 71, 70, 69, 69, 68, 67, 67, 66, 65, 65, 64, 63, 63, 62, 61, 61, 60, 59, 58, 58, 57, 56, 56, 55, 54, 54, 53, 52, 51, 51, 50, 49, 49, 48, 47, 46, 46, 45, 44, 44, 43, 42, 41, 41, 40, 39, 38, 38, 37, 36, 35, 35, 34, 33, 32, 32, 31, 30, 29, 29, 28, 27, 26, 26, 25, 24, 23, 22, 22, 21, 20, 19, 19, 18, 17, 16, 16, 15, 14, 13, 12, 12, 11, 10, 9, 9, 8, 7, 6, 5, 5, 4, 3, 2, 2, 1, 0, -1, -2, -2, -3, -4, -5, -5, -6, -7, -8, -9, -9, -10, -11, -12, -12, -13, -14, -15, -16, -16, -17, -18, -19, -19, -20, -21, -22, -22, -23, -24, -25, -26, -26, -27, -28, -29, -29, -30, -31, -32, -32, -33, -34, -35, -35, -36, -37, -38, -38, -39, -40, -41, -41, -42, -43, -44, -44, -45, -46, -46, -47, -48, -49, -49, -50, -51, -51, -52, -53, -54, -54, -55, -56, -56, -57, -58, -58, -59, -60, -61, -61, -62, -63, -63, -64, -65, -65, -66, -67, -67, -68, -69, -69, -70, -71, -71, -72, -72, -73, -74, -74, -75, -76, -76, -77, -78, -78, -79, -79, -80, -81, -81, -82, -82, -83, -84, -84, -85, -85, -86, -86, -87, -88, -88, -89, -89, -90, -90, -91, -91, -92, -93, -93, -94, -94, -95, -95, -96, -96, -97, -97, -98, -98, -99, -99, -100, -100, -101, -101, -102, -102, -102, -103, -103, -104, -104, -105, -105, -106, -106, -106, -107, -107, -108, -108, -109, -109, -109, -110, -110, -111, -111, -111, -112, -112, -112, -113, -113, -113, -114, -114, -114, -115, -115, -115, -116, -116, -116, -117, -117, -117, -118, -118, -118, -118, -119, -119, -119, -120, -120, -120, -120, -121, -121, -121, -121, -122, -122, -122, -122, -122, -123, -123, -123, -123, -123, -124, -124, -124, -124, -124, -124, -125, -125, -125, -125, -125, -125, -125, -126, -126, -126, -126, -126, -126, -126, -126, -126, -126, -126, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -126, -126, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125, -125, -125, -125, -124, -124, -124, -124, -124, -124, -123, -123, -123, -123, -123, -122, -122, -122, -122, -122, -121, -121, -121, -121, -120, -120, -120, -120, -119, -119, -119, -118, -118, -118, -118, -117, -117, -117, -116, -116, -116, -115, -115, -115, -114, -114, -114, -113, -113, -113, -112, -112, -112, -111, -111, -111, -110, -110, -109, -109, -109, -108, -108, -107, -107, -106, -106, -106, -105, -105, -104, -104, -103, -103, -102, -102, -102, -101, -101, -100, -100, -99, -99, -98, -98, -97, -97, -96, -96, -95, -95, -94, -94, -93, -93, -92, -91, -91, -90, -90, -89, -89, -88, -88, -87, -86, -86, -85, -85, -84, -84, -83, -82, -82, -81, -81, -80, -79, -79, -78, -78, -77, -76, -76, -75, -74, -74, -73, -72, -72, -71, -71, -70, -69, -69, -68, -67, -67, -66, -65, -65, -64, -63, -63, -62, -61, -61, -60, -59, -58, -58, -57, -56, -56, -55, -54, -54, -53, -52, -51, -51, -50, -49, -49, -48, -47, -46, -46, -45, -44, -44, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -35, -35, -34, -33, -32, -32, -31, -30, -29, -29, -28, -27, -26, -26, -25, -24, -23, -22, -22, -21, -20, -19, -19, -18, -17, -16, -16, -15, -14, -13, -12, -12, -11, -10, -9, -9, -8, -7, -6, -5, -5, -4, -3, -2, -2, -1
};

#endif
