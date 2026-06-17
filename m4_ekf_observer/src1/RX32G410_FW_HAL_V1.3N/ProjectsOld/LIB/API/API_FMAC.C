/*********************************************************************************************
Copyright <2024> <Icore Technology (Nanjing)  Co.,Ltd>
All Rights Reserved,
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to
endorse or promote products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************/

/*
*********************************************************************************************************
*                                              rx32g4xx
*                                           Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : rx32g4xx
* File         : main.c
* By           : RX_DV_Team
*********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/

#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"
#include	"API_FMAC.H"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FmacInt 1	

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FMAC_HandleTypeDef hfmac;
DMA_HandleTypeDef  hdma_fmac_preload;
DMA_HandleTypeDef  hdma_fmac_write;
DMA_HandleTypeDef  hdma_fmac_read;

/* FMAC configuration structure */
FMAC_FilterConfigTypeDef sFmacConfig={0};

/* 4阶 FIR 180K 低通滤波器 */
static double TestCoeff[5] =
{
    0.0142,  0.2252, 0.5212,  0.2252,  0.0142
};


__weak		void API_FMAC_AppAdcOverCallBack(API_FMAC_MEMDEF* fmacMem)
{}
__weak		void API_FMAC_AppPowerOverCallBack(API_FMAC_MEMDEF* fmacMem)
{}

///* 转换后的浮点数系数（Q1.15 → float） */
//static float TestCoeff_Float = 
//{
//    465 / 32768.0f,   // = 0.014190674f
//    7379 / 32768.0f,  // = 0.225204468f
//    17079 / 32768.0f, // = 0.521255493f
//    7379 / 32768.0f,  // = 0.225204468f
//    465 / 32768.0f    // = 0.014190674f
//};


/* Q1.15格式 8阶 FIR 180K 低通滤波器 */
//const int16_t TestCoeff_Q1P15[COEFF_VECTOR_B_SIZE+1] = {
//    (int16_t)(0.018099 * 32768),   // =  593
//    (int16_t)(-0.033233 * 32768),  // = -1089
//    (int16_t)(-0.046478 * 32768),  // = -1523
//    (int16_t)(0.317199 * 32768),   // = 10393
//    (int16_t)(0.480827 * 32768),   // = 15758
//    (int16_t)(0.317199 * 32768),   // = 10393
//    (int16_t)(-0.046478 * 32768),  // = -1523
//    (int16_t)(-0.033233 * 32768),  // = -1089
//    (int16_t)(0.018099 * 32768)    // =  593
//};


#ifdef	Fmac9

const int16_t aFilterCoeffB[COEFF_VECTOR_B_SIZE] = {
    (int16_t)(0.018099 / 0.992 * 32768),// = 598,   // 原 593 → 598
    (int16_t)(-0.033233 / 0.992 * 32768),// = -1098, // 原 -1089 → -1098
    (int16_t)(-0.046478 / 0.992 * 32768),// = -1536, // 原 -1523 → -1536
    (int16_t)(0.317199 / 0.992 * 32768) ,//= 10478,  // 原 10393 → 10478
    (int16_t)(0.480827 / 0.992 * 32768) ,//= 15878,  // 原 15758 → 15878
    (int16_t)(0.317199 / 0.992 * 32768) ,//= 10478,  // 同上
    (int16_t)(-0.046478 / 0.992 * 32768), //= -1536, // 同上
    (int16_t)(-0.033233 / 0.992 * 32768), //= -1098, // 同上
    (int16_t)(0.018099 / 0.992 * 32768) ,//= 598     // 同上
};

#endif

#ifdef	Fmac21

const int16_t aFilterCoeffB[COEFF_VECTOR_B_SIZE] = {
  -659,
  -1915,
  -2005,
  -358,
  1679,
  1089,
  -1853,
  -2807,
  2077,
  10186,
  14235,
  10186,
  2077,
  -2807,
  -1853,
  1089,
  1679,
  -358,
  -2005,
  -1915,
  -659
};

#endif


/* 输入数据数组，实际需要100个ADC采样值 */
//static int16_t adcInputValues[100] =
//{
//    2048,  2027,  2046,  2045,  2044,  2043,  2042,  2041,  2060,  2039,
//    2048,  2029,  2050,  2051,  2052,  2053,  2054,  2055,  2066,  2057,
//    2048,  2027,  2046,  2045,  2044,  2043,  2042,  2041,  2060,  2039,
//    2048,  2029,  2050,  2051,  2052,  2053,  2054,  2055,  2066,  2057,
//    2048,  2027,  2046,  2045,  2044,  2043,  2042,  2041,  2060,  2039,
//    2048,  2029,  2050,  2051,  2052,  2053,  2054,  2055,  2066,  2057,
//    2048,  2027,  2046,  2045,  2044,  2043,  2042,  2041,  2060,  2039,
//    2048,  2029,  2050,  2051,  2052,  2053,  2054,  2055,  2066,  2057,
//    2048,  2027,  2046,  2045,  2044,  2043,  2042,  2041,  2060,  2039,
//    2048,  2029,  2050,  2051,  2052,  2053,  2054,  2055,  2066,  2057,
//};





//#define SAMPLE_RATE 1000000  // 1MHz采样率
#define NUM_SAMPLES 100      // 100个数据点

// Q1.15格式混合信号数组 (20kHz基频 + 200kHz噪声)
const int16_t adcInputValues[NUM_SAMPLES] = {
     0,   30,  119,  267,  474,  739, 1061, 1438, 1867, 2345,  // 0-9
  2868, 3429, 4023, 4641, 5276, 5920, 6563, 7198, 7816, 8410,  // 10-19
  8971, 9493, 9968,10391,10755,11056,11289,11450,11537,11548,  // 20-29
 11482,11340,11122,10830,10467,10037, 9545, 8995, 8394, 7748,  // 30-39
  7064, 6350, 15614, 0, 4109, 3358, 2619, 1901, 1213,  562,  // 40-49
   -46, -609,-1120,-1574,-1968,-2298,-2562,-2758,-2885,-2943,  // 50-59
 -2934,-2858,-2718,-2518,-2260,-1950,-1592,-1191, -753, -284,  // 60-69
   206,  707, 1210, 1707, 2189, 2647, 3073, 0, 13798, 4084,  // 70-79
  4311, 4475, 0, 14605, 4569, 4466, 4299, 4070, 3784, 3445,  // 80-89
  3059, 2632, 2170, 1681, 1172,  651,  126, -397, -913,-1417   // 90-99
};

//// 浮点版本参考数组 (不需要math.h)
//const float mixed_signal_float[NUM_SAMPLES] = {
//  0.00000f, 0.00092f, 0.00364f, 0.00816f, 0.01448f, 0.02257f, 0.03240f, 0.04392f, 0.05706f, 0.07177f,
//  0.08796f, 0.10556f, 0.12446f, 0.14457f, 0.16576f, 0.18792f, 0.21089f, 0.23455f, 0.25873f, 0.28329f,
//  0.30805f, 0.33286f, 0.35755f, 0.38195f, 0.40589f, 0.42922f, 0.45177f, 0.47339f, 0.49394f, 0.51328f,
//  0.53128f, 0.54782f, 0.56279f, 0.57609f, 0.58764f, 0.59737f, 0.60522f, 0.61115f, 0.61513f, 0.61715f,
//  0.61720f, 0.61530f, 0.61149f, 0.60580f, 0.59830f, 0.58905f, 0.57814f, 0.56566f, 0.55171f, 0.53640f,
//  0.51985f, 0.50218f, 0.48353f, 0.46402f, 0.44381f, 0.42304f, 0.40185f, 0.38038f, 0.35879f, 0.33721f,
//  0.31577f, 0.29462f, 0.27387f, 0.25365f, 0.23406f, 0.21521f, 0.19719f, 0.18008f, 0.16396f, 0.14888f,
//  0.13491f, 0.12208f, 0.11043f, 0.09998f, 0.09074f, 0.08272f, 0.07592f, 0.07032f, 0.06590f, 0.06262f,
//  0.06045f, 0.05934f, 0.05924f, 0.06009f, 0.06183f, 0.06439f, 0.06770f, 0.07167f, 0.07623f, 0.08128f,
//  0.08675f, 0.09254f, 0.09855f, 0.10470f, 0.11089f, 0.11703f, 0.12303f, 0.12880f, 0.13426f, 0.13932f
//};




/* 输出数据数组 */
static int16_t adcFirData[300];
  


/* Array of filter coefficients B (feed-forward taps) in Q1.15 format */

#ifdef	Fmac5

static int16_t aFilterCoeffB[COEFF_VECTOR_B_SIZE] =
{
    2212,  8848, 13272,  8848,  2212
};

#endif
/* Array of input values in Q1.15 format (in four parts in order to write new data during the calculation) */





#if 0


#include <stdint.h>

