/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2019 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

/***********************************************************************************************************/
/* Website: http://www.nuvoton.com                                                                         */
/*  E-Mail : MicroC-8bit@nuvoton.com                                                                       */
/*  Date   : Jan/21/2019                                                                                   */
/***********************************************************************************************************/

/************************************************************************************************************/
/*  File Function: MS51 DEMO project                                                                        */
/************************************************************************************************************/

#include "MS51.h"

//typedef unsigned char       	U8;
//typedef unsigned int         	U16;
//typedef unsigned long int 		U32;

//bit BIT_TMP;
#define SYS_CLOCK 								(24000000ul)

#define ENABLE_EXTRA_DEMO

//#define ENABLE_TIMER0_1MS_IRQ
//#define ENABLE_UART0_LOG
//#define ENABLE_GPIO_DEMO
//#define ENABLE_PWM_TRIG_LED
//#define ENABLE_SPI_TRIG_LED
#define ENABLE_GPIO_TRIG_LED
//#define ENABLE_TIMER1_GPIO_TRIG_LED

//#define USE_P00
//#define USE_P01
#define USE_P12

#if defined (USE_P00)
#define set_LEDGPIO(x)			(P00 = x)
#elif defined (USE_P01)
#define set_LEDGPIO(x)			(P01 = x)
#elif defined (USE_P12)
#define set_LEDGPIO(x)			(P12 = x)
#endif


#define LED_NUM 								(6)	//(18)

//#if defined (ENABLE_PWM_TRIG_LED)
//#define LED_DATA_LEN 							(LED_NUM * 24)
//#else
#define LED_DATA_LEN 							(LED_NUM * 3)
//#endif

#define MS_LED_LATCH							(450)	//(450)
#define MS_1MS									(1500)


#define SPI_12M									(0)
#define SPI_6M									(1)
#define SPI_3M									(2)
#define SPI_1_5M									(3)

//#define TIMER_LOG_MS							(1000ul)
//#define GPIO_TOGGLE_MS							(500ul)
//#define UART_LOG_MS							(50ul)
#define DEMO_MS								(500ul)//	(5000ul)

#define HEARTBEAT_STEPS							(16)

//#define PACK_RGB(r,g,b)							((g<<16)+(r<<8)+b)
//#define UNPACK_R(data)							(uint8_t)((data>>8)&0xFF)
//#define UNPACK_G(data)							(uint8_t)((data>>16)&0xFF)
//#define UNPACK_B(data)							(uint8_t)((data)&0xFF)



/*
	CPU	SPI Freq	Timing	
	24	12			0.083333333	us
		6			0.166666667	
		3			0.333333333	
		1.5			0.666666667	
				
	16	8			0.125	
		4			0.25	
		2			0.5	
		1			1	

	WS2812C-2020
	T1H 1 HIGH 580ns~1�gs
	T1L 1 LOW 220ns~420ns
	==> 1111 1100 (0xFC)	==> if SPI = 6M
	
	T0H 0 HIGH 220ns~380ns
	T0L 0 LOW 580ns~1�gs
	==> 1100 0000 (0xC0)	==> if SPI = 6M	
 	_____   
	|     |___|   1111 1100  high level
 	___         
	|   |_____|   1100 0000  low level


	
	RES : more than 280�gs
*/
#if defined (ENABLE_SPI_TRIG_LED)
#define WS_SPI_FREQ 								(800000ul)
#define WS_SPI_HIGH 								(0xFC)
#define WS_SPI_LOW	 							(0xC0)
#endif

//#define WS_RES 	 								(0x00) 

#if defined (ENABLE_PWM_TRIG_LED)
#define WS_PWM_FREQ 	 						(800000ul) 
#define WS_PWM_HIGH 							(75) 
#define WS_PWM_LOW 							(28) // 912 - 375 , 925 ~ 325
#endif

#define WS_RES_POS_FRONT						(0) 
#define WS_RES_POS_BACK 						(1) 

#if defined (ENABLE_TIMER1_GPIO_TRIG_LED)
uint16_t count;
uint16_t endcount;
uint8_t flag;

#define WS_FLAG_HIGH	 						(1)
#define WS_FLAG_LOW	 						(0)
#define WS_TIM_FREQ 							(800000ul)
#define WS_TIM_HIGH 							(2)
#define WS_TIM_LOW	 							(1)

uint8_t u8TH1_Tmp = 0;
uint8_t u8TL1_Tmp = 0;
#endif

#if defined (ENABLE_TIMER0_1MS_IRQ)
uint8_t u8TH0_Tmp = 0;
uint8_t u8TL0_Tmp = 0;
uint16_t u16Cnt = 0;
uint16_t u16Period = 0;
#endif

volatile uint8_t xdata DataBuffer[LED_DATA_LEN] = {0};

uint8_t DemoState = 0;

enum
{
	state_Default = 0,

	state_Rainbow,
	state_RainbowCycle ,	
	state_TheaterChase,	
	state_TheaterChaseRainbow ,
	state_RGBSequence,
	state_AlternateColors,	
	state_FullEmpty,
	state_PatternMove,
#if defined (ENABLE_EXTRA_DEMO)	
//	state_ColorWheel,
	state_AllColors,
#endif	
//	state_HeartBeat,
	state_CircularRing,
//	state_FromAToB,

	state_Max,
};

