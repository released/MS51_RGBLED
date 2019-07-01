# MS51_RGBLED

update @ 2019/07/01

Only define ENABLE_GPIO_TRIG_LED available

SPI (ENABLE_SPI_TRIG_LED) , 

1. DATA timing meet WS2812C spec , 

2. however , unable to keep low in 8 bit clock (need DMA , which MS51 don't have it)

PWM (ENABLE_PWM_TRIG_LED) , 

1. DATA timing meet WS2812C spec , RESET timing meet WS2812C spec

2. however , unable to control only send 24 bit data per LED

Scenario : 

1. collect data by setLED_Color to fill DataBuffer array

2. display data by setLED_Display , also include reset pulse in front and back

- Demo procedure , change demo state per 5000 ms

- enable ENABLE_GPIO_DEMO for GPIO toggle , to monitor each waveform status