int16_t aInputValuesIn[] = {
    0, 0, 0, 0, 304, 1686, 2694, 2996, 2519, 1415,
    144, 0, 0, 0, 0, 0, 37, 1352, 2450, 2895,
    2600, 1653, 357, 0, 0, 0, 0, 0, 1012, 2165,
    2764, 2646, 1847, 629, 0, 0, 0, 0, 0, 0,
    681, 1884, 2604, 2643, 2016, 904, 0, 0, 0, 0,
    0, 0, 383, 1579, 2407, 2609, 2140, 1139, 56, 0,
    0, 0, 0, 0, 111, 1278, 2204, 2535, 2220, 1356,
    228, 0, 0, 0, 0, 0, 0, 990, 1967, 2434,
    2270, 1527, 445, 0, 0, 0, 0, 0, 0, 705,
    1728, 2306, 2279, 1682, 684, 0, 0, 0, 0, 0,
    0, 444, 1474, 2149, 2268, 1799, 0, 0, 0, 0,
    0, 0, 197, 1219, 1982, 2215, 1882, 1092, 118, 0,
    0, 0, 0, 0, 4, 972, 1787, 2144, 1944, 1247,
    291, 0, 0, 0, 0, 0, 0, 725, 1592, 2044,
    1959, 1389, 498, 0, 0, 0, 0, 0, 0, 504,
    1382, 1920, 1961, 1502, 679, 0, 0, 0, 0, 0,
    0, 283, 1168, 1782, 1930, 1591, 864, 30, 0, 0,
    0, 0, 0, 108, 956, 1623, 1880, 1652, 1007, 167,
    0, 0, 0, 0, 0, 0, 739, 1464, 1679, 1144,
    346, 0, 0, 0, 0, 0, 0, 542, 1287, 1712,
    1695, 1254, 511, 0, 0, 0, 0, 0, 0, 351,
    1106, 1604, 1679, 1331, 676, 0, 0, 0, 0, 0,
    0, 205, 927, 1475, 1648, 1400, 807, 71, 0, 0,
    0, 0, 0, 110, 750, 1343, 1596, 1437, 930, 220,
    0, 0, 0, 0, 0, 59, 574, 1194, 1521, 1461,
    1038, 367, 0, 0, 0, 0, 0, 24, 399, 1043,
    1439, 1462, 1120, 520, 0, 0, 0, 0, 0, 0,
    892, 1331, 1442, 1188, 646, 3, 0, 0, 0, 0,
    0, 194, 731, 1225, 1408, 1227, 757, 126, 0, 0,
    0, 0, 0, 152, 584, 1103, 1351, 1256, 854, 255,
    0, 0, 0, 0, 0, 122, 435, 976, 1290, 1263,
    931, 388, 0, 0, 0, 0, 0, 97, 319, 847,
    1203, 1264, 1000, 502, 0, 0, 0, 0, 0, 82,
    250, 711, 1118, 1236, 1039, 604, 50, 0, 0, 0,
    0, 63, 216, 588, 1018, 1197, 1077, 694, 155, 0,
    0, 0, 52, 188, 455, 913, 1151, 1092, 771, 276,
    0, 0, 0, 0, 42, 161, 349, 799, 1087, 1097,
    831, 382, 0, 0, 0, 0, 32, 148, 286, 687,
    1016, 1086, 879, 480, 0, 0, 0, 0, 20, 127,
    251, 572, 927, 1059, 924, 569, 87, 0, 0, 0,
    3, 120, 228, 453, 842, 1026, 938, 639, 198, 0,
    0, 0, 0, 110, 197, 361, 748, 971, 954, 703,
    287, 0, 0, 0, 0, 99, 190, 303, 647, 920,
    951, 743, 377, 0, 0, 0, 94, 167, 277, 554,
    846, 933, 788, 460, 31, 0, 0, 0, 79, 165,
    254, 451, 776, 910, 812, 525, 127, 0, 0, 0,
    828, 214, 0, 0, 0, 59, 146, 220, 318, 610,
    828, 831, 631, 298, 0, 0, 0, 56, 142, 199,
    291, 526, 768, 821, 676, 374, 3, 0, 0, 39,
    131, 192, 269, 431, 707, 810, 703, 439, 88
};
#endif

#if 1

int16_t aInputValuesIn[] = {

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 4, 0, 0, 0, 3, 740, 1460, 1860,
    1509, 1237, 903, 490, 0, 0, 0, 2, 4, 63,
    664, 613, 487, 375, 72, 0, 0, 1, 1, 4,
    0, 0, 13, 3, 4, 4, 0, 0, 43, 158,
    244, 268, 252, 212, 127, 94, 54, 32, 33, 43,
    55, 84, 92, 86, 67, 61, 46, 0, 146, 151,
    129, 98, 90, 76, 30, 3, 2, 4, 0, 0,
    0, 2, 4, 0, 0, 0, 175, 286, 248, 191,
    154, 111, 103, 102, 88, 80, 77, 71, 63, 72,
    65, 55, 55, 63, 68, 0, 48, 133, 134, 110,
    94, 41, 19, 1, 2, 0, 0, 0, 1, 3,
    4, 0, 0, 0, 43, 134, 152, 137, 130, 125,
    111, 92, 88, 81, 75, 71, 68, 63, 66, 57,
    60, 58, 40, 0, 211, 224, 151, 110, 60, 40,
    21, 3, 2, 0, 0, 0, 1, 2, 0, 0,
    0, 0, 94, 158, 214, 196, 169, 147, 123, 92,
    78, 75, 69, 66, 70, 68, 62, 56, 57, 69,
    62, 0, 161, 143, 115, 79, 69, 53, 37, 3,
    4, 0, 0, 0, 1, 4, 1, 0, 0, 1,
    179, 278, 260, 196, 149, 123, 103, 100, 88, 77,
    75, 71, 78, 68, 65, 63, 63, 68, 73, 0,
    41, 121, 132, 110, 93, 67, 15, 2, 4, 0,
    0, 0, 1, 4, 2, 0, 0, 3, 63, 159,
    168, 144, 139, 125, 122, 116, 88, 81, 75, 71,
    76, 72, 74, 61, 62, 63, 29, 6, 193, 213,
    154, 112, 88, 34, 23, 2, 4, 0, 0, 0,
    2, 4, 4, 0, 0, 1, 76, 152, 204, 211,
    163, 143, 124, 94, 84, 77, 71, 76, 72, 80,
    65, 57, 55, 63, 56, 0, 21, 155, 124, 95,
    66, 45, 27, 4, 0, 0, 0, 2, 2, 4,
    0, 0, 0, 3, 191, 300, 340, 201, 149, 127,
    114, 97, 82, 79, 70, 86, 76, 65, 67, 63,
    62, 63, 68, 0, 31, 114, 127, 116, 92, 69,
    22, 4, 2, 0, 0, 3, 2, 4, 0, 0,
    1, 3, 87, 116, 173, 141, 131, 124, 116, 112,
    96, 87, 79, 79, 76, 70, 65, 55, 60, 63,
    68, 21, 167, 226, 158, 120, 85, 35, 19, 4,
    0, 0, 0, 1, 4, 2, 0, 0, 1, 2,
    30, 152, 197, 201, 159, 143, 126, 120, 90, 79,
    78, 76, 73, 68, 69, 58, 63, 68, 48, 0,
    35, 156, 111, 84, 59, 57, 31, 20, 0, 0,
    0, 2, 4, 4, 0, 0, 1, 3, 212, 319,
    358, 197, 150, 127, 116, 93, 84, 83, 76, 73,
    72, 71, 69, 59, 63, 72, 74, 0, 23, 105,
    124, 112, 93, 69, 39, 0, 0, 0, 2, 0,
    4, 0, 0, 0, 1, 4, 92, 152, 175, 139,
    134, 127, 127, 113, 101, 86, 79, 76, 72, 65,
    63, 61, 62, 69, 70, 27, 151, 234, 212, 120,
    87, 61, 22, 2, 0, 0, 1, 2, 4, 0,
    0, 0, 3, 6, 24, 136, 177, 175, 166, 148
};   



#endif



#if 0