#if defined (ENABLE_EXTRA_DEMO)
const uint8_t code colorsFull[766][3] = 
{ 
	{	255	,	0	,	0	}	,
	{	254	,	1	,	0	}	,
	{	253	,	2	,	0	}	,
	{	252	,	3	,	0	}	,
	{	251	,	4	,	0	}	,
	{	250	,	5	,	0	}	,
	{	249	,	6	,	0	}	,
	{	248	,	7	,	0	}	,
	{	247	,	8	,	0	}	,
	{	246	,	9	,	0	}	,
	{	245	,	10	,	0	}	,
	{	244	,	11	,	0	}	,
	{	243	,	12	,	0	}	,
	{	242	,	13	,	0	}	,
	{	241	,	14	,	0	}	,
	{	240	,	15	,	0	}	,
	{	239	,	16	,	0	}	,
	{	238	,	17	,	0	}	,
	{	237	,	18	,	0	}	,
	{	236	,	19	,	0	}	,
	{	235	,	20	,	0	}	,
	{	234	,	21	,	0	}	,
	{	233	,	22	,	0	}	,
	{	232	,	23	,	0	}	,
	{	231	,	24	,	0	}	,
	{	230	,	25	,	0	}	,
	{	229	,	26	,	0	}	,
	{	228	,	27	,	0	}	,
	{	227	,	28	,	0	}	,
	{	226	,	29	,	0	}	,
	{	225	,	30	,	0	}	,
	{	224	,	31	,	0	}	,
	{	223	,	32	,	0	}	,
	{	222	,	33	,	0	}	,
	{	221	,	34	,	0	}	,
	{	220	,	35	,	0	}	,
	{	219	,	36	,	0	}	,
	{	218	,	37	,	0	}	,
	{	217	,	38	,	0	}	,
	{	216	,	39	,	0	}	,
	{	215	,	40	,	0	}	,
	{	214	,	41	,	0	}	,
	{	213	,	42	,	0	}	,
	{	212	,	43	,	0	}	,
	{	211	,	44	,	0	}	,
	{	210	,	45	,	0	}	,
	{	209	,	46	,	0	}	,
	{	208	,	47	,	0	}	,
	{	207	,	48	,	0	}	,
	{	206	,	49	,	0	}	,
	{	205	,	50	,	0	}	,
	{	204	,	51	,	0	}	,
	{	203	,	52	,	0	}	,
	{	202	,	53	,	0	}	,
	{	201	,	54	,	0	}	,
	{	200	,	55	,	0	}	,
	{	199	,	56	,	0	}	,
	{	198	,	57	,	0	}	,
	{	197	,	58	,	0	}	,
	{	196	,	59	,	0	}	,
	{	195	,	60	,	0	}	,
	{	194	,	61	,	0	}	,
	{	193	,	62	,	0	}	,
	{	192	,	63	,	0	}	,
	{	191	,	64	,	0	}	,
	{	190	,	65	,	0	}	,
	{	189	,	66	,	0	}	,
	{	188	,	67	,	0	}	,
	{	187	,	68	,	0	}	,
	{	186	,	69	,	0	}	,
	{	185	,	70	,	0	}	,
	{	184	,	71	,	0	}	,
	{	183	,	72	,	0	}	,
	{	182	,	73	,	0	}	,
	{	181	,	74	,	0	}	,
	{	180	,	75	,	0	}	,
	{	179	,	76	,	0	}	,
	{	178	,	77	,	0	}	,
	{	177	,	78	,	0	}	,
	{	176	,	79	,	0	}	,
	{	175	,	80	,	0	}	,
	{	174	,	81	,	0	}	,
	{	173	,	82	,	0	}	,
	{	172	,	83	,	0	}	,
	{	171	,	84	,	0	}	,
	{	170	,	85	,	0	}	,
	{	169	,	86	,	0	}	,
	{	168	,	87	,	0	}	,
	{	167	,	88	,	0	}	,
	{	166	,	89	,	0	}	,
	{	165	,	90	,	0	}	,
	{	164	,	91	,	0	}	,
	{	163	,	92	,	0	}	,
	{	162	,	93	,	0	}	,
	{	161	,	94	,	0	}	,
	{	160	,	95	,	0	}	,
	{	159	,	96	,	0	}	,
	{	158	,	97	,	0	}	,
	{	157	,	98	,	0	}	,
	{	156	,	99	,	0	}	,
	{	155	,	100	,	0	}	,
	{	154	,	101	,	0	}	,
	{	153	,	102	,	0	}	,
	{	152	,	103	,	0	}	,
	{	151	,	104	,	0	}	,
	{	150	,	105	,	0	}	,
	{	149	,	106	,	0	}	,
	{	148	,	107	,	0	}	,
	{	147	,	108	,	0	}	,
	{	146	,	109	,	0	}	,
	{	145	,	110	,	0	}	,
	{	144	,	111	,	0	}	,
	{	143	,	112	,	0	}	,
	{	142	,	113	,	0	}	,
	{	141	,	114	,	0	}	,
	{	140	,	115	,	0	}	,
	{	139	,	116	,	0	}	,
	{	138	,	117	,	0	}	,
	{	137	,	118	,	0	}	,
	{	136	,	119	,	0	}	,
	{	135	,	120	,	0	}	,
	{	134	,	121	,	0	}	,
	{	133	,	122	,	0	}	,
	{	132	,	123	,	0	}	,
	{	131	,	124	,	0	}	,
	{	130	,	125	,	0	}	,
	{	129	,	126	,	0	}	,
	{	128	,	127	,	0	}	,
	{	127	,	128	,	0	}	,
	{	126	,	129	,	0	}	,
	{	125	,	130	,	0	}	,
	{	124	,	131	,	0	}	,
	{	123	,	132	,	0	}	,
	{	122	,	133	,	0	}	,
	{	121	,	134	,	0	}	,
	{	120	,	135	,	0	}	,
	{	119	,	136	,	0	}	,
	{	118	,	137	,	0	}	,
	{	117	,	138	,	0	}	,
	{	116	,	139	,	0	}	,
	{	115	,	140	,	0	}	,
	{	114	,	141	,	0	}	,
	{	113	,	142	,	0	}	,
	{	112	,	143	,	0	}	,
	{	111	,	144	,	0	}	,
	{	110	,	145	,	0	}	,
	{	109	,	146	,	0	}	,
	{	108	,	147	,	0	}	,
	{	107	,	148	,	0	}	,
	{	106	,	149	,	0	}	,
	{	105	,	150	,	0	}	,
	{	104	,	151	,	0	}	,
	{	103	,	152	,	0	}	,
	{	102	,	153	,	0	}	,
	{	101	,	154	,	0	}	,
	{	100	,	155	,	0	}	,
	{	99	,	156	,	0	}	,
	{	98	,	157	,	0	}	,
	{	97	,	158	,	0	}	,
	{	96	,	159	,	0	}	,
	{	95	,	160	,	0	}	,
	{	94	,	161	,	0	}	,
	{	93	,	162	,	0	}	,
	{	92	,	163	,	0	}	,
	{	91	,	164	,	0	}	,
	{	90	,	165	,	0	}	,
	{	89	,	166	,	0	}	,
	{	88	,	167	,	0	}	,
	{	87	,	168	,	0	}	,
	{	86	,	169	,	0	}	,
	{	85	,	170	,	0	}	,
	{	84	,	171	,	0	}	,
	{	83	,	172	,	0	}	,
	{	82	,	173	,	0	}	,
	{	81	,	174	,	0	}	,
	{	80	,	175	,	0	}	,
	{	79	,	176	,	0	}	,
	{	78	,	177	,	0	}	,
	{	77	,	178	,	0	}	,
	{	76	,	179	,	0	}	,
	{	75	,	180	,	0	}	,
	{	74	,	181	,	0	}	,
	{	73	,	182	,	0	}	,
	{	72	,	183	,	0	}	,
	{	71	,	184	,	0	}	,
	{	70	,	185	,	0	}	,
	{	69	,	186	,	0	}	,
	{	68	,	187	,	0	}	,
	{	67	,	188	,	0	}	,
	{	66	,	189	,	0	}	,
	{	65	,	190	,	0	}	,
	{	64	,	191	,	0	}	,
	{	63	,	192	,	0	}	,
	{	62	,	193	,	0	}	,
	{	61	,	194	,	0	}	,
	{	60	,	195	,	0	}	,
	{	59	,	196	,	0	}	,
	{	58	,	197	,	0	}	,
	{	57	,	198	,	0	}	,
	{	56	,	199	,	0	}	,
	{	55	,	200	,	0	}	,
	{	54	,	201	,	0	}	,
	{	53	,	202	,	0	}	,
	{	52	,	203	,	0	}	,
	{	51	,	204	,	0	}	,
	{	50	,	205	,	0	}	,
	{	49	,	206	,	0	}	,
	{	48	,	207	,	0	}	,
	{	47	,	208	,	0	}	,
	{	46	,	209	,	0	}	,
	{	45	,	210	,	0	}	,
	{	44	,	211	,	0	}	,
	{	43	,	212	,	0	}	,
	{	42	,	213	,	0	}	,
	{	41	,	214	,	0	}	,
	{	40	,	215	,	0	}	,
	{	39	,	216	,	0	}	,
	{	38	,	217	,	0	}	,
	{	37	,	218	,	0	}	,
	{	36	,	219	,	0	}	,
	{	35	,	220	,	0	}	,
	{	34	,	221	,	0	}	,
	{	33	,	222	,	0	}	,
	{	32	,	223	,	0	}	,
	{	31	,	224	,	0	}	,
	{	30	,	225	,	0	}	,
	{	29	,	226	,	0	}	,
	{	28	,	227	,	0	}	,
	{	27	,	228	,	0	}	,
	{	26	,	229	,	0	}	,
	{	25	,	230	,	0	}	,
	{	24	,	231	,	0	}	,
	{	23	,	232	,	0	}	,
	{	22	,	233	,	0	}	,
	{	21	,	234	,	0	}	,
	{	20	,	235	,	0	}	,
	{	19	,	236	,	0	}	,
	{	18	,	237	,	0	}	,
	{	17	,	238	,	0	}	,
	{	16	,	239	,	0	}	,
	{	15	,	240	,	0	}	,
	{	14	,	241	,	0	}	,
	{	13	,	242	,	0	}	,
	{	12	,	243	,	0	}	,
	{	11	,	244	,	0	}	,
	{	10	,	245	,	0	}	,
	{	9	,	246	,	0	}	,
	{	8	,	247	,	0	}	,
	{	7	,	248	,	0	}	,
	{	6	,	249	,	0	}	,
	{	5	,	250	,	0	}	,
	{	4	,	251	,	0	}	,
	{	3	,	252	,	0	}	,
	{	2	,	253	,	0	}	,
	{	1	,	254	,	0	}	,
	{	0	,	255	,	0	}	,
	{	0	,	254	,	1	}	,
	{	0	,	253	,	2	}	,
	{	0	,	252	,	3	}	,
	{	0	,	251	,	4	}	,
	{	0	,	250	,	5	}	,
	{	0	,	249	,	6	}	,
	{	0	,	248	,	7	}	,
	{	0	,	247	,	8	}	,
	{	0	,	246	,	9	}	,
	{	0	,	245	,	10	}	,
	{	0	,	244	,	11	}	,
	{	0	,	243	,	12	}	,
	{	0	,	242	,	13	}	,
	{	0	,	241	,	14	}	,
	{	0	,	240	,	15	}	,
	{	0	,	239	,	16	}	,
	{	0	,	238	,	17	}	,
	{	0	,	237	,	18	}	,
	{	0	,	236	,	19	}	,
	{	0	,	235	,	20	}	,
	{	0	,	234	,	21	}	,
	{	0	,	233	,	22	}	,
	{	0	,	232	,	23	}	,
	{	0	,	231	,	24	}	,
	{	0	,	230	,	25	}	,
	{	0	,	229	,	26	}	,
	{	0	,	228	,	27	}	,
	{	0	,	227	,	28	}	,
	{	0	,	226	,	29	}	,
	{	0	,	225	,	30	}	,
	{	0	,	224	,	31	}	,
	{	0	,	223	,	32	}	,
	{	0	,	222	,	33	}	,
	{	0	,	221	,	34	}	,
	{	0	,	220	,	35	}	,
	{	0	,	219	,	36	}	,
	{	0	,	218	,	37	}	,
	{	0	,	217	,	38	}	,
	{	0	,	216	,	39	}	,
	{	0	,	215	,	40	}	,
	{	0	,	214	,	41	}	,
	{	0	,	213	,	42	}	,
	{	0	,	212	,	43	}	,
	{	0	,	211	,	44	}	,
	{	0	,	210	,	45	}	,
	{	0	,	209	,	46	}	,
	{	0	,	208	,	47	}	,
	{	0	,	207	,	48	}	,
	{	0	,	206	,	49	}	,
	{	0	,	205	,	50	}	,
	{	0	,	204	,	51	}	,
	{	0	,	203	,	52	}	,
	{	0	,	202	,	53	}	,
	{	0	,	201	,	54	}	,
	{	0	,	200	,	55	}	,
	{	0	,	199	,	56	}	,
	{	0	,	198	,	57	}	,
	{	0	,	197	,	58	}	,
	{	0	,	196	,	59	}	,
	{	0	,	195	,	60	}	,
	{	0	,	194	,	61	}	,
	{	0	,	193	,	62	}	,
	{	0	,	192	,	63	}	,
	{	0	,	191	,	64	}	,
	{	0	,	190	,	65	}	,
	{	0	,	189	,	66	}	,
	{	0	,	188	,	67	}	,
	{	0	,	187	,	68	}	,
	{	0	,	186	,	69	}	,
	{	0	,	185	,	70	}	,
	{	0	,	184	,	71	}	,
	{	0	,	183	,	72	}	,
	{	0	,	182	,	73	}	,
	{	0	,	181	,	74	}	,
	{	0	,	180	,	75	}	,
	{	0	,	179	,	76	}	,
	{	0	,	178	,	77	}	,
	{	0	,	177	,	78	}	,
	{	0	,	176	,	79	}	,
	{	0	,	175	,	80	}	,
	{	0	,	174	,	81	}	,
	{	0	,	173	,	82	}	,
	{	0	,	172	,	83	}	,
	{	0	,	171	,	84	}	,
	{	0	,	170	,	85	}	,
	{	0	,	169	,	86	}	,
	{	0	,	168	,	87	}	,
	{	0	,	167	,	88	}	,
	{	0	,	166	,	89	}	,
	{	0	,	165	,	90	}	,
	{	0	,	164	,	91	}	,
	{	0	,	163	,	92	}	,
	{	0	,	162	,	93	}	,
	{	0	,	161	,	94	}	,
	{	0	,	160	,	95	}	,
	{	0	,	159	,	96	}	,
	{	0	,	158	,	97	}	,
	{	0	,	157	,	98	}	,
	{	0	,	156	,	99	}	,
	{	0	,	155	,	100	}	,
	{	0	,	154	,	101	}	,
	{	0	,	153	,	102	}	,
	{	0	,	152	,	103	}	,
	{	0	,	151	,	104	}	,
	{	0	,	150	,	105	}	,
	{	0	,	149	,	106	}	,
	{	0	,	148	,	107	}	,
	{	0	,	147	,	108	}	,
	{	0	,	146	,	109	}	,
	{	0	,	145	,	110	}	,
	{	0	,	144	,	111	}	,
	{	0	,	143	,	112	}	,
	{	0	,	142	,	113	}	,
	{	0	,	141	,	114	}	,
	{	0	,	140	,	115	}	,
	{	0	,	139	,	116	}	,
	{	0	,	138	,	117	}	,
	{	0	,	137	,	118	}	,
	{	0	,	136	,	119	}	,
	{	0	,	135	,	120	}	,
	{	0	,	134	,	121	}	,
	{	0	,	133	,	122	}	,
	{	0	,	132	,	123	}	,
	{	0	,	131	,	124	}	,
	{	0	,	130	,	125	}	,
	{	0	,	129	,	126	}	,
	{	0	,	128	,	127	}	,
	{	0	,	127	,	128	}	,
	{	0	,	126	,	129	}	,
	{	0	,	125	,	130	}	,
	{	0	,	124	,	131	}	,
	{	0	,	123	,	132	}	,
	{	0	,	122	,	133	}	,
	{	0	,	121	,	134	}	,
	{	0	,	120	,	135	}	,
	{	0	,	119	,	136	}	,
	{	0	,	118	,	137	}	,
	{	0	,	117	,	138	}	,
	{	0	,	116	,	139	}	,
	{	0	,	115	,	140	}	,
	{	0	,	114	,	141	}	,
	{	0	,	113	,	142	}	,
	{	0	,	112	,	143	}	,
	{	0	,	111	,	144	}	,
	{	0	,	110	,	145	}	,
	{	0	,	109	,	146	}	,
	{	0	,	108	,	147	}	,
	{	0	,	107	,	148	}	,
	{	0	,	106	,	149	}	,
	{	0	,	105	,	150	}	,
	{	0	,	104	,	151	}	,
	{	0	,	103	,	152	}	,
	{	0	,	102	,	153	}	,
	{	0	,	101	,	154	}	,
	{	0	,	100	,	155	}	,
	{	0	,	99	,	156	}	,
	{	0	,	98	,	157	}	,
	{	0	,	97	,	158	}	,
	{	0	,	96	,	159	}	,
	{	0	,	95	,	160	}	,
	{	0	,	94	,	161	}	,
	{	0	,	93	,	162	}	,
	{	0	,	92	,	163	}	,
	{	0	,	91	,	164	}	,
	{	0	,	90	,	165	}	,
	{	0	,	89	,	166	}	,
	{	0	,	88	,	167	}	,
	{	0	,	87	,	168	}	,
	{	0	,	86	,	169	}	,
	{	0	,	85	,	170	}	,
	{	0	,	84	,	171	}	,
	{	0	,	83	,	172	}	,
	{	0	,	82	,	173	}	,
	{	0	,	81	,	174	}	,
	{	0	,	80	,	175	}	,
	{	0	,	79	,	176	}	,
	{	0	,	78	,	177	}	,
	{	0	,	77	,	178	}	,
	{	0	,	76	,	179	}	,
	{	0	,	75	,	180	}	,
	{	0	,	74	,	181	}	,
	{	0	,	73	,	182	}	,
	{	0	,	72	,	183	}	,
	{	0	,	71	,	184	}	,
	{	0	,	70	,	185	}	,
	{	0	,	69	,	186	}	,
	{	0	,	68	,	187	}	,
	{	0	,	67	,	188	}	,
	{	0	,	66	,	189	}	,
	{	0	,	65	,	190	}	,
	{	0	,	64	,	191	}	,
	{	0	,	63	,	192	}	,
	{	0	,	62	,	193	}	,
	{	0	,	61	,	194	}	,
	{	0	,	60	,	195	}	,
	{	0	,	59	,	196	}	,
	{	0	,	58	,	197	}	,
	{	0	,	57	,	198	}	,
	{	0	,	56	,	199	}	,
	{	0	,	55	,	200	}	,
	{	0	,	54	,	201	}	,
	{	0	,	53	,	202	}	,
	{	0	,	52	,	203	}	,
	{	0	,	51	,	204	}	,
	{	0	,	50	,	205	}	,
	{	0	,	49	,	206	}	,
	{	0	,	48	,	207	}	,
	{	0	,	47	,	208	}	,
	{	0	,	46	,	209	}	,
	{	0	,	45	,	210	}	,
	{	0	,	44	,	211	}	,
	{	0	,	43	,	212	}	,
	{	0	,	42	,	213	}	,
	{	0	,	41	,	214	}	,
	{	0	,	40	,	215	}	,
	{	0	,	39	,	216	}	,
	{	0	,	38	,	217	}	,
	{	0	,	37	,	218	}	,
	{	0	,	36	,	219	}	,
	{	0	,	35	,	220	}	,
	{	0	,	34	,	221	}	,
	{	0	,	33	,	222	}	,
	{	0	,	32	,	223	}	,
	{	0	,	31	,	224	}	,
	{	0	,	30	,	225	}	,
	{	0	,	29	,	226	}	,
	{	0	,	28	,	227	}	,
	{	0	,	27	,	228	}	,
	{	0	,	26	,	229	}	,
	{	0	,	25	,	230	}	,
	{	0	,	24	,	231	}	,
	{	0	,	23	,	232	}	,
	{	0	,	22	,	233	}	,
	{	0	,	21	,	234	}	,
	{	0	,	20	,	235	}	,
	{	0	,	19	,	236	}	,
	{	0	,	18	,	237	}	,
	{	0	,	17	,	238	}	,
	{	0	,	16	,	239	}	,
	{	0	,	15	,	240	}	,
	{	0	,	14	,	241	}	,
	{	0	,	13	,	242	}	,
	{	0	,	12	,	243	}	,
	{	0	,	11	,	244	}	,
	{	0	,	10	,	245	}	,
	{	0	,	9	,	246	}	,
	{	0	,	8	,	247	}	,
	{	0	,	7	,	248	}	,
	{	0	,	6	,	249	}	,
	{	0	,	5	,	250	}	,
	{	0	,	4	,	251	}	,
	{	0	,	3	,	252	}	,
	{	0	,	2	,	253	}	,
	{	0	,	1	,	254	}	,
	{	0	,	0	,	255	}	,
	{	1	,	0	,	254	}	,
	{	2	,	0	,	253	}	,
	{	3	,	0	,	252	}	,
	{	4	,	0	,	251	}	,
	{	5	,	0	,	250	}	,
	{	6	,	0	,	249	}	,
	{	7	,	0	,	248	}	,
	{	8	,	0	,	247	}	,
	{	9	,	0	,	246	}	,
	{	10	,	0	,	245	}	,
	{	11	,	0	,	244	}	,
	{	12	,	0	,	243	}	,
	{	13	,	0	,	242	}	,
	{	14	,	0	,	241	}	,
	{	15	,	0	,	240	}	,
	{	16	,	0	,	239	}	,
	{	17	,	0	,	238	}	,
	{	18	,	0	,	237	}	,
	{	19	,	0	,	236	}	,
	{	20	,	0	,	235	}	,
	{	21	,	0	,	234	}	,
	{	22	,	0	,	233	}	,
	{	23	,	0	,	232	}	,
	{	24	,	0	,	231	}	,
	{	25	,	0	,	230	}	,
	{	26	,	0	,	229	}	,
	{	27	,	0	,	228	}	,
	{	28	,	0	,	227	}	,
	{	29	,	0	,	226	}	,
	{	30	,	0	,	225	}	,
	{	31	,	0	,	224	}	,
	{	32	,	0	,	223	}	,
	{	33	,	0	,	222	}	,
	{	34	,	0	,	221	}	,
	{	35	,	0	,	220	}	,
	{	36	,	0	,	219	}	,
	{	37	,	0	,	218	}	,
	{	38	,	0	,	217	}	,
	{	39	,	0	,	216	}	,
	{	40	,	0	,	215	}	,
	{	41	,	0	,	214	}	,
	{	42	,	0	,	213	}	,
	{	43	,	0	,	212	}	,
	{	44	,	0	,	211	}	,
	{	45	,	0	,	210	}	,
	{	46	,	0	,	209	}	,
	{	47	,	0	,	208	}	,
	{	48	,	0	,	207	}	,
	{	49	,	0	,	206	}	,
	{	50	,	0	,	205	}	,
	{	51	,	0	,	204	}	,
	{	52	,	0	,	203	}	,
	{	53	,	0	,	202	}	,
	{	54	,	0	,	201	}	,
	{	55	,	0	,	200	}	,
	{	56	,	0	,	199	}	,
	{	57	,	0	,	198	}	,
	{	58	,	0	,	197	}	,
	{	59	,	0	,	196	}	,
	{	60	,	0	,	195	}	,
	{	61	,	0	,	194	}	,
	{	62	,	0	,	193	}	,
	{	63	,	0	,	192	}	,
	{	64	,	0	,	191	}	,
	{	65	,	0	,	190	}	,
	{	66	,	0	,	189	}	,
	{	67	,	0	,	188	}	,
	{	68	,	0	,	187	}	,
	{	69	,	0	,	186	}	,
	{	70	,	0	,	185	}	,
	{	71	,	0	,	184	}	,
	{	72	,	0	,	183	}	,
	{	73	,	0	,	182	}	,
	{	74	,	0	,	181	}	,
	{	75	,	0	,	180	}	,
	{	76	,	0	,	179	}	,
	{	77	,	0	,	178	}	,
	{	78	,	0	,	177	}	,
	{	79	,	0	,	176	}	,
	{	80	,	0	,	175	}	,
	{	81	,	0	,	174	}	,
	{	82	,	0	,	173	}	,
	{	83	,	0	,	172	}	,
	{	84	,	0	,	171	}	,
	{	85	,	0	,	170	}	,
	{	86	,	0	,	169	}	,
	{	87	,	0	,	168	}	,
	{	88	,	0	,	167	}	,
	{	89	,	0	,	166	}	,
	{	90	,	0	,	165	}	,
	{	91	,	0	,	164	}	,
	{	92	,	0	,	163	}	,
	{	93	,	0	,	162	}	,
	{	94	,	0	,	161	}	,
	{	95	,	0	,	160	}	,
	{	96	,	0	,	159	}	,
	{	97	,	0	,	158	}	,
	{	98	,	0	,	157	}	,
	{	99	,	0	,	156	}	,
	{	100	,	0	,	155	}	,
	{	101	,	0	,	154	}	,
	{	102	,	0	,	153	}	,
	{	103	,	0	,	152	}	,
	{	104	,	0	,	151	}	,
	{	105	,	0	,	150	}	,
	{	106	,	0	,	149	}	,
	{	107	,	0	,	148	}	,
	{	108	,	0	,	147	}	,
	{	109	,	0	,	146	}	,
	{	110	,	0	,	145	}	,
	{	111	,	0	,	144	}	,
	{	112	,	0	,	143	}	,
	{	113	,	0	,	142	}	,
	{	114	,	0	,	141	}	,
	{	115	,	0	,	140	}	,
	{	116	,	0	,	139	}	,
	{	117	,	0	,	138	}	,
	{	118	,	0	,	137	}	,
	{	119	,	0	,	136	}	,
	{	120	,	0	,	135	}	,
	{	121	,	0	,	134	}	,
	{	122	,	0	,	133	}	,
	{	123	,	0	,	132	}	,
	{	124	,	0	,	131	}	,
	{	125	,	0	,	130	}	,
	{	126	,	0	,	129	}	,
	{	127	,	0	,	128	}	,
	{	128	,	0	,	127	}	,
	{	129	,	0	,	126	}	,
	{	130	,	0	,	125	}	,
	{	131	,	0	,	124	}	,
	{	132	,	0	,	123	}	,
	{	133	,	0	,	122	}	,
	{	134	,	0	,	121	}	,
	{	135	,	0	,	120	}	,
	{	136	,	0	,	119	}	,
	{	137	,	0	,	118	}	,
	{	138	,	0	,	117	}	,
	{	139	,	0	,	116	}	,
	{	140	,	0	,	115	}	,
	{	141	,	0	,	114	}	,
	{	142	,	0	,	113	}	,
	{	143	,	0	,	112	}	,
	{	144	,	0	,	111	}	,
	{	145	,	0	,	110	}	,
	{	146	,	0	,	109	}	,
	{	147	,	0	,	108	}	,
	{	148	,	0	,	107	}	,
	{	149	,	0	,	106	}	,
	{	150	,	0	,	105	}	,
	{	151	,	0	,	104	}	,
	{	152	,	0	,	103	}	,
	{	153	,	0	,	102	}	,
	{	154	,	0	,	101	}	,
	{	155	,	0	,	100	}	,
	{	156	,	0	,	99	}	,
	{	157	,	0	,	98	}	,
	{	158	,	0	,	97	}	,
	{	159	,	0	,	96	}	,
	{	160	,	0	,	95	}	,
	{	161	,	0	,	94	}	,
	{	162	,	0	,	93	}	,
	{	163	,	0	,	92	}	,
	{	164	,	0	,	91	}	,
	{	165	,	0	,	90	}	,
	{	166	,	0	,	89	}	,
	{	167	,	0	,	88	}	,
	{	168	,	0	,	87	}	,
	{	169	,	0	,	86	}	,
	{	170	,	0	,	85	}	,
	{	171	,	0	,	84	}	,
	{	172	,	0	,	83	}	,
	{	173	,	0	,	82	}	,
	{	174	,	0	,	81	}	,
	{	175	,	0	,	80	}	,
	{	176	,	0	,	79	}	,
	{	177	,	0	,	78	}	,
	{	178	,	0	,	77	}	,
	{	179	,	0	,	76	}	,
	{	180	,	0	,	75	}	,
	{	181	,	0	,	74	}	,
	{	182	,	0	,	73	}	,
	{	183	,	0	,	72	}	,
	{	184	,	0	,	71	}	,
	{	185	,	0	,	70	}	,
	{	186	,	0	,	69	}	,
	{	187	,	0	,	68	}	,
	{	188	,	0	,	67	}	,
	{	189	,	0	,	66	}	,
	{	190	,	0	,	65	}	,
	{	191	,	0	,	64	}	,
	{	192	,	0	,	63	}	,
	{	193	,	0	,	62	}	,
	{	194	,	0	,	61	}	,
	{	195	,	0	,	60	}	,
	{	196	,	0	,	59	}	,
	{	197	,	0	,	58	}	,
	{	198	,	0	,	57	}	,
	{	199	,	0	,	56	}	,
	{	200	,	0	,	55	}	,
	{	201	,	0	,	54	}	,
	{	202	,	0	,	53	}	,
	{	203	,	0	,	52	}	,
	{	204	,	0	,	51	}	,
	{	205	,	0	,	50	}	,
	{	206	,	0	,	49	}	,
	{	207	,	0	,	48	}	,
	{	208	,	0	,	47	}	,
	{	209	,	0	,	46	}	,
	{	210	,	0	,	45	}	,
	{	211	,	0	,	44	}	,
	{	212	,	0	,	43	}	,
	{	213	,	0	,	42	}	,
	{	214	,	0	,	41	}	,
	{	215	,	0	,	40	}	,
	{	216	,	0	,	39	}	,
	{	217	,	0	,	38	}	,
	{	218	,	0	,	37	}	,
	{	219	,	0	,	36	}	,
	{	220	,	0	,	35	}	,
	{	221	,	0	,	34	}	,
	{	222	,	0	,	33	}	,
	{	223	,	0	,	32	}	,
	{	224	,	0	,	31	}	,
	{	225	,	0	,	30	}	,
	{	226	,	0	,	29	}	,
	{	227	,	0	,	28	}	,
	{	228	,	0	,	27	}	,
	{	229	,	0	,	26	}	,
	{	230	,	0	,	25	}	,
	{	231	,	0	,	24	}	,
	{	232	,	0	,	23	}	,
	{	233	,	0	,	22	}	,
	{	234	,	0	,	21	}	,
	{	235	,	0	,	20	}	,
	{	236	,	0	,	19	}	,
	{	237	,	0	,	18	}	,
	{	238	,	0	,	17	}	,
	{	239	,	0	,	16	}	,
	{	240	,	0	,	15	}	,
	{	241	,	0	,	14	}	,
	{	242	,	0	,	13	}	,
	{	243	,	0	,	12	}	,
	{	244	,	0	,	11	}	,
	{	245	,	0	,	10	}	,
	{	246	,	0	,	9	}	,
	{	247	,	0	,	8	}	,
	{	248	,	0	,	7	}	,
	{	249	,	0	,	6	}	,
	{	250	,	0	,	5	}	,
	{	251	,	0	,	4	}	,
	{	252	,	0	,	3	}	,
	{	253	,	0	,	2	}	,
	{	254	,	0	,	1	}	,
	{	255	,	0	,	0	}	,

};
#endif 

