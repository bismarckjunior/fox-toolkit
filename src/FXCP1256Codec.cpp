#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXTextCodec.h"
#include "FXCP1256Codec.h"

namespace FX {

FXIMPLEMENT(FXCP1256Codec,FXTextCodec,NULL,0)


//// Created by codec tool on 03/25/2005 from: CP1256.TXT ////
const unsigned short forward_data[256]={
   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
   96,   97,   98,   99,   100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,
   8364, 1662, 8218, 402,  8222, 8230, 8224, 8225, 710,  8240, 1657, 8249, 338,  1670, 1688, 1672,
   1711, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 1705, 8482, 1681, 8250, 339,  8204, 8205, 1722,
   160,  1548, 162,  163,  164,  165,  166,  167,  168,  169,  1726, 171,  172,  173,  174,  175,
   176,  177,  178,  179,  180,  181,  182,  183,  184,  185,  1563, 187,  188,  189,  190,  1567,
   1729, 1569, 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, 1579, 1580, 1581, 1582, 1583,
   1584, 1585, 1586, 1587, 1588, 1589, 1590, 215,  1591, 1592, 1593, 1594, 1600, 1601, 1602, 1603,
   224,  1604, 226,  1605, 1606, 1607, 1608, 231,  232,  233,  234,  235,  1609, 1610, 238,  239,
   1611, 1612, 1613, 1614, 244,  1615, 1616, 247,  1617, 249,  1618, 251,  252,  8206, 8207, 1746,
  };


const unsigned char reverse_plane[17]={
  0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
  };

const unsigned char reverse_pages[73]={
  0,  45, 91, 91, 91, 91, 91, 91, 155,91, 91, 91, 91, 91, 91, 91,
  91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
  91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
  91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
  91, 91, 91, 91, 91, 91, 91, 91, 91,
  };

const unsigned short reverse_block[219]={
  0,   16,  32,  48,  64,  80,  96,  112, 128, 128, 144, 160, 128, 175, 191, 207,
  128, 128, 128, 128, 128, 221, 128, 128, 128, 235, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 245, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 252, 265, 281,
  297, 313, 329, 128, 336, 351, 366, 375, 391, 406, 420, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 424, 440, 456, 472, 128,
  128, 128, 128, 128, 128, 483, 128, 128, 128, 128, 128, 128, 128, 497, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  };

const unsigned char reverse_data[513]={
   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
   96,   97,   98,   99,   100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   160,  26,   162,  163,  164,  165,  166,  167,  168,  169,  26,   171,  172,  173,  174,  175,
   176,  177,  178,  179,  180,  181,  182,  183,  184,  185,  26,   187,  188,  189,  190,  26,
   26,   26,   26,   26,   26,   26,   215,  26,   26,   26,   26,   26,   26,   26,   26,   224,
   26,   226,  26,   26,   26,   26,   231,  232,  233,  234,  235,  26,   26,   238,  239,  26,
   26,   26,   26,   244,  26,   26,   247,  26,   249,  26,   251,  252,  26,   26,   26,   140,
   156,  26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   131,  26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   136,  26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   161,  26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   186,  26,   26,   26,   191,  26,   193,  194,  195,  196,  197,  198,
   199,  200,  201,  202,  203,  204,  205,  206,  207,  208,  209,  210,  211,  212,  213,  214,
   216,  217,  218,  219,  26,   26,   26,   26,   26,   220,  221,  222,  223,  225,  227,  228,
   229,  230,  236,  237,  240,  241,  242,  243,  245,  246,  248,  250,  26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   138,  26,   26,   26,   26,   129,  26,
   26,   26,   26,   26,   26,   141,  26,   143,  26,   26,   26,   26,   26,   26,   26,   154,
   26,   26,   26,   26,   26,   26,   142,  26,   26,   26,   26,   26,   26,   26,   26,   26,
   152,  26,   26,   26,   26,   26,   144,  26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   159,  26,   26,   26,   170,  26,   192,  26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   255,  26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   157,  158,  253,  254,  26,   26,   26,   150,  151,  26,   26,   26,
   145,  146,  130,  26,   147,  148,  132,  26,   134,  135,  149,  26,   26,   26,   133,  26,
   26,   26,   26,   26,   26,   26,   26,   26,   137,  26,   26,   26,   26,   26,   26,   26,
   26,   139,  155,  26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   128,
   26,   26,   26,   153,  26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,
  };


FXint FXCP1256Codec::mb2wc(FXwchar& wc,const FXchar* src,FXint nsrc) const {
  if(nsrc<1) return -1;
  wc=forward_data[(FXuchar)src[0]];
  return 1;
  }


FXint FXCP1256Codec::wc2mb(FXchar* dst,FXint ndst,FXwchar wc) const {
  if(ndst<1) return -1;
  dst[0]=reverse_data[reverse_block[reverse_pages[reverse_plane[wc>>16]+((wc>>10)&63)]+((wc>>4)&63)]+(wc&15)];
  return 1;
  }

FXint FXCP1256Codec::mibEnum() const {
  return 2256;
  }


const FXchar* FXCP1256Codec::name() const {
  return "windows-1256";
  }


const FXchar* FXCP1256Codec::mimeName() const {
  return "windows-1256";
  }


const FXchar* const* FXCP1256Codec::aliases() const {
  static const FXchar *const list[]={"microsoft-cp1256","windows-1256","cp1256",NULL};
  return list;
  }

}