int16_t aInputValuesIn[] = {
    729, 710, 794, 1065, 2828, 4067, 4073, 3964,
    2478, 1371, 1079, 976, 974, 1039, 1273, 1652,
    2070, 2025, 1620, 1358, 1231, 1197, 1202, 1230,
    1325, 1456, 1543, 1541, 1484, 1396, 1327, 1311,
    1316, 1343, 1399, 1471, 1514, 1514, 1485, 1444,
    1408, 1383, 1392, 1414, 1446, 1475, 1501, 1503,
    1483, 1464, 1446, 1431, 1437, 1449, 1483, 1500,
    1503, 1494, 1483, 1476, 1474, 1467, 1480, 1496,
    1499, 1504, 1508, 1501, 1491, 1485, 1487, 1486,
    1493, 1501, 1503, 1501, 1507, 1510, 1495, 1497,
    1500, 1502, 1503, 1512, 1518, 1518, 1511, 1516,
    1518, 1515, 1517, 1522, 1515, 1518, 1524, 1522,
    1515, 1520, 1524, 1518, 1518, 1521, 1526, 1519,
    1528, 1536, 1536, 1536, 1540, 1540, 1536, 1538,
    1550, 1539, 1538, 1545, 1542, 1537, 1542, 1544,
    1539, 1541, 1544, 1545, 1539, 1544, 1546, 1543,
    1541, 1548, 1548, 1545, 1552, 1556, 1551, 1553,
    1560, 1551, 1556, 1564, 1558, 1558, 1560, 1567,
    1559, 1568, 1567, 1563, 1563, 1565, 1567, 1563,
    1568, 1566, 1566, 1567, 1576, 1575, 1571, 1576,
    1582, 1575, 1577, 1581, 1583, 1575, 1584, 1588,
    1582, 1584, 1586, 1586, 1583, 1586, 1592, 1586,
    1587, 1589, 1590, 1583, 1588, 1594, 1590, 1590,
    1596, 1599, 1591, 1594, 1598, 1595, 1597, 1599,
    1599, 1599, 1606, 1606, 1598, 1607, 1609, 1610,
    1603, 1609, 1614, 1607, 1605, 1612, 1612, 1607,
    1612, 1614, 1611, 1613, 1617, 1615, 1615, 1620,
    1628, 1623, 1623, 1628, 1621, 1626, 1624, 1627,
    1626, 1628, 1628, 1625, 1626, 1630, 1627, 1627,
    1631, 1634, 1633, 1640, 1640, 1638, 1637, 1644,
    1645, 1639, 1642, 1646, 1643, 1639, 1644, 1646,
    1641, 1650, 1646, 1639, 1644, 1644, 1646, 1643,
    1650, 1654, 1651, 1652, 1657, 1662, 1653, 1658,
    1662, 1655, 1658, 1661, 1662, 1659, 1660, 1662,
    1662, 1663, 1663, 1668, 1661, 1663, 1670, 1663,
    1663, 1663, 1668, 1667, 1674, 1678, 1678, 1671,
    1684, 1684, 1679, 1684, 1688, 1679, 1681, 1688,
    1685, 1677, 1684, 1688, 1679, 1688, 1687, 1677,
    1685, 1692, 1694, 1691, 1689, 1694, 1691, 1690,
    1696, 1695, 1691, 1700, 1706, 1699, 1701, 1704,
    1710, 1707, 1709, 1714, 1711, 1705, 1708, 1708,
    1703, 1712, 1704, 1707, 1708, 1712, 1710, 1703,
    1712, 1716, 1714, 1712, 1720, 1722, 1715, 1722,
    1726, 1723, 1724, 1726, 1726, 1721, 1722, 1724,
    1719, 1721, 1726, 1727, 1723, 1736, 1734, 1728,
    1728, 1732, 1734, 1731, 1736, 1736, 1734, 1735,
    1738, 1740, 1735, 1736, 1742, 1735, 1734, 1741,
    1738, 1733, 1738, 1742, 1744, 1745, 1751, 1751,
    1746, 1750, 1747, 1749, 1752, 1751, 1747, 1753,
    1756, 1751, 1753, 1758, 1759, 1755, 1760, 1764,
    1751, 1755, 1758, 1758, 1755, 1760, 1762, 1759,
    1764, 1764, 1766, 1763, 1768, 1774, 1771, 1771,
    1780, 1773, 1771, 1776, 1778, 1775, 1773, 1778,
    1775, 1769, 1770, 1772, 1771, 1776, 1780, 1780,
    1775, 1784, 1782, 1778, 1779, 1785, 1783, 1779,
    1784, 1792, 1783, 1785, 1792, 1796, 1796, 1792,
    1800, 1800, 1795, 1796, 1800, 1801, 1793, 1800,
    1802, 1797, 1797, 1804, 1804, 1799, 1808, 1807,
    1807, 1808, 1814, 1813, 1807, 1810, 1818, 1807,
    1808, 1812, 1815, 1807, 1816, 1818, 1815, 1813,
    1820, 1820, 1813, 1818, 1822, 1822, 1824, 1823,
    1823, 1815, 1825, 1830, 1823, 1821, 1828, 1828,
    1819
};

#endif




#if 1




static int16_t aInputValuesInput[INPUT_ARRAY_1_SIZE+INPUT_ARRAY_2_SIZE+INPUT_ARRAY_3_SIZE+INPUT_ARRAY_4_SIZE] =
{
       0,  5276, -1548, 13844,     7, 17551,  5802, 16142, 14198, 12009,
   21624,  8678, 24576,  8672, 21611, 11990, 14172, 16111,  5765, 17510,
     -37, 13797, -1598,  5225,   -51, -5327,  1498,-13892,   -52,-17592,
   -5838,-16174,-14223,-12029,-21637, -8685,-24576, -8665,-21597,-11970,
  -14146,-16080, -5729,-17469,    82,-13749,  1647, -5174,   103,  5378,
   -1449, 13939,    96, 17632,  5874, 16205, 14249, 12048, 21650,  8691,
   24575,  8658, 21583, 11950, 14120, 16048,  5692, 17428,  -127, 13701,
   -1697,  5122,  -154, -5429,  1399,-13987,  -141,-17673, -5910,-16236,
  -14274,-12068,-21663, -8698,-24575, -8651,-21570,-11930,-14094,-16016,
   -5655,-17387,   171,-13654,  1747, -5071,   206,  5480, -1349, 14034,

     185, 17713,  5946, 16267, 14299, 12087, 21676,  8704, 24574,  8643,
   21556, 11909, 14067, 15984,  5618, 17346,  -216, 13606, -1797,  5020,
    -257, -5530,  1300,-14081,  -229,-17754, -5982,-16297,-14324,-12106,
  -21688, -8710,-24574, -8636,-21542,-11889,-14041,-15952, -5581,-17304,
     261,-13558,  1847, -4969,   309,  5581, -1250, 14128,   273, 17794,
    6018, 16328, 14349, 12124, 21701,  8715, 24573,  8628, 21527, 11868,
   14014, 15920,  5544, 17263,  -306, 13510, -1897,  4918,  -360, -5632,
    1201,-14176,  -317,-17834, -6053,-16358,-14374,-12143,-21713, -8721,
  -24571, -8620,-21513,-11847,-13988,-15888, -5507,-17221,   352,-13462,
    1947, -4867,   412,  5683, -1152, 14223,   361, 17874,  6089, 16389,
   14399, 12162, 21725,  8726, 24570,  8612, 21498, 11826, 13961, 15856,
    5470, 17180,  -397, 13414, -1997,  4816,  -463, -5734,  1102,-14270,

    -405,-17914, -6124,-16419,-14423,-12180,-21737, -8732,-24569, -8604,
  -21484,-11805,-13934,-15823, -5432,-17138,   442,-13366,  2047, -4764,
     515,  5785, -1053, 14317,   449, 17954,  6160, 16449, 14447, 12198,
   21749,  8737, 24567,  8596, 21469, 11784, 13907, 15790,  5395, 17096,
    -488, 13318, -2097,  4713,  -566, -5836,  1004,-14363,  -493,-17994,
   -6195,-16479,-14472,-12216,-21760, -8742,-24565, -8587,-21454,-11763,
  -13879,-15758, -5357,-17054,   533,-13269,  2147, -4662,   618,  5886,
    -955, 14410,   536, 18033,  6230, 16509, 14496, 12234, 21772,  8747,
   24563,  8579, 21438, 11741, 13852, 15725,  5319, 17012,  -579, 13221,
   -2198,  4611,  -669, -5937,   905,-14457,  -580,-18073, -6265,-16538,
  -14520,-12252,-21783, -8751,-24561, -8570,-21423,-11720,-13824,-15692,

   -5282,-16970,   624,-13173,  2248, -4559,   720,  5988,  -856, 14504,
     623, 18112,  6299, 16568, 14543, 12270, 21794,  8756, 24559,  8561,
   21408, 11698, 13797, 15659,  5244, 16928,  -670, 13124, -2298,  4508,
    -772, -6038,   807,-14550,  -667,-18152, -6334,-16597,-14567,-12287,
  -21805, -8760,-24557, -8552,-21392,-11676,-13769,-15625, -5205,-16886,
     716,-13076,  2348, -4457,   823,  6089,  -758, 14597,   710, 18191,
    6369, 16626, 14590, 12304, 21816,  8764, 24554,  8542, 21376, 11654,
   13741, 15592,  5167, 16843,  -761, 13027, -2399,  4405,  -875, -6140,
     709,-14643,  -753,-18230, -6403,-16656,-14614,-12321,-21827, -8768,
  -24551, -8533,-21360,-11632,-13713,-15559, -5129,-16801,   807,-12979,
    2449, -4354,   926,  6190,  -660, 14690,   796, 18269,  6437, 16685,
   14637, 12338, 21837,  8772, 24548,  8523, 21344, 11609, 13684, 15525,
    5090, 16758,  -853, 12930, -2500,  4303,  -977, -6241,   612,-14736,
    -839,-18308, -6472,-16713,-14660,-12355,-21847, -8776,-24545, -8514,
  -21328,-11587,-13656,-15491, -5052,-16715,   899,-12882,  2550, -4251,
    1029,  6291,  -563, 14782,   882, 18347,  6506, 16742, 14683, 12372,
   21858,  8779, 24542,  8504, 21311, 11564, 13628, 15457,  5013, 16673,
    -946, 12833, -2600,  4200, -1080, -6342,   514,-14828,  -925,-18386,
   -6540,-16771
};