void delay(uint16_t dly)
{
/*
	delay(100) : 68.19 us
	delay(200) : 134 us
	delay(300) : 220 us
	delay(400) : 287 us	
	delay(400) : 287 us	
	delay(450) : 321 us		
	delay(500) : 354 us	
	
	delay(1500) : 1ms	
*/


	while( dly--);
}

void delay_ms(uint16_t ms)
{
	uint16_t i = 0 ;
	for (i=0;i<ms;i++)
	{
		delay(MS_1MS);
	}
}

#if defined (ENABLE_TIMER1_GPIO_TRIG_LED)
void OP_WS2812C_DATA1(void)
{
	endcount = WS_TIM_HIGH;
	flag = WS_FLAG_HIGH;
	count = 0;
	while(count<endcount);

	endcount = WS_TIM_LOW;
	flag = WS_FLAG_LOW;
	count = 0;
	while(count<endcount);	
}

void OP_WS2812C_DATA0(void)
{
	endcount = WS_TIM_LOW;
	flag = WS_FLAG_HIGH;
	count = 0;
	while(count<endcount);

	endcount = WS_TIM_HIGH;
	flag = WS_FLAG_LOW;
	count = 0;
	while(count<endcount);
}


void OP_WS2812C_Send_1Byte(uint8_t Data)
{
	uint8_t i = 0;

	for (i = 0; i < 8; i ++)
	{
		if (Data & 0x80)
		{
			OP_WS2812C_DATA1();
		}
		else
		{
			OP_WS2812C_DATA0();
		}
		Data <<= 1;	
	} 	
}


void OP_WS2812C_Send_1bit(uint8_t Data)
{
	switch(Data)
	{
		case 0: 
			OP_WS2812C_DATA0();				
			break;
		default:
			OP_WS2812C_DATA1();
			break;
	}	
}

void Timer1_IRQHandler(void)
{
	count++;
	P00 = (flag == 1)?(1):(0);
}

void Timer1_ISR(void) interrupt 3        // Vector @  0x1B
{
    TH1 = u8TH1_Tmp;
    TL1 = u8TL1_Tmp;
    clr_TCON_TF1;
	
	Timer1_IRQHandler();
}

void Timer1_GPIO_Initial(void)
{
/*
	WS2812C-2020
	T1H 1 HIGH 580ns~1�gs
	T1L 1 LOW 220ns~420ns
	==> 1111 1100 (0xFC)	==> if FREQ = 6M
	
	T0H 0 HIGH 220ns~380ns
	T0L 0 LOW 580ns~1�gs
	==> 1100 0000 (0xC0)	==> if FREQ = 6M

	1.25 us = (1/24M) * x , 
	x = 30
	=> 1/800 000 = (1/24 000000) , 
	x = sysclock / target freq

	=> target : 0.375 us
	=> Tcy : 1/24M = 0.0417 us

	TH : (65536 - target/Tcy)/256
	TL : (65536 - target/Tcy)%256
*/
	uint16_t res = 0;

	//GPIO	init
	P00_PUSHPULL_MODE;	

	//variable init
	count = 0;
	flag = 0;
	P00 = 0;

	//timer	init
	ENABLE_TIMER1_MODE1;
	TIMER1_FSYS;

	res = 65536 - 300;	//0xFFFF - 0x1FF0;
	u8TH1_Tmp = 0xFF ; //HIBYTE(res);//(SYS_CLOCK/WS_TIM_FREQ)
	u8TL1_Tmp = 0x50 ;	//LOBYTE(res);

    TH1 = u8TH1_Tmp;
    TL1 = u8TL1_Tmp;

    ENABLE_TIMER1_INTERRUPT;                       //enable Timer1 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR1;                                  //Timer1 run

}