int16_t*	aInputValues1;
int16_t*	aInputValues2;//=	(int16_t*)(aInputValues1+INPUT_ARRAY_1_SIZE);//sizeof(int16_t));
int16_t*	aInputValues3;//=	(int16_t*)(aInputValues1+(INPUT_ARRAY_1_SIZE+INPUT_ARRAY_2_SIZE));
int16_t*	aInputValues4;//=	(int16_t*)(aInputValues1+(INPUT_ARRAY_1_SIZE+INPUT_ARRAY_2_SIZE+INPUT_ARRAY_3_SIZE));

#else
static int16_t aInputValues1[INPUT_ARRAY_1_SIZE] =
{
       0,  5276, -1548, 13844,     7, 17551,  5802, 16142, 14198, 12009,
   21624,  8678, 24576,  8672, 21611, 11990, 14172, 16111,  5765, 17510,
     -37, 13797, -1598,  5225,   -51, -5327,  1498,-13892,   -52,-17592,
   -5838,-16174,-14223,-12029,-21637, -8685,-24576, -8665,-21597,-11970,
  -14146,-16080, -5729,-17469,    82,-13749,  1647, -5174,   103,  5378,
   -1449, 13939,    96, 17632,  5874, 16205, 14249, 12048, 21650,  8691,
   24575,  8658, 21583, 11950, 14120, 16048,  5692, 17428,  -127, 13701,
   -1697,  5122,  -154, -5429,  1399,-13987,  -141,-17673, -5910,-16236,
  -14274,-12068,-21663, -8698,-24575, -8651,-21570,-11930,-14094,-16016,
   -5655,-17387,   171,-13654,  1747, -5071,   206,  5480, -1349, 14034,
};
static int16_t aInputValues2[INPUT_ARRAY_2_SIZE] =
{
     185, 17713,  5946, 16267, 14299, 12087, 21676,  8704, 24574,  8643,
   21556, 11909, 14067, 15984,  5618, 17346,  -216, 13606, -1797,  5020,
    -257, -5530,  1300,-14081,  -229,-17754, -5982,-16297,-14324,-12106,
  -21688, -8710,-24574, -8636,-21542,-11889,-14041,-15952, -5581,-17304,
     261,-13558,  1847, -4969,   309,  5581, -1250, 14128,   273, 17794,
    6018, 16328, 14349, 12124, 21701,  8715, 24573,  8628, 21527, 11868,
   14014, 15920,  5544, 17263,  -306, 13510, -1897,  4918,  -360, -5632,
    1201,-14176,  -317,-17834, -6053,-16358,-14374,-12143,-21713, -8721,
  -24571, -8620,-21513,-11847,-13988,-15888, -5507,-17221,   352,-13462,
    1947, -4867,   412,  5683, -1152, 14223,   361, 17874,  6089, 16389,
   14399, 12162, 21725,  8726, 24570,  8612, 21498, 11826, 13961, 15856,
    5470, 17180,  -397, 13414, -1997,  4816,  -463, -5734,  1102,-14270,
};
static int16_t aInputValues3[INPUT_ARRAY_3_SIZE] =
{
    -405,-17914, -6124,-16419,-14423,-12180,-21737, -8732,-24569, -8604,
  -21484,-11805,-13934,-15823, -5432,-17138,   442,-13366,  2047, -4764,
     515,  5785, -1053, 14317,   449, 17954,  6160, 16449, 14447, 12198,
   21749,  8737, 24567,  8596, 21469, 11784, 13907, 15790,  5395, 17096,
    -488, 13318, -2097,  4713,  -566, -5836,  1004,-14363,  -493,-17994,
   -6195,-16479,-14472,-12216,-21760, -8742,-24565, -8587,-21454,-11763,
  -13879,-15758, -5357,-17054,   533,-13269,  2147, -4662,   618,  5886,
    -955, 14410,   536, 18033,  6230, 16509, 14496, 12234, 21772,  8747,
   24563,  8579, 21438, 11741, 13852, 15725,  5319, 17012,  -579, 13221,
   -2198,  4611,  -669, -5937,   905,-14457,  -580,-18073, -6265,-16538,
  -14520,-12252,-21783, -8751,-24561, -8570,-21423,-11720,-13824,-15692,
};
static int16_t aInputValues4[INPUT_ARRAY_4_SIZE] =
{
   -5282,-16970,   624,-13173,  2248, -4559,   720,  5988,  -856, 14504,
     623, 18112,  6299, 16568, 14543, 12270, 21794,  8756, 24559,  8561,
   21408, 11698, 13797, 15659,  5244, 16928,  -670, 13124, -2298,  4508,
    -772, -6038,   807,-14550,  -667,-18152, -6334,-16597,-14567,-12287,
  -21805, -8760,-24557, -8552,-21392,-11676,-13769,-15625, -5205,-16886,
     716,-13076,  2348, -4457,   823,  6089,  -758, 14597,   710, 18191,
    6369, 16626, 14590, 12304, 21816,  8764, 24554,  8542, 21376, 11654,
   13741, 15592,  5167, 16843,  -761, 13027, -2399,  4405,  -875, -6140,
     709,-14643,  -753,-18230, -6403,-16656,-14614,-12321,-21827, -8768,
  -24551, -8533,-21360,-11632,-13713,-15559, -5129,-16801,   807,-12979,
    2449, -4354,   926,  6190,  -660, 14690,   796, 18269,  6437, 16685,
   14637, 12338, 21837,  8772, 24548,  8523, 21344, 11609, 13684, 15525,
    5090, 16758,  -853, 12930, -2500,  4303,  -977, -6241,   612,-14736,
    -839,-18308, -6472,-16713,-14660,-12355,-21847, -8776,-24545, -8514,
  -21328,-11587,-13656,-15491, -5052,-16715,   899,-12882,  2550, -4251,
    1029,  6291,  -563, 14782,   882, 18347,  6506, 16742, 14683, 12372,
   21858,  8779, 24542,  8504, 21311, 11564, 13628, 15457,  5013, 16673,
    -946, 12833, -2600,  4200, -1080, -6342,   514,-14828,  -925,-18386,
   -6540,-16771
};



#endif




/* Array of calculated filtered data in Q1.15 format (two parts) */
static int16_t aCalculatedFilteredData1[OUTPUT_ARRAY_1_SIZE];
static int16_t aCalculatedFilteredData2[OUTPUT_ARRAY_2_SIZE];

/* Expected number of calculated samples for the used aCalculatedFilteredDataX */
uint16_t CurrentInputArraySize;

/* Expected number of calculated samples for the used aCalculatedFilteredDataX */
uint16_t ExpectedCalculatedFilteredDataSize;

/* Status of the calculation */
__IO uint32_t FilterConfigCallbackCount    = 0;
__IO uint32_t FilterPreloadCallbackCount   = 0;
__IO uint32_t HalfGetDataCallbackCount     = 0;
__IO uint32_t GetDataCallbackCount         = 0;
__IO uint32_t OutputDataReadyCallbackCount = 0;
__IO uint32_t ErrorCount                   = 0;