#endif

#if defined (ENABLE_GPIO_TRIG_LED)
void GPIO_WS2812C_DATA1(void)
{				
	set_LEDGPIO(1);
	nop;nop;nop;nop;nop;
	set_LEDGPIO(1);
	nop;nop;nop;nop;nop;
	
	set_LEDGPIO(0);
	nop;nop;nop;nop;nop;
}

void GPIO_WS2812C_DATA0(void)
{	
	set_LEDGPIO(1);
	nop;nop;nop;nop;nop;
			
	set_LEDGPIO(0);
	nop;nop;nop;nop;nop;
	set_LEDGPIO(0);
	nop;nop;nop;nop;nop;	
}

void GPIO_WS2812C_Send_1Byte(uint8_t Data)
{
	uint8_t i = 0;

	for (i = 0; i < 8; i ++)
	{
		if (Data & 0x80)
		{
			GPIO_WS2812C_DATA1();
		}
		else
		{
			GPIO_WS2812C_DATA0();
		}
		Data <<= 1;	
	} 	
}


void GPIO_WS2812C_Send_1bit(uint8_t Data)
{
//	P13 = 1;		
	switch(Data)	//	nop; 225ns
	{
		case 0: 
			GPIO_WS2812C_DATA0();				
			break;
		default:
			GPIO_WS2812C_DATA1();
			break;
	}
//	P13 = 0;	
}

void GPIO_WS2812C_Initial(void)
{      
	#if defined (USE_P00)
	P00_PUSHPULL_MODE;														// P00 (MOSI) Quasi mode	
	#elif defined (USE_P01)
	P01_PUSHPULL_MODE;		
	#elif defined (USE_P12)
	P12_PUSHPULL_MODE;	
	#endif
}

#endif

#if defined (ENABLE_SPI_TRIG_LED)
void SPI_Send_Data(uint8_t Data)	//check SPIOVF for FIFO
{
#if 1    

    while(SPSR & SET_BIT2);
    SPDR = Data;  
#else
    SPDR = Data;                       
    while(!(SPSR & SET_BIT7));
    clr_SPIF;
#endif 
}

void SPI_Send_1bit(uint8_t Data)
{
	/* Send SPI1 data */
	switch(Data)
	{
		case 0:       
			SPI_Send_Data(WS_SPI_LOW);  
			break;
		default:      
			SPI_Send_Data(WS_SPI_HIGH);  
			break;
	}
}

void SPI_Initial(uint8_t idx)
{      
	P15_QUASI_MODE;														// P15 (SS) Quasi mode
	P10_QUASI_MODE;														// P10 (SPCLK) Quasi mode
	P00_QUASI_MODE;														// P00 (MOSI) Quasi mode
//	P01_QUASI_MODE;														// P22 (MISO) Quasi mode
    
    set_DISMODF;                                							// SS General purpose I/O ( No Mode Fault ) 
//    clr_SSOE;
    set_SSOE;																// Automatic SS output 
   
    clr_LSBFE;                                  								// MSB first         

//    clr_CPOL;                                   								// The SPI clock is low in idle mode
	set_CPOL;
    set_CPHA;                                   								// The data is sample on the second edge of SPI clock 
    
    set_MSTR;                                   							// SPI in Master mode 

	//TRM page 90
//    SPICLK_DIV2;                        									// Select SPI clock 
	switch(idx)
	{
		case SPI_12M: 
			SPICLK_DIV2;
			break;
		case SPI_6M: 
			SPICLK_DIV4;
			break;	
		case SPI_3M: 
			SPICLK_DIV8;																// set to SPI 3M
			break;
		case SPI_1_5M: 
			SPICLK_DIV16;
			break;				
	}

	set_SPIEN;                                  								// Enable SPI function 
    clr_SPIF;
}
#endif

#if defined (ENABLE_PWM_TRIG_LED)
void PWM_ISR (void) interrupt 13
{
	#if 0
    clr_PWMCON0_PWMF;               // clear PWM interrupt flag
	clr_EIE_EPWM;

//	P13 = 0;
	if ((LOAD) == 0)	//((u16Period) == 0)
	{
//		P14 = 1;
		
		clr_PMEN0;
		clr_PMD0;	//set_PMD0;

//		P14 = 0;
	}

//	#else
    clr_PWMCON0_PWMF;               // clear PWM interrupt flag
	clr_EIE_EPWM;
	if (LOAD == 0)
	{		
		clr_PWMCON0_PWMRUN;	
	}
	#endif
}

void PWMx_CHx_SetDuty(uint16_t d)
{
	uint16_t res = 0 ;
//	uint16_t cnt = 0x1 ;
	
	//USE interrupt ?
//    PWM_INT_PWM0;  
//    PWM_RISING_INT; 
//    set_EIE_EPWM;                  /*Enable PWM interrupt  */
//    ENABLE_GLOBAL_INTERRUPT; 

	u16Period = MAKEWORD(PWMPH,PWMPL);
	res = d*(u16Period+1)/100;

    PWM0H = HIBYTE(res);
    PWM0L = LOBYTE(res);
    set_PWMCON0_LOAD;
    set_PWMCON0_PWMRUN;	

//	res = u16Period-1;
//	while (u16Period-- != 0);

//	while(cnt--);	
//    PWM0H = HIBYTE(0);
//    PWM0L = LOBYTE(0);
//    set_PWMCON0_LOAD;
//    set_PWMCON0_PWMRUN;	

//	clr_PWMCON0_PWMRUN;
}

void PWMx_WS2812C_Send_1byte(uint8_t Data)
{
	#if 1

	uint8_t i = 0;

	for (i = 0; i < 8; i ++)
	{
		if (Data & 0x80)
		{
			PWMx_CHx_SetDuty(WS_PWM_HIGH);
		}
		else
		{
			PWMx_CHx_SetDuty(WS_PWM_LOW);
		}
		Data <<= 1;	
	} 	

	#else
	switch(Data)
	{
		case 0:       
			PWMx_CHx_SetDuty(WS_PWM_LOW);  
			break;
		default:      
			PWMx_CHx_SetDuty(WS_PWM_HIGH);  
			break;
	}	
	#endif
}

void PWMx_CHx_Initial(void)
{
	uint16_t res = 0;

	P12_PUSHPULL_MODE;	//Add this to enhance MOS output capability
    PWM0_P12_OUTPUT_ENABLE;
	
    PWM_IMDEPENDENT_MODE;

	/*
		In  edge-aligned mode, the 16-bit counter  uses single slop  operation  by  counting  up  from  0000H  to 
		{PWMnPH, PWMnPL} and then starting from 0000H.
	*/
	PWM_EDGE_TYPE;
	
    PWM_CLOCK_DIV_2;

//	clr_PWMCON0_PWMRUN;
	
/*
	PWM frequency   = Fpwm/((PWMPH,PWMPL)+1) = (24MHz/2)/((PWMPH,PWMPL)+1) = 800KHz
*/	
	res = (SYS_CLOCK >> 1)/WS_PWM_FREQ - 1;
//	printf("%d\r\n" , res);

	PWMPH = HIBYTE(res);	//HIBYTE((SYS_CLOCK>>1)/uFrequency-1);
    PWMPL = LOBYTE(res);	//LOBYTE((SYS_CLOCK>>1)/uFrequency-1);

//	PWMx_CHx_SetDuty(0);

}
#endif


/*
	DATA COLLECT CPU : 
	18 LED : 3.468 ms
	17 LED : 3.287 ms
	10 LED : 1.947 ms	, latch : 602
	5 LED : 0.965 ms	, latch : 324
	4 LED : 0.784 ms	, latch : 254	
	3 LED : 0.591 ms	, latch : 192
	2 LED : 0.401 ms	, latch : 134	
	1 LED : 0.210 ms	, latch : 79		
*/

void setLED_ResetPulse(uint8_t pos)	//target : 280us
{
	uint16_t i = 0 ;

	#if defined (ENABLE_PWM_TRIG_LED)
	if (pos == WS_RES_POS_FRONT)	//57 us
	{
		for (i = 0 ; i < 7 ; i++)	
		{
			PWMx_CHx_SetDuty(0x00);
		}
	}
	else
	{
		for (i = 0 ; i < 40 ; i++)		//317 us
		{
			PWMx_CHx_SetDuty(0x00);
		}
	}
	#elif defined (ENABLE_SPI_TRIG_LED)	
	if (pos == WS_RES_POS_FRONT)	//57 us
	{
		delay(100);
	}
	else
	{
		delay(MS_LED_LATCH);
	}
	#elif defined (ENABLE_GPIO_TRIG_LED) | defined (ENABLE_TIMER1_GPIO_TRIG_LED)
	set_LEDGPIO(0);
	
	if (pos == WS_RES_POS_FRONT)	//57 us
	{
		delay(100);
	}
	else
	{
		delay(MS_LED_LATCH);
	}
	#endif

}

void setLED_Display(uint16_t DataCount)
{
	uint16_t Count = DataCount;

#if defined (ENABLE_PWM_TRIG_LED)	// PWM unable to stop per 24 bit data
	uint16_t i = 0;
	signed int j = 0;

	setLED_ResetPulse(WS_RES_POS_FRONT);

	for(i=0;i<(LED_NUM);i++)
	{
		PWMx_WS2812C_Send_1byte(DataBuffer[i*3]);
		PWMx_WS2812C_Send_1byte(DataBuffer[i*3+1]);
		PWMx_WS2812C_Send_1byte(DataBuffer[i*3+2]);		
	}
	
	setLED_ResetPulse(WS_RES_POS_BACK); 
	
#elif defined (ENABLE_SPI_TRIG_LED)	// SPI unable to keep pull low under 8 bit clock , unless use DMA
	uint8_t i = 0;
	signed int j = 0;
	
	setLED_ResetPulse(WS_RES_POS_FRONT);	
	for(i=0;i<(LED_NUM);i++)
	{
//		P13 = 1 ;
		for(j=7;j>=0;j--)  
		{
			SPI_Send_1bit(DataBuffer[i*3]  &(1<<j));
		}
		for(j=7;j>=0;j--)  
		{
			SPI_Send_1bit(DataBuffer[i*3+1]&(1<<j));
		}
		for(j=7;j>=0;j--)  
		{
			SPI_Send_1bit(DataBuffer[i*3+2]&(1<<j));
		}
//		P13 = 0;
	}
	setLED_ResetPulse(WS_RES_POS_BACK);
	
#elif defined (ENABLE_GPIO_TRIG_LED)
	uint8_t i = 0;
//	signed int j = 0;
	
	setLED_ResetPulse(WS_RES_POS_FRONT);
	#if 1
//	P13 = 1;	

	clr_EA;
	for(i=0;i<(LED_NUM);i++)
	{
		GPIO_WS2812C_Send_1Byte(DataBuffer[i*3]);
		GPIO_WS2812C_Send_1Byte(DataBuffer[i*3+1]);
		GPIO_WS2812C_Send_1Byte(DataBuffer[i*3+2]);		
	}
	set_EA;

	#else
	for(i=0;i<(LED_NUM);i++)
	{
		for(j=7;j>=0;j--)  
		{
			GPIO_WS2812C_Send_1bit(DataBuffer[i*3]  &(1<<j));
		}
		for(j=7;j>=0;j--)  
		{
			GPIO_WS2812C_Send_1bit(DataBuffer[i*3+1]&(1<<j));
		}
		for(j=7;j>=0;j--)  
		{
			GPIO_WS2812C_Send_1bit(DataBuffer[i*3+2]&(1<<j));
		}
		
	}
	#endif
//	P13 = 0;	
	setLED_ResetPulse(WS_RES_POS_BACK);
	
#elif defined (ENABLE_TIMER1_GPIO_TRIG_LED)
	uint8_t i = 0;
	
	setLED_ResetPulse(WS_RES_POS_FRONT);
	
	for(i=0;i<(LED_NUM);i++)
	{
		OP_WS2812C_Send_1Byte(DataBuffer[i*3]);
		OP_WS2812C_Send_1Byte(DataBuffer[i*3+1]);
		OP_WS2812C_Send_1Byte(DataBuffer[i*3+2]);		
	}
	
	setLED_ResetPulse(WS_RES_POS_BACK);
#endif
}

void setLED_BufferClear(void)
{
	memset(DataBuffer,0x00,LED_DATA_LEN);
}

void setLED_Color(uint8_t DeviceNumber ,uint8_t RED, uint8_t GREEN, uint8_t BLUE)	// Output a colour Format: GGRRBB	
{
	uint16_t index = 0;	
	uint8_t i = 0;
	#if defined (ENABLE_SPI_TRIG_LED) | defined (ENABLE_GPIO_TRIG_LED) | defined (ENABLE_PWM_TRIG_LED) | defined (ENABLE_TIMER1_GPIO_TRIG_LED)	
	DataBuffer[DeviceNumber*3]   = GREEN;
	DataBuffer[DeviceNumber*3+1] = RED;
	DataBuffer[DeviceNumber*3+2] = BLUE;

	#elif defined (ENABLE_PWM_TRIG_LED__NOT_USE)
	for	(i=0; i<8; i++) 	// GREEN data
	{
		DataBuffer[index + DeviceNumber*24] = ((GREEN<<i) & 0x80)?(WS_PWM_HIGH):(WS_PWM_LOW);
//		printf("G :0x%2X\r\n",DataBuffer[index + DeviceNumber*24]);
		index++ ;
	}
	for	(i=0; i<8; i++) 	// RED
	{
		DataBuffer[index + DeviceNumber*24] = ((RED<<i) & 0x80)?(WS_PWM_HIGH):(WS_PWM_LOW);
//		printf("R :0x%2X\r\n",DataBuffer[index + DeviceNumber*24]);
		index++ ;
	}
	for	(i=0; i<8; i++) 	// BLUE
	{
		DataBuffer[index + DeviceNumber*24] = ((BLUE<<i) & 0x80)?(WS_PWM_HIGH):(WS_PWM_LOW);
//		printf("B :0x%2X\r\n",DataBuffer[index + DeviceNumber*24]);
		index++ ;
	}
	#endif
}

void setLED_ColorWipe(uint8_t RED, uint8_t GREEN, uint8_t BLUE) 
{
	uint8_t index = 0 ;

	for (index = 0; index < LED_NUM; index++)
	{
		setLED_Color(index, RED, GREEN, BLUE);
	}

	setLED_Display(LED_DATA_LEN);
}

//https://github.com/fduignan/NucleoF042_SingleWS2812B/blob/master/main.c
void _stripEffect_Rainbow(uint8_t* RED, uint8_t* GREEN, uint8_t* BLUE)	
{   
	// Cycle through the colours of the rainbow (non-uniform brightness however)
	// Inspired by : http://academe.co.uk/2012/04/arduino-cycling-through-colours-of-the-rainbow/
	
	static uint8_t Red = 255;
	static uint8_t Green = 0;
	static uint8_t Blue = 0;
	static uint8_t State = 0;
	switch (State)
	{
		case 0:{
			Green++;
			if (Green == 255)
				State = 1;
			break;
		}
		case 1:{
			Red--;
			if (Red == 0)
				State = 2;
			break;
		}
		case 2:{
			Blue++;
			if (Blue == 255)
				State = 3;			
			break;
		}
		case 3:{
			Green--;
			if (Green == 0)
				State = 4;
			break;
		}
		case 4:{
			Red++;
			if (Red == 255)
				State = 5;
			break;
		}
		case 5:{
			Blue --;
			if (Blue == 0)
				State = 0;
			break;
		}		
	}

	*GREEN = Green;
	*RED = Red;
	*BLUE = Blue;	
//	return (Green << 16) + (Red << 8) + Blue;


}