/* Array of reference filtered data for FIR "5 feed-forward taps, gain = 1" in Q1.15 format */
static const int16_t aRefFilteredData[ARRAY_SIZE] =
{
#if defined(CLIP_ENABLED)
  0x2370, 0x3498, 0x447e, 0x539a, 0x60ed, 0x6cdb, 0x76c6, 0x7ea8, 0x7fff, 0x7fff,
  0x7fff, 0x7fff, 0x7fff, 0x7e7e, 0x768f, 0x6c98, 0x609e, 0x5343, 0x441f, 0x3432,
  0x2306, 0x1185, 0xff92, 0xeda0, 0xdc24, 0xcb01, 0xbb21, 0xac0d, 0x9ec4, 0x92e1,
  0x8902, 0x812d, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x81ac, 0x89a7, 0x93aa,
  0x9fae, 0xad14, 0xbc40, 0xcc33, 0xdd63, 0xeee7, 0x00dc, 0x12cc, 0x2444, 0x3563,
  0x453c, 0x5448, 0x6188, 0x6d61, 0x7734, 0x7efb, 0x7fff, 0x7fff, 0x7fff, 0x7fff,
  0x7fff, 0x7e28, 0x7620, 0x6c11, 0x6002, 0x5293, 0x435f, 0x3366, 0x2230, 0x10aa,
  0xfeb5, 0xecc5, 0xdb50, 0xca36, 0xba63, 0xab5f, 0x9e2a, 0x925c, 0x8895, 0x80da,
  0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8201, 0x8a17, 0x9432, 0xa04c, 0xadc4,
  0xbcff, 0xccff, 0xde39, 0xefc3, 0x01b9, 0x13a7, 0x251a, 0x362f, 0x45fb, 0x54f6,
  0x6222, 0x6de5, 0x77a0, 0x7f4e, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7dd1,
  0x75af, 0x6b88, 0x5f64, 0x51e3, 0x42a0, 0x329a, 0x215b, 0x0fcf, 0xfdd9, 0xebec,
  0xda7c, 0xc96c, 0xb9a6, 0xaab2, 0x9d90, 0x91d9, 0x882a, 0x8089, 0x8000, 0x8000,
  0x8000, 0x8000, 0x8000, 0x8259, 0x8a88, 0x94bb, 0xa0eb, 0xae75, 0xbdc0, 0xcdcc,
  0xdf0f, 0xf09e, 0x0295, 0x1481, 0x25ed, 0x36f8, 0x46b7, 0x55a3, 0x62bc, 0x6e68,
  0x780b, 0x7f9e, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7d79, 0x753d, 0x6aff,
  0x5ec5, 0x5132, 0x41df, 0x31cc, 0x2085, 0x0ef4, 0xfcfc, 0xeb11, 0xd9a8, 0xc8a1,
  0xb8e9, 0xaa06, 0x9cf8, 0x9156, 0x87bf, 0x8039, 0x8000, 0x8000, 0x8000, 0x8000,
  0x8000, 0x82b2, 0x8afa, 0x9545, 0xa189, 0xaf27, 0xbe82, 0xce9a, 0xdfe5, 0xf179,
  0x0371, 0x155b, 0x26c1, 0x37c2, 0x4774, 0x564f, 0x6354, 0x6eeb, 0x7875, 0x7fef,
  0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7d20, 0x74cb, 0x6a75, 0x5e26, 0x5080,
  0x411d, 0x30fe, 0x1faf, 0x0e19, 0xfc20, 0xea37, 0xd8d4, 0xc7d8, 0xb82d, 0xa95b,
  0x9c60, 0x90d4, 0x8756, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x830c,
  0x8b6e, 0x95d0, 0xa229, 0xafd9, 0xbf43, 0xcf67, 0xe0bc, 0xf254, 0x044e, 0x1635,
  0x2794, 0x388c, 0x4830, 0x56fa, 0x63ec, 0x6f6c, 0x78dd, 0x7fff, 0x7fff, 0x7fff,
  0x7fff, 0x7fff, 0x7fff, 0x7cc6, 0x7457, 0x69e9, 0x5d85, 0x4fcc, 0x405a, 0x3030,
  0x1ed8, 0x0d3d, 0xfb43, 0xe95e, 0xd802, 0xc70f, 0xb771, 0xa8b0, 0x9bc8, 0x9053,
  0x86ee, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8367, 0x8be3, 0x965c,
  0xa2ca, 0xb08d, 0xc006, 0xd036, 0xe193, 0xf32f, 0x052a, 0x170e, 0x2866, 0x3953,
  0x48ea, 0x57a4, 0x6482, 0x6fec, 0x7945, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff,
  0x7fff, 0x7c6a, 0x73e2, 0x695d, 0x5ce3, 0x4f18, 0x3f97, 0x2f61, 0x1e00, 0x0c62,
  0xfa67, 0xe884, 0xd72e, 0xc646, 0xb6b6, 0xa806, 0x9b33, 0x8fd4, 0x8687, 0x8000,
  0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x83c3, 0x8c58, 0x96e9, 0xa36c, 0xb140,
  0xc0c9, 0xd105, 0xe26a, 0xf40b, 0x0606, 0x17e7, 0x293a, 0x3a1c, 0x49a5, 0x584c,
  0x6516, 0x706a, 0x79ab, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7c0e,
  0x736c, 0x68d0, 0x5c42, 0x4e64, 0x3ed4, 0x2e92, 0x1d29, 0x0b86, 0xf98b, 0xe7ab,
  0xd65d, 0xc57f, 0xb5fd, 0xa75d, 0x9a9e, 0x8f56, 0x8622, 0x8000, 0x8000, 0x8000,
  0x8000, 0x8000, 0x8000, 0x8421, 0x8ccf, 0x9777, 0xa410, 0xb1f6, 0xc18d, 0xd1d4,
  0xe341, 0xf4e6, 0x06e2, 0x18c0, 0x2a0c, 0x3ae4, 0x4a5f, 0x58f6, 0x65ab, 0x70e7,
  0x7a0f, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7baf, 0x72f4, 0x6840,
  0x5b9e, 0x4daf, 0x3e10, 0x2dc2, 0x1c51, 0x0aa9, 0xf8ae, 0xe6d2, 0xd58a, 0xc4b8,
  0xb544, 0xa6b6, 0x9a0a, 0x8ed8, 0x85bd, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
  0x8000, 0x847f, 0x8d47, 0x9806, 0xa4b3, 0xb2ab, 0xc251, 0xd2a4, 0xe419, 0xf5c3,
  0x07bf, 0x1999, 0x2add, 0x3bab, 0x4b17, 0x599c, 0x663e, 0x7164, 0x7a73, 0x7fff,
  0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7b4f, 0x727b, 0x67b0, 0x5af9, 0x4cf8,
  0x3d4b, 0x2cf2, 0x1b7a, 0x09cf, 0xf7d3, 0xe5fa, 0xd4ba, 0xc3f2, 0xb48c, 0xa60f,
  0x9978, 0x8e5d, 0x855b, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x84e0,
  0x8dc1, 0x9897, 0xa558, 0xb362, 0xc316, 0xd374, 0xe4f1, 0xf69f, 0x089b, 0x1a71,
  0x2bae, 0x3c70, 0x4bcf, 0x5a43, 0x66d1, 0x71e0, 0x7ad6, 0x7fff, 0x7fff, 0x7fff,
  0x7fff, 0x7fff, 0x7fff, 0x7aee, 0x7201, 0x6720, 0x5a54, 0x4c41, 0x3c85, 0x2c22,
  0x1aa2, 0x08f3, 0xf6f6, 0xe522, 0xd3e9, 0xc32b, 0xb3d4, 0xa568
#else
  0x2370, 0x3498, 0x447e, 0x539a, 0x60ed, 0x6cdb, 0x76c6, 0x7ea8, 0x8483, 0x87d3,
  0x892a, 0x87c5, 0x8467, 0x7e7e, 0x768f, 0x6c98, 0x609e, 0x5343, 0x441f, 0x3432,
  0x2306, 0x1185, 0xff92, 0xeda0, 0xdc24, 0xcb01, 0xbb21, 0xac0d, 0x9ec4, 0x92e1,
  0x8902, 0x812d, 0x7b60, 0x781d, 0x76d5, 0x7848, 0x7bb5, 0x81ac, 0x89a7, 0x93aa,
  0x9fae, 0xad14, 0xbc40, 0xcc33, 0xdd63, 0xeee7, 0x00dc, 0x12cc, 0x2444, 0x3563,
  0x453c, 0x5448, 0x6188, 0x6d61, 0x7734, 0x7efb, 0x84bb, 0x87ef, 0x8929, 0x87a7,
  0x842c, 0x7e28, 0x7620, 0x6c11, 0x6002, 0x5293, 0x435f, 0x3366, 0x2230, 0x10aa,
  0xfeb5, 0xecc5, 0xdb50, 0xca36, 0xba63, 0xab5f, 0x9e2a, 0x925c, 0x8895, 0x80da,
  0x7b28, 0x7802, 0x76d6, 0x7867, 0x7bef, 0x8201, 0x8a17, 0x9432, 0xa04c, 0xadc4,
  0xbcff, 0xccff, 0xde39, 0xefc3, 0x01b9, 0x13a7, 0x251a, 0x362f, 0x45fb, 0x54f6,
  0x6222, 0x6de5, 0x77a0, 0x7f4e, 0x84f2, 0x880a, 0x8927, 0x8788, 0x83f1, 0x7dd1,
  0x75af, 0x6b88, 0x5f64, 0x51e3, 0x42a0, 0x329a, 0x215b, 0x0fcf, 0xfdd9, 0xebec,
  0xda7c, 0xc96c, 0xb9a6, 0xaab2, 0x9d90, 0x91d9, 0x882a, 0x8089, 0x7af2, 0x77e8,
  0x76d9, 0x7886, 0x7c2b, 0x8259, 0x8a88, 0x94bb, 0xa0eb, 0xae75, 0xbdc0, 0xcdcc,
  0xdf0f, 0xf09e, 0x0295, 0x1481, 0x25ed, 0x36f8, 0x46b7, 0x55a3, 0x62bc, 0x6e68,
  0x780b, 0x7f9e, 0x8527, 0x8823, 0x8923, 0x8767, 0x83b4, 0x7d79, 0x753d, 0x6aff,
  0x5ec5, 0x5132, 0x41df, 0x31cc, 0x2085, 0x0ef4, 0xfcfc, 0xeb11, 0xd9a8, 0xc8a1,
  0xb8e9, 0xaa06, 0x9cf8, 0x9156, 0x87bf, 0x8039, 0x7abd, 0x77d1, 0x76df, 0x78a9,
  0x7c6a, 0x82b2, 0x8afa, 0x9545, 0xa189, 0xaf27, 0xbe82, 0xce9a, 0xdfe5, 0xf179,
  0x0371, 0x155b, 0x26c1, 0x37c2, 0x4774, 0x564f, 0x6354, 0x6eeb, 0x7875, 0x7fef,
  0x855c, 0x883a, 0x891d, 0x8745, 0x8375, 0x7d20, 0x74cb, 0x6a75, 0x5e26, 0x5080,
  0x411d, 0x30fe, 0x1faf, 0x0e19, 0xfc20, 0xea37, 0xd8d4, 0xc7d8, 0xb82d, 0xa95b,
  0x9c60, 0x90d4, 0x8756, 0x7fea, 0x7a8a, 0x77b9, 0x76e4, 0x78cb, 0x7ca8, 0x830c,
  0x8b6e, 0x95d0, 0xa229, 0xafd9, 0xbf43, 0xcf67, 0xe0bc, 0xf254, 0x044e, 0x1635,
  0x2794, 0x388c, 0x4830, 0x56fa, 0x63ec, 0x6f6c, 0x78dd, 0x803c, 0x858e, 0x8851,
  0x8918, 0x8723, 0x8337, 0x7cc6, 0x7457, 0x69e9, 0x5d85, 0x4fcc, 0x405a, 0x3030,
  0x1ed8, 0x0d3d, 0xfb43, 0xe95e, 0xd802, 0xc70f, 0xb771, 0xa8b0, 0x9bc8, 0x9053,
  0x86ee, 0x7f9c, 0x7a58, 0x77a4, 0x76ec, 0x78ef, 0x7ce8, 0x8367, 0x8be3, 0x965c,
  0xa2ca, 0xb08d, 0xc006, 0xd036, 0xe193, 0xf32f, 0x052a, 0x170e, 0x2866, 0x3953,
  0x48ea, 0x57a4, 0x6482, 0x6fec, 0x7945, 0x808a, 0x85c0, 0x8866, 0x890f, 0x86fd,
  0x82f5, 0x7c6a, 0x73e2, 0x695d, 0x5ce3, 0x4f18, 0x3f97, 0x2f61, 0x1e00, 0x0c62,
  0xfa67, 0xe884, 0xd72e, 0xc646, 0xb6b6, 0xa806, 0x9b33, 0x8fd4, 0x8687, 0x7f4f,
  0x7a27, 0x7790, 0x76f4, 0x7914, 0x7d2a, 0x83c3, 0x8c58, 0x96e9, 0xa36c, 0xb140,
  0xc0c9, 0xd105, 0xe26a, 0xf40b, 0x0606, 0x17e7, 0x293a, 0x3a1c, 0x49a5, 0x584c,
  0x6516, 0x706a, 0x79ab, 0x80d5, 0x85f0, 0x8879, 0x8906, 0x86d8, 0x82b4, 0x7c0e,
  0x736c, 0x68d0, 0x5c42, 0x4e64, 0x3ed4, 0x2e92, 0x1d29, 0x0b86, 0xf98b, 0xe7ab,
  0xd65d, 0xc57f, 0xb5fd, 0xa75d, 0x9a9e, 0x8f56, 0x8622, 0x7f05, 0x79f8, 0x777d,
  0x76fe, 0x793b, 0x7d6c, 0x8421, 0x8ccf, 0x9777, 0xa410, 0xb1f6, 0xc18d, 0xd1d4,
  0xe341, 0xf4e6, 0x06e2, 0x18c0, 0x2a0c, 0x3ae4, 0x4a5f, 0x58f6, 0x65ab, 0x70e7,
  0x7a0f, 0x811f, 0x861e, 0x888b, 0x88fb, 0x86b0, 0x8270, 0x7baf, 0x72f4, 0x6840,
  0x5b9e, 0x4daf, 0x3e10, 0x2dc2, 0x1c51, 0x0aa9, 0xf8ae, 0xe6d2, 0xd58a, 0xc4b8,
  0xb544, 0xa6b6, 0x9a0a, 0x8ed8, 0x85bd, 0x7ebb, 0x79ca, 0x776b, 0x770a, 0x7963,
  0x7db1, 0x847f, 0x8d47, 0x9806, 0xa4b3, 0xb2ab, 0xc251, 0xd2a4, 0xe419, 0xf5c3,
  0x07bf, 0x1999, 0x2add, 0x3bab, 0x4b17, 0x599c, 0x663e, 0x7164, 0x7a73, 0x8168,
  0x864b, 0x889b, 0x88ef, 0x8687, 0x822b, 0x7b4f, 0x727b, 0x67b0, 0x5af9, 0x4cf8,
  0x3d4b, 0x2cf2, 0x1b7a, 0x09cf, 0xf7d3, 0xe5fa, 0xd4ba, 0xc3f2, 0xb48c, 0xa60f,
  0x9978, 0x8e5d, 0x855b, 0x7e73, 0x799d, 0x775b, 0x7716, 0x798c, 0x7df6, 0x84e0,
  0x8dc1, 0x9897, 0xa558, 0xb362, 0xc316, 0xd374, 0xe4f1, 0xf69f, 0x089b, 0x1a71,
  0x2bae, 0x3c70, 0x4bcf, 0x5a43, 0x66d1, 0x71e0, 0x7ad6, 0x81b1, 0x8678, 0x88ab,
  0x88e2, 0x865d, 0x81e5, 0x7aee, 0x7201, 0x6720, 0x5a54, 0x4c41, 0x3c85, 0x2c22,
  0x1aa2, 0x08f3, 0xf6f6, 0xe522, 0xd3e9, 0xc32b, 0xb3d4, 0xa568
#endif /* CLIP_ENABLED */
};