uint8_t _stripEffect_Wheel(uint8_t WheelPos , uint8_t* RED, uint8_t* GREEN, uint8_t* BLUE) 
{
	WheelPos = 255 - WheelPos;
	if(WheelPos < 85) 
	{
		*GREEN = 0;
		*RED = 255 - WheelPos * 3;
		*BLUE = WheelPos * 3;
		return 0;
		
//		return PACK_RGB(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if(WheelPos < 170) 
	{
		WheelPos -= 85;

		*GREEN = WheelPos * 3;
		*RED = 0;
		*BLUE = 255 - WheelPos * 3;
		return 0;
		
//		return PACK_RGB(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	
	WheelPos -= 170;

	*GREEN = 255 - WheelPos * 3;
	*RED = WheelPos * 3;
	*BLUE = 0;
	return 0;
	
//	return PACK_RGB(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//void getFromAToB(uint16_t time, uint16_t steps, 
//		uint8_t redA,	uint8_t greenA, uint8_t blueA, 
//		uint8_t redB, uint8_t greenB, uint8_t blueB) 
//{
//	uint16_t i = 0;
//	signed int deltaRed, deltaGreen, deltaBlue;

//	deltaRed = ((signed long) redB - (signed long) redA) / (signed long) steps;
//	deltaGreen = ((signed long) greenB - (signed long) greenA) / (signed long) steps;
//	deltaBlue = ((signed long) blueB - (signed long) blueA) / (signed long) steps;

//	for (i = 0; i < steps; i++) 
//	{
//		setLED_ColorWipe(redA + i * deltaRed, greenA + i * deltaGreen,	+blueA + i * deltaBlue);
//	
//		delay(time / steps);
//	}
//}

void getCircularRing(uint16_t time, uint8_t red, uint8_t green,uint8_t blue) 
{
	uint8_t index = 0 ;

	for (index = 0; index < LED_NUM; index++) 
	{
		setLED_ColorWipe(0, 0, 0);
		
		setLED_Color(index, red, green, blue);

    	setLED_Display(LED_DATA_LEN);

		delay_ms(time);
	}
}

// the interval should be in the 300-1000 range for best visual effect
//void getHeartBeat(uint16_t time, uint8_t red, uint8_t green,uint8_t blue) 
//{
//	uint8_t redInc, greenInc, blueInc;
//	uint8_t maxRed, maxGreen, maxBlue;

//	uint8_t index = 0 ;
//	uint8_t beatcnt = 35 ;
//	
//	redInc = red / HEARTBEAT_STEPS;
//	greenInc = green / HEARTBEAT_STEPS;
//	blueInc = blue / HEARTBEAT_STEPS;

//	maxRed = redInc * (HEARTBEAT_STEPS - 1);
//	maxGreen = greenInc * (HEARTBEAT_STEPS - 1);
//	maxBlue = blueInc * (HEARTBEAT_STEPS - 1);

//	setLED_ColorWipe(0, 0, 0);

//	// first stroke
//	for (index = 0; index < HEARTBEAT_STEPS; index++) 
//	{
//		setLED_ColorWipe(index * redInc, index * greenInc, index * blueInc);
//	
//		delay(time / (beatcnt+15));	//50
//	}
//	for (index = 0; index < HEARTBEAT_STEPS; index++) 
//	{
//		setLED_ColorWipe(maxRed - index * redInc, maxGreen - index * greenInc,maxBlue - index * blueInc);
//		
//		delay(time / (beatcnt+10));	//45
//	}
//	
//	// second stroke
//	for (index = 0; index < HEARTBEAT_STEPS; index++) 
//	{
//		setLED_ColorWipe(index * redInc, index * greenInc, index * blueInc);
//		
//		delay(time / (beatcnt+5));	//40
//	}
//	for (index = 0; index < HEARTBEAT_STEPS; index++) 
//	{
//		setLED_ColorWipe(maxRed - index * redInc, maxGreen - index * greenInc,maxBlue - index * blueInc);
//		
//		delay(time / (beatcnt));	//35
//	}
//	// relax..
//	delay_ms(time);

//}

#if defined (ENABLE_EXTRA_DEMO)
void getAllColors(uint16_t time) 
{
	uint16_t index = 0 ;
	uint8_t led = 0;

	for (index = 0;index < 766;index++)
	{
		#if 1
		for (led = 0; led < LED_NUM; led++)
		{
			setLED_Color(led,colorsFull[index][0],colorsFull[index][1],colorsFull[index][2]);			
		}
	
		#else
		for (led = 0; led < LED_NUM; led++)
		{
			setLED_Color(led,colorsFull[index + led][0],colorsFull[index + led][1],colorsFull[index + led][2]);
			
		}
	
		if ((index + led) >= 766)
			index = 0;
		#endif

		setLED_Display(LED_DATA_LEN);
		
		delay_ms(time);
	}
}

//void getColorWheel(uint16_t time) 
//{
//	uint16_t index = 0 ;
//	uint8_t led = 0, colorIndex = 0;

//	for (led = 0; led < LED_NUM; led++) 
//	{
//		colorIndex = (index + led * (766 / LED_NUM)) % 766;
//		
//		setLED_Color(led, colorsFull[colorIndex][0],	colorsFull[colorIndex][1], colorsFull[colorIndex][2]);

//		setLED_Display(LED_DATA_LEN);

//		delay_ms(time);	

//		index += (766 / LED_NUM);
//		index %= 766;		
//	}
//}
#endif /*ENABLE_EXTRA_DEMO*/

void getPatternMove(uint16_t time, uint16_t parts, uint8_t red,uint8_t green, uint8_t blue) 
{
	uint8_t index = 0 ;
	uint16_t indexStep;
	uint16_t i;

	indexStep = LED_NUM / parts;

	setLED_ColorWipe(0, 0, 0);

	for (index = 0; index < LED_NUM; index++) 
	{
		setLED_ColorWipe(0, 0, 0);
		
		for (i = 0; i < parts; i++)
		{
			setLED_Color(index + i * indexStep, red, green, blue);
		}

		setLED_Display(LED_DATA_LEN);
		
		delay_ms(time);
	}	
}

void getFullEmpty(uint16_t time, uint8_t red, uint8_t green,uint8_t blue) 
{
	uint8_t index = 0 ;

	setLED_ColorWipe(0, 0, 0);

	for (index = 0; index < LED_NUM; index++) 
	{
		setLED_Color(index, red, green, blue);

		setLED_Display(LED_DATA_LEN);
		
		delay_ms(time);
	}

	for (index = 0; index < LED_NUM; index++) 
	{
		setLED_Color(index,0, 0, 0);

		setLED_Display(LED_DATA_LEN);
		
		delay_ms(time);
	}
}

void getAlternateColors(uint16_t time, uint16_t steps,
		uint8_t redA, uint8_t greenA, uint8_t blueA, 
		uint8_t redB, uint8_t greenB, uint8_t blueB) 
{
	uint16_t i = 0;
	signed long  deltaRed, deltaGreen, deltaBlue;

	deltaRed = ((signed long) redB - (signed long) redA) / steps;
	deltaGreen = ((signed long) greenB - (signed long) greenA) / steps;
	deltaBlue = ((signed long) blueB - (signed long) blueA) / steps;

	for (i = 0; i < steps; i++) 
	{
		setLED_ColorWipe(redA + i * deltaRed, greenA + i * deltaGreen,	+blueA + i * deltaBlue);

		delay(time / steps);
	}
	for (i = steps - 1; i > 0; i--) 
	{
		setLED_ColorWipe(redA + i * deltaRed, greenA + i * deltaGreen,	+blueA + i * deltaBlue);
		
		delay(time / steps);
	}
}


void getRGBSequence(uint16_t time) 
{
	static uint8_t intensity = 0;
	static uint8_t sign = 0;

    uint8_t i = 0;
    if (sign) 
	{
        intensity++;
        if (intensity == 255) 
            sign = 0; 
    }
    else 
	{
        intensity--;
        if (intensity == 0)
            sign = 1;
    }

    for (i = 0; i < LED_NUM; i++) 
	{
        setLED_Color(i, intensity*((i%3)%2),intensity*(((i+1)%3)%2),intensity*(((i+2)%3)%2));

	    setLED_Display(LED_DATA_LEN);
		
	    delay(time);			
    }

}

void getTheaterChaseRainbow(uint16_t time) 
{
	uint16_t j = 0;
	uint8_t q = 0; 
	uint16_t i=0;
	uint8_t red , green, blue = 0;
	
	for (j=0; j < 256; j++) 
	{     // cycle all 256 colors in the wheel
		for (q=0; q < 3; q++) 
		{
			for (i=0; i < LED_NUM ; i=i+3) 
			{				
				_stripEffect_Wheel(( (i+j) % 255),&red,&green,&blue);
				setLED_Color(i+q, red,green,blue);
			
//				setLED_Color(i+q, 
//					UNPACK_R(_stripEffect_Wheel( (i+j) % 255)),
//					UNPACK_G(_stripEffect_Wheel( (i+j) % 255)),
//					UNPACK_B(_stripEffect_Wheel( (i+j) % 255)));    //turn every third pixel on
			}
			setLED_Display(LED_DATA_LEN);

			delay_ms(time);

			for (i=0; i < LED_NUM ; i=i+3) 
			{
				setLED_Color(i+q, 0 , 0 , 0);        //turn every third pixel off
			}
		}
	}
}

void getTheaterChase(uint16_t time ,uint8_t RED, uint8_t GREEN, uint8_t BLUE) 
{
	uint16_t j = 0;
	uint16_t q = 0;
	uint16_t i = 0;

	for (j=0; j<10; j++) 
	{  //do 10 cycles of chasing
		for (q=0; q < 3; q++) 
		{
			for (i=0; i < LED_NUM; i=i+3) 
			{
				setLED_Color(i+q, RED,GREEN,BLUE);    //turn every third pixel on
			}
			setLED_Display(LED_DATA_LEN);

			delay_ms(time);

			for (i=0; i < LED_NUM; i=i+3) 
			{
				setLED_Color(i+q, 0 , 0 , 0);        //turn every third pixel off
			}
		}
	}
}


void getRainbowCycle(uint8_t cycle , uint16_t time) 
{
	uint16_t i = 0, j = 0;
	uint8_t red , green, blue = 0;
	
	for(j=0; j<256*cycle; j++) 
	{ 
		// 5 cycles of all colors on wheel
		for(i=0; i< LED_NUM; i++) 
		{
			_stripEffect_Wheel((((i * 256 / LED_NUM) + j) & 255),&red,&green,&blue);
			setLED_Color(i,red,green,blue);
		
//			setLED_Color(i, 
//				UNPACK_R(_stripEffect_Wheel(((i * 256 / LED_NUM) + j) & 255)),
//				UNPACK_G(_stripEffect_Wheel(((i * 256 / LED_NUM) + j) & 255)),
//				UNPACK_B(_stripEffect_Wheel(((i * 256 / LED_NUM) + j) & 255)));
		}
		
		setLED_Display(LED_DATA_LEN);

		delay_ms(time);
	}
}

void getRainbow(uint16_t time)
{
	uint8_t i = 0;
	uint8_t red , green, blue = 0;

	for (i=0;i<LED_NUM;i++)
	{
		_stripEffect_Rainbow(&red,&green,&blue);
		setLED_Color(i,red,green,blue); // Output a colour Format: GGRRBB							
	}
	
	// Now send out the bits to the SPI bus
	setLED_Display(LED_DATA_LEN);  	

	delay_ms(time);	
}

void getBreathing(uint16_t time)
{
	uint8_t i = 0;
	uint16_t j = 0;
	static uint8_t flag = 1;
	uint8_t led = 0;
	uint16_t index = 0 ;
	uint8_t red,green,blue = 0;

	red = colorsFull[index][0];
	green = colorsFull[index][1];
	blue = colorsFull[index][2];
	
	if (flag)
	{
		for (led = 0; led < LED_NUM; led++)
		{
			setLED_Color(led,red,green,blue);			
		}
		setLED_Display(LED_DATA_LEN);
		delay_ms(time);	
		flag = 0;
	}
	else
	{
		for (j=0xFF;j>0x7F;j--)
		{
			setLED_ColorWipe(j,j,j);
			delay_ms(time);
		}
		flag = 1;
	}

	if ((index++) > 766)
	{
		index = 0;
	}
	
}

void SimpleTest(void)
{
//	uint16_t i = 0;
//	static uint8_t flag = 1;
//	if (flag)
//	{
//		for (i = 0x00 ;i < 0xFF; i++)
//		{
//			setLED_ColorWipe(i,i,i);
//			delay_ms(60);
//		}
//		flag = 0;
//	}
//	else
//	{
//		for (i = 0xFF ;i > 0x00; i--)
//		{
//			setLED_ColorWipe(i,i,i);
//			delay_ms(60);
//		}		
//		flag = 1;
//	}

//	PWMx_CHx_SetDuty(WS_PWM_HIGH);
//	delay_ms(50);

//	PWMx_CHx_SetDuty(WS_PWM_LOW);
//	delay_ms(50);

//	setLED_BufferClear();
//	setLED_ColorWipe(0xFF, 0x00, 0x00);
//	setLED_Color(0, 0xFF, 0x00, 0x00);
//	setLED_Display(LED_DATA_LEN);
//	delay_ms(300);

//	setLED_ColorWipe(0x00, 0x00, 0x00);
//	delay_ms(1500);

//	setLED_ColorWipe(0xFF, 0x01, 0x02);
//	delay_ms(1);
//	setLED_ColorWipe(0x00, 0xFF, 0x00);
//	delay_ms(1500);
//	setLED_ColorWipe(0x00, 0x00, 0xFF);
//	delay_ms(1500);	
//	setLED_ColorWipe(0x00, 0xFF, 0x00);
//	delay_ms(1500);
//	setLED_ColorWipe(0xFF, 0x00, 0x00);
//	delay_ms(1500);
//	setLED_ColorWipe(0x00, 0xFF, 0x00);
//	delay_ms(1500);
//	setLED_ColorWipe(0x00, 0x00, 0xFF);
//	delay_ms(1500);		
//	setLED_ColorWipe(0x00, 0x00, 0x00);
//	delay_ms(1500);

//	setLED_ColorWipe(0xFF, 0xFF, 0xFF);
//	delay_ms(1500);

//	OP_WS2812C_Send_1bit(1);
//	delay_ms(1);
//	OP_WS2812C_Send_1bit(0);
//	delay_ms(1);


//	GPIO_WS2812C_DATA1();;
//	delay_ms(1);
//	GPIO_WS2812C_DATA0();;
//	delay_ms(1);	
//	GPIO_WS2812C_Send_1bit(0);
//	GPIO_WS2812C_Send_1bit(0);
//	GPIO_WS2812C_Send_1bit(0);
//	GPIO_WS2812C_Send_1bit(0);
//	GPIO_WS2812C_Send_1bit(0);	
//	delay_ms(1);
	
//	delay(1);
//	P00 = 0;
//	delay(1);
//	P00 = 1;

//	setLED_ColorWipe(0xFF, 0x00, 0x00);
//	delay_ms(1500);

//	getBreathing(30);
	
//	getRainbow(30);	//OK
//	getRainbowCycle(1 ,5);//OK
//	getTheaterChase(60,0x3F,0x3F,0x00);//OK
//	getTheaterChaseRainbow(1);//OK
//	getRGBSequence(3);//OK
//	getAlternateColors(50, 16, 0xFF, 0, 0, 0, 0, 0xFF);//OK
//	getFullEmpty(30, 0x3F, 0x3F, 0x3F);//OK
//	getPatternMove(30, 1, 0x3F, 0x3F, 0x00);//OK
//	getHeartBeat(1000, HEARTBEAT_STEPS*4, HEARTBEAT_STEPS*0, HEARTBEAT_STEPS*2);
//	getCircularRing(30, 0xFF, 0xFF, 0xFF);
//	getFromAToB(3, 4, 0x3F, 0, 0, 0xFF, 0, 0xFF);
//	getColorWheel(5);
//	getAllColors(1);
			
//	setLED_ColorWipe(0xFF, 0xFF, 0xFF);//test
//	SPI_Send_Data(0x01);

//	setLED_ColorWipe(0xFF,0xFF,0xFF);
//	delay_ms(300);	
//	setLED_ColorWipe(0x00,0x00,0x00);
//	delay_ms(300);	

	P13 = 1;
	setLED_ColorWipe(255,0,0);
	P13 = 0;	
		
}

void StateMachine(void)
{

	if (DemoState++ >= state_Max)
	{
		DemoState = state_Default+1;
	}
//	DemoState = state_AllColors;	//quick test

	#if defined (ENABLE_UART0_LOG)
	printf("demo : 0x%X\r\n",DemoState);
	#endif

	switch(DemoState)
	{
		case state_Rainbow:
			getRainbow(50);
			break;

		case state_RainbowCycle:
			getRainbowCycle(1 ,5);	//(5 , 1000);
			break;

		case state_TheaterChase:
			getTheaterChase(60,0xFF,0x00,0x00);
//			getTheaterChase(60,0x00,0xFF,0x00);
//			getTheaterChase(60,0x00,0x00,0xFF);
		
			break;

		case state_TheaterChaseRainbow:
			getTheaterChaseRainbow(1);
			break;

		case state_RGBSequence:
			getRGBSequence(3);
			break;
		
		case state_AlternateColors:
			getAlternateColors(50, 16, 0xFF, 0, 0, 0, 0, 0xFF);
			break;

		case state_FullEmpty:
			getFullEmpty(30, 0x3F, 0x3F, 0x3F);
			break;
		
		case state_PatternMove:
			getPatternMove(30, 1, 0x3F, 0x3F, 0x00);
			break;

		#if defined (ENABLE_EXTRA_DEMO)
//		case state_ColorWheel:	//not obviously
//			getColorWheel(5);	
//			break;

		case state_AllColors:
			getAllColors(1);	
			break;
		#endif	/*ENABLE_EXTRA_DEMO*/

//		case state_HeartBeat:	//not obviously
//			getHeartBeat(700, 64, 0, 16);
//			break;	

		case state_CircularRing:
			getCircularRing(30, 0x3F, 0x3F, 0x3F);
			break;	
		
//		case state_FromAToB:	//not obviously
//			getFromAToB(3, 4, 0x3F, 0, 0, 0xFF, 0, 0xFF);
//			break;	
			
	}

	delay_ms(DEMO_MS);
}

#if defined (ENABLE_GPIO_DEMO)
//void GPIO_Toggle(void)
//{
//    P12 ^= 1;
//}

void GPIO_Init(void)
{
//    P12_PUSHPULL_MODE;
    P13_PUSHPULL_MODE;	
	P14_PUSHPULL_MODE;	
}
#endif

#if defined (ENABLE_UART0_LOG)
void SendString(uint8_t* Data)
{
	#if 1
	uint16_t i = 0;

	while (Data[i] != '\0')
	{
		UART_Send_Data(UART0,Data[i++]);		
	}

	#endif

	#if 0
	uint16_t i = 0;
	
	for(i = 0;i< (strlen(Data)) ;i++ )
	{
		UART_Send_Data(UART0,Data[i]);
	}
	#endif

	#if 0
    while(*Data)  
    {  
        UART_Send_Data(UART0, (unsigned char) *Data++);  
    } 
	#endif
}


void UART0_Init(void)
{
    UART_Open(SYS_CLOCK,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF;
}
#endif

#if defined (ENABLE_TIMER0_1MS_IRQ)
void Timer0_IncCnt(void)
{
	u16Cnt++;
}

void Timer0_SetCnt(uint16_t res)
{
	u16Cnt = res;
}

uint16_t Timer0_GetCnt(void)
{
	return u16Cnt;
}

void Timer0_IRQHandler(void)
{
//	static uint16_t LOG_TIMER = 0;
//	static uint16_t CNT_TIMER = 0;
//	static uint16_t CNT_GPIO = 0;
//	static uint16_t CNT_DEMO = 0;

//	if (CNT_GPIO++ >= GPIO_TOGGLE_MS)
//	{		
//		CNT_GPIO = 0;
//		#if defined (ENABLE_GPIO_DEMO)
//		GPIO_Toggle();
//		#endif
//	}	

//	if (CNT_TIMER++ >= TIMER_LOG_MS)
//	{		
//		CNT_TIMER = 0;
//    	printf("LOG:%d\r\n",LOG_TIMER++);
//    	printf("u16DataAdjust:%d\r\n",u16DataAdjust);
//		SendString("LOG_TIMER\r\n");
//	}

//	if (CNT_DEMO++ >= DEMO_MS)
//	{		
//		CNT_DEMO = 0;		
//	}	

	if (Timer0_GetCnt()>=1)
	{
		Timer0_IncCnt();
	}

}

void Timer0_ISR(void) interrupt 1        // Vector @  0x0B
{
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    clr_TCON_TF0;
	
	Timer0_IRQHandler();
}

void BasicTimer_TIMER0_Init(void)
{
	uint16_t res = 0;

	ENABLE_TIMER0_MODE1;
	
	u8TH0_Tmp = HIBYTE(TIMER_DIV12_VALUE_1ms_FOSC_240000);
	u8TL0_Tmp = LOBYTE(TIMER_DIV12_VALUE_1ms_FOSC_240000); 

    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;

    ENABLE_TIMER0_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR0;                                  //Timer0 run
}
#endif

void SYS_Init(void)
{
	MODIFY_HIRC_24();

    ALL_GPIO_QUASI_MODE;
    ENABLE_GLOBAL_INTERRUPT;                // global enable bit	
}

void main (void) 
{
    SYS_Init();

	#if defined (ENABLE_UART0_LOG)
    UART0_Init();
	#endif

	#if defined (ENABLE_GPIO_DEMO)
	GPIO_Init();					//P12 , GPIO
	#endif

	#if defined (ENABLE_PWM_TRIG_LED)
	PWMx_CHx_Initial();
	#elif defined (ENABLE_SPI_TRIG_LED)	
	SPI_Initial(SPI_6M);
	#elif defined (ENABLE_GPIO_TRIG_LED)
	GPIO_WS2812C_Initial();
	#elif defined (ENABLE_TIMER1_GPIO_TRIG_LED)
	Timer1_GPIO_Initial();
	
	#endif
	

	#if defined (ENABLE_TIMER0_1MS_IRQ)
	BasicTimer_TIMER0_Init();
	#endif

	P13_PUSHPULL_MODE;	
	
    while(1)
    {		
		StateMachine();
//		SimpleTest();
    }
}