/* Auxiliary counter */
uint32_t Index;

/* Private function prototypes -----------------------------------------------*/
static void MX_DMA_Init(void);
static void MX_FMAC_Init(void);

__weak void Error_Handler(void)
{
}	

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

API_FMAC_MEMDEF		FmacMem={0};

// 调用示例

API_FMAC_MEMDEF*	API_FMAC_GetMemAddress(void)
{
		return	&FmacMem;
	
}	
//默认值	

		

//					aInputValues2=	(int16_t*)(aInputValues1+INPUT_ARRAY_1_SIZE);//sizeof(int16_t));
//					aInputValues3=	(int16_t*)(aInputValues1+(INPUT_ARRAY_1_SIZE+INPUT_ARRAY_2_SIZE));
//					aInputValues4=	(int16_t*)(aInputValues1+(INPUT_ARRAY_1_SIZE+INPUT_ARRAY_2_SIZE+INPUT_ARRAY_3_SIZE));
//		
		


// Q1.15格式信号数组 (16位有符号整数)
//int16_t adcInputValues[NUM_SAMPLES];

#include <math.h>
#include	"API_GPIO.H"
//void	 API_FMAC_INIT(void)
	
void	API_FMAC_RestConfig(void)
{
	
//		API_FMAC_GetMemAddress(FMAC_MEM);
//    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//    HAL_Init();

//    /* System Clock configure */
//    HAL_SystemClocks_Config(FW_SYSCLK);
//    
//    /* UART configure (for printf function) */
//    HAL_UART_Config();
//    
//    /* Default UART output */
//    printf("Hello World\n");
//    generate_mixed_signal();
    /* Configure LED */
  
//    HAL_BSP_LED_On();
//    HAL_BSP_LED_Off();  
	
//		FmacMem.X1=(int16_t*)aInputValuesInput;//aInputValuesIn;
		FmacMem.X2=(int16_t*)aFilterCoeffB;	//aFilterCoeffB;//
		FmacMem.coeffBsize=COEFF_VECTOR_B_SIZE;
//		FmacMem.Y=(int16_t*)adcFirData;
//		FmacMem.outSize	=ARRAY_SIZE;
	
	
  
    /* Initialize all configured peripherals */
    MX_DMA_Init();
    MX_FMAC_Init();

    /*## Configure the FMAC peripheral ###########################################*/



		sFmacConfig.InputBaseAddress  = INPUT_BUFFER_BASE;
    sFmacConfig.InputBufferSize   = INPUT_BUFFER_SIZE;
    sFmacConfig.InputThreshold    = INPUT_THRESHOLD;
//    sFmacConfig.InputBaseAddress  = 5;                       //输入缓冲区 5：地址5~104
//    sFmacConfig.InputBufferSize   = 127;                     //输入缓冲区 127：容量计算=系数长度N(5)+吞吐优化预留空间D1(本例D1=122)
//    sFmacConfig.InputThreshold    = INPUT_THRESHOLD;         //输入缓冲区 1：使能了DMA写请求，阈值为1

    sFmacConfig.CoeffBaseAddress  = COEFFICIENT_BUFFER_BASE;
    sFmacConfig.CoeffBufferSize   = COEFFICIENT_BUFFER_SIZE;
//    sFmacConfig.CoeffBaseAddress  = 0;                       //系数缓冲区 0：地址0~4
//    sFmacConfig.CoeffBufferSize   = 5;                       //系数缓冲区 5：5个元素
    
    sFmacConfig.OutputBaseAddress = OUTPUT_BUFFER_BASE;      
    sFmacConfig.OutputBufferSize  = OUTPUT_BUFFER_SIZE;
    sFmacConfig.OutputThreshold   = OUTPUT_THRESHOLD;
//    sFmacConfig.OutputBaseAddress = 132;                     //输出缓冲区 5：地址132~254     
//    sFmacConfig.OutputBufferSize  = 123;                     //输出缓冲区 5：容量计算=吞吐优化预留空间D2(本例D2=123，最小1) 
//    sFmacConfig.OutputThreshold   = OUTPUT_THRESHOLD;        //输出缓冲区 5：地址5~104     
  
    
    sFmacConfig.pCoeffA           = NULL;
    sFmacConfig.CoeffASize        = 0;
//    sFmacConfig.pCoeffB           = aFilterCoeffB;
        //180K 4阶 FIR低通滤波
    sFmacConfig.CoeffBSize        = COEFF_VECTOR_B_SIZE;     //4阶=5个系数
    sFmacConfig.Filter            = FMAC_FUNC_CONVO_FIR;     //FIR滤波器类型
    sFmacConfig.InputAccess       = FMAC_BUFFER_ACCESS_DMA;
    sFmacConfig.OutputAccess      = FMAC_BUFFER_ACCESS_DMA;

#if defined(CLIP_ENABLED)
    sFmacConfig.Clip              = FMAC_CLIP_ENABLED;
#else
    sFmacConfig.Clip              = FMAC_CLIP_DISABLED;
#endif

    sFmacConfig.P                 = COEFF_VECTOR_B_SIZE;     //4阶=5个系数
    sFmacConfig.Q                 = FILTER_PARAM_Q_NOT_USED;
    sFmacConfig.R                 = GAIN;

    sFmacConfig.pCoeffB           = FmacMem.X2;//(short*)&TestCoeff_Q1P15; 


}

void	API_FMAC_ChangeCoffeBsize(void)
{
	
		sFmacConfig.InputBaseAddress  = FmacMem.coeffBsize;	
    sFmacConfig.OutputBaseAddress = FmacMem.coeffBsize+INPUT_BUFFER_SIZE;   

    sFmacConfig.CoeffBSize        = FmacMem.coeffBsize;     //4阶=5个系数
    sFmacConfig.P                 = FmacMem.coeffBsize;     //4阶=5个系数
}


void	API_FMAC_Rest(void)
{
	

	
	aInputValues1=	FmacMem.X1;

		if(sFmacConfig.CoeffBSize==0)		//没有初始化成功
//		if(1)
		{
			API_FMAC_RestConfig();
//			if (HAL_FMAC_FilterConfig_DMA(&hfmac, &sFmacConfig) != HAL_OK)
////    if (HAL_FMAC_FilterConfig(&hfmac, &sFmacConfig) != HAL_OK)
//			{
//        /* Configuration Error */
//        Error_Handler();
//			}			
			
			
		}		
		else
		{
			API_FMAC_ChangeCoffeBsize();
			
//					if (HAL_FMAC_FilterPreload_DMA(&hfmac, &aInputValues1[FmacMem.outSize-COEFF_VECTOR_B_SIZE+1], COEFF_VECTOR_B_SIZE-1, NULL, 0) != HAL_OK)
//    {
//            /* Configuration Error */
//            Error_Handler();
//    }		
			
		}	

//			API_GPIO_WritePin(DebugA_pin,1);		//FMAC START
//	API_GPIO_WritePin(DebugB_pin,1);
//step1  COEFF_VECTOR_B_SIZE to wdata 写入滤波系数


			if (HAL_FMAC_FilterConfig_DMA(&hfmac, &sFmacConfig) != HAL_OK)
			{
        /* Configuration Error */
        Error_Handler();
			}	


#if FmacInt

#else			
			
    while(FilterConfigCallbackCount == 0)
		{}
		
    
  

//step 2 inputvalue	wdata	预写入结尾数据
        /* Preload the filter state at end of previous frame */
        if (HAL_FMAC_FilterPreload_DMA(&hfmac, &aInputValues1[ARRAY_SIZE-COEFF_VECTOR_B_SIZE+1], COEFF_VECTOR_B_SIZE-1, NULL, 0) != HAL_OK)
        {
            /* Configuration Error */
            Error_Handler();
        }		
		

				
				

    /*  Before starting a new process, you need to check the current state of the peripheral;
        if it's busy you need to wait for the end of current transfer before starting the calculation.
        For simplicity reasons, this example is just waiting till the end of the
        process, but the application may perform other tasks while the transfer is ongoing. */
    while (HAL_FMAC_GetState(&hfmac) != HAL_FMAC_STATE_READY)
    {
    }
#endif				
				
#if FmacInt

#else			
        /* Start calculation of FIR filter in DMA mode */
		
//step3  设置OUTPUT SIZE RDATA  ->POUTPUT 		
        ExpectedCalculatedFilteredDataSize = ARRAY_SIZE;
        if (HAL_FMAC_FilterStart(&hfmac, FmacMem.Y, &ExpectedCalculatedFilteredDataSize) != HAL_OK)
        {
            /* Processing Error */
            Error_Handler();
        }
        /*## Append data to start the DMA process after the preloaded data handling ##*/
//Step4  真正开始输入数据
				CurrentInputArraySize = ARRAY_SIZE;
        if (HAL_FMAC_AppendFilterData(&hfmac,
                                      aInputValues1,
                                      &CurrentInputArraySize) != HAL_OK)
        {
            ErrorCount++;
        }				
				
				
#endif
				
#if FmacInt

#else			


//		OutputDataReadyCallbackCount=1;
    while((HalfGetDataCallbackCount < GET_DATA_CALLBACK_COUNT) ||
          (GetDataCallbackCount < GET_DATA_CALLBACK_COUNT) ||
          (OutputDataReadyCallbackCount < DATA_RDY_CALLBACK_COUNT))
    {
        if(ErrorCount != 0)
        {
            /* Processing Error */
            Error_Handler();
        }
    }

	if (HAL_FMAC_FilterStop(&hfmac) != HAL_OK)
    {
        /* Processing Error */
        Error_Handler();
    }
#endif

//    /*## Check the final error status ############################################*/
//    if(ErrorCount != 0)
//    {
//        /* Processing Error */
//        Error_Handler();
//    }

//    /*## Compare FMAC results to the reference values ############################*/
//    for (Index = 0; Index < OUTPUT_ARRAY_1_SIZE; Index++)
//    {
//        if (aCalculatedFilteredData1[Index]  != aRefFilteredData[Index])
//        {
//            /* Processing Error */
//            Error_Handler();
//        }
//    }
//    for (Index = 0; Index < OUTPUT_ARRAY_2_SIZE; Index++)
//    {
//        if (aCalculatedFilteredData2[Index]  != aRefFilteredData[OUTPUT_ARRAY_1_SIZE + Index])
//        {
//            /* Processing Error */
//            Error_Handler();
//        }
//    }

//    /* There is no error in the output values: Turn LED2 on */
//    HAL_BSP_LED_On();

//    /* Show example state */
//    printf("Process ended successfully !\n");


    /* Infinite loop */
		
		

		
		

}

/* Private functions ---------------------------------------------------------*/

//void HAL_MspInit(void)
//{
//  /* USER CODE BEGIN MspInit 0 */

//  /* USER CODE END MspInit 0 */

//  __HAL_RCC_SYSCFG_CLK_ENABLE();
//  __HAL_RCC_PWR_CLK_ENABLE();

//  /* USER CODE BEGIN MspInit 1 */

//  /* USER CODE END MspInit 1 */
//}

/**
* @brief FMAC MSP Initialization
* This function configures the hardware resources used in this example
* @param hfmac: FMAC handle pointer
* @retval None
*/

void HAL_FMAC_MspInit(FMAC_HandleTypeDef* hfmac)
{
    if(hfmac->Instance==FMAC)
    {
    /* USER CODE BEGIN FMAC_MspInit 0 */
    
    /* USER CODE END FMAC_MspInit 0 */

        /* Peripheral clock enable */
        __HAL_RCC_FMAC_CLK_ENABLE();

        /* FMAC DMA Init */
        /* FMAC_PRELOAD Init */
        hdma_fmac_preload.Instance = DMA1_Channel6;
        hdma_fmac_preload.Init.Request = DMA_REQUEST_MEM2MEM;
        hdma_fmac_preload.Init.Direction = DMA_MEMORY_TO_MEMORY;
        hdma_fmac_preload.Init.PeriphInc = DMA_PINC_ENABLE;
        hdma_fmac_preload.Init.MemInc = DMA_MINC_DISABLE;
        hdma_fmac_preload.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_fmac_preload.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_fmac_preload.Init.Mode = DMA_NORMAL;
        hdma_fmac_preload.Init.Priority = DMA_PRIORITY_HIGH;
        if (HAL_DMA_Init(&hdma_fmac_preload) != HAL_OK)
        {
        Error_Handler();
        }

        __HAL_LINKDMA(hfmac,hdmaPreload,hdma_fmac_preload);

        /* FMAC_WRITE Init */
         hdma_fmac_write.Instance = DMA1_Channel5;
         hdma_fmac_write.Init.Request = DMA_REQUEST_FMAC_WRITE;
         hdma_fmac_write.Init.Direction = DMA_MEMORY_TO_PERIPH;
         hdma_fmac_write.Init.PeriphInc = DMA_PINC_DISABLE;
         hdma_fmac_write.Init.MemInc = DMA_MINC_ENABLE;
         hdma_fmac_write.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
         hdma_fmac_write.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
         hdma_fmac_write.Init.Mode = DMA_NORMAL;
         hdma_fmac_write.Init.Priority = DMA_PRIORITY_HIGH;
         if (HAL_DMA_Init(&hdma_fmac_write) != HAL_OK)
         {
             Error_Handler();
         }

         __HAL_LINKDMA(hfmac,hdmaIn,hdma_fmac_write);
    

        //增加FMAC读取DMA搬运
        /* FMAC_READ Init */
        hdma_fmac_read.Instance = DMA2_Channel6;
        hdma_fmac_read.Init.Request = DMA_REQUEST_FMAC_READ;
        hdma_fmac_read.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_fmac_read.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_fmac_read.Init.MemInc = DMA_MINC_ENABLE;
        hdma_fmac_read.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_fmac_read.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_fmac_read.Init.Mode = DMA_NORMAL;
        hdma_fmac_read.Init.Priority = DMA_PRIORITY_HIGH;
        if (HAL_DMA_Init(&hdma_fmac_read) != HAL_OK)
        {
            Error_Handler();
        }
    
        __HAL_LINKDMA(hfmac,hdmaOut,hdma_fmac_read);


        /* FMAC interrupt Init */
        HAL_NVIC_SetPriority(FMAC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FMAC_IRQn);
    /* USER CODE BEGIN FMAC_MspInit 1 */
    
    /* USER CODE END FMAC_MspInit 1 */
    }
}

/**
  * @brief FMAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_FMAC_Init(void)
{
	FilterConfigCallbackCount    = 0;
	FilterPreloadCallbackCount   = 0;
	HalfGetDataCallbackCount     = 0;
	GetDataCallbackCount         = 0;
	OutputDataReadyCallbackCount = 0;
	ErrorCount                   = 0;
	
    hfmac.Instance = FMAC;
    if (HAL_FMAC_Init(&hfmac) != HAL_OK)
    {
        Error_Handler();
    }
		__HAL_DMA_DISABLE(&hdma_fmac_read);
		
}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();


// #define	DMA_Channel_FMAC_PREO		DMA1_Channel5			 //fmac
// #define	DMA_Channel_FMAC_WRITE		DMA1_Channel6			 //fmac
// #define	DMA_Channel_FMAC_READ		DMA2_Channel6	    //注意rx32g4xx_it.c中断源的对应

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    /* DMA1_Channel5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
        /* DMA2_Channel7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Channel6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Channel6_IRQn);


}

/* USER CODE BEGIN 4 */
/**
  * @brief FMAC filter configuration callback
  * @par hfmac: FMAC HAL handle
  * @retval None
  */
void HAL_FMAC_FilterConfigCallback(FMAC_HandleTypeDef *hfmac)
{
    FilterConfigCallbackCount++;;
#if FmacInt		
		if (HAL_FMAC_FilterPreload_DMA(hfmac, &aInputValues1[FmacMem.outSize-COEFF_VECTOR_B_SIZE+1], COEFF_VECTOR_B_SIZE-1, NULL, 0) != HAL_OK)
    {
            /* Configuration Error */
            Error_Handler();
    }		
#endif	
}

/**
  * @brief FMAC filter preload callback
  * @par hfmac: FMAC HAL handle
  * @retval None
  */
void HAL_FMAC_FilterPreloadCallback(FMAC_HandleTypeDef *hfmac)
{
    FilterPreloadCallbackCount++;;
	
#if FmacInt		
//step3  设置OUTPUT SIZE RDATA  ->POUTPUT 		
        ExpectedCalculatedFilteredDataSize = FmacMem.outSize;
        if (HAL_FMAC_FilterStart(hfmac, FmacMem.Y, &ExpectedCalculatedFilteredDataSize) != HAL_OK)
        {
            /* Processing Error */
            Error_Handler();
        }
        /*## Append data to start the DMA process after the preloaded data handling ##*/
//Step4  真正开始输入数据
				CurrentInputArraySize = FmacMem.outSize;
        if (HAL_FMAC_AppendFilterData(hfmac,
                                      aInputValues1,
                                      &CurrentInputArraySize) != HAL_OK)
        {
            ErrorCount++;
        }			
#endif
	
}

/**
  * @brief FMAC half get data callback
  * @par hfmac: FMAC HAL handle
  * @retval None
  */
void HAL_FMAC_HalfGetDataCallback(FMAC_HandleTypeDef *hfmac)
{
    HalfGetDataCallbackCount++;
}

/**
  * @brief FMAC get data callback
  * @par hfmac: FMAC HAL handle
  * @retval None
  */
void HAL_FMAC_GetDataCallback(FMAC_HandleTypeDef *hfmac)
{
    GetDataCallbackCount++;
	

//    if (GetDataCallbackCount == 1)
//    {
//        /* The preloaded data (1) and the appended data (2) have been handled,
//           write the following input values into FMAC (3 over 4) */
//        CurrentInputArraySize = INPUT_ARRAY_3_SIZE;
//        if (HAL_FMAC_AppendFilterData(hfmac,
//                                      aInputValues3,
//                                      &CurrentInputArraySize) != HAL_OK)
//        {
//            ErrorCount++;
//        }
//    }
//    else if (GetDataCallbackCount == 2)
//    {
//        /* Write the following input values into FMAC (4 over 4) */
//        CurrentInputArraySize = INPUT_ARRAY_4_SIZE;
//        if (HAL_FMAC_AppendFilterData(hfmac,
//                                      aInputValues4,
//                                      &CurrentInputArraySize) != HAL_OK)
//        {
//            ErrorCount++;
//        }
//    }
//    else
//    {
//        /* No more data to write */
//    }
}

/**
  * @brief FMAC output data ready callback
  * @par hfmac: FMAC HAL handle
  * @retval None
  */
void HAL_FMAC_OutputDataReadyCallback(FMAC_HandleTypeDef *hfmac)
{
    OutputDataReadyCallbackCount++;

#if FmacInt	
		if (HAL_FMAC_FilterStop(hfmac) != HAL_OK)
    {
        /* Processing Error */
        Error_Handler();
    }
		
    switch(FmacMem.type)
    {
			
			case FmacPan:
			
      API_FMAC_AppPowerOverCallBack(&FmacMem);  //检锅
			break;
    
			case	FmacTxa:
   
		  API_FMAC_AppAdcOverCallBack(&FmacMem);    //谐振电流处理
			break;
		}
		FmacMem.type=FmacStop;

		
//		API_GPIO_WritePin(DebugA_pin,0);		//FMAC END
#endif			
	
	
//    if (OutputDataReadyCallbackCount == 1)
//    {
//        ExpectedCalculatedFilteredDataSize = OUTPUT_ARRAY_2_SIZE;
//        if (HAL_FMAC_ConfigFilterOutputBuffer(hfmac,
//                                              aCalculatedFilteredData2,
//                                              &ExpectedCalculatedFilteredDataSize) != HAL_OK)
//        {
//            ErrorCount++;
//        }
//    }
//    else
//    {
//        /* No more data will be read, disable the FMAC read interrupt */
//        __HAL_FMAC_DISABLE_IT(hfmac, FMAC_IT_RIEN);
//    }
}

/**
  * @brief FMAC error callback
  * @par hfmac: FMAC HAL handle
  * @retval None
  */
void HAL_FMAC_ErrorCallback(FMAC_HandleTypeDef *hfmac)
{
    ErrorCount++;
}

void	API_DMA_FmacPreload_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_fmac_preload);
}	
void	API_DMA_FmacWrite_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_fmac_write);
}	
void	API_DMA_FmacRead_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_fmac_read);
}	



void	API_FMAC_IRQHandler(void)
{
	HAL_FMAC_IRQHandler(&hfmac);
}

int16_t*   API_FMAC_InputValueAdress(void)
{
	return	aInputValues1+COEFF_VECTOR_B_SIZE;
}	


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
//void Error_Handler(void)
//{
////    printf("ERROR!\r\n");
//    while(1)
//    {
//        /* LED2 is blinking */
//        HAL_BSP_LED_Toggle();
//        HAL_Delay(500);
//    }
//}
