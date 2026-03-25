#include  "leds.h"


void LEDS_Init(void) {
    /* 开启 GPIOC 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    /* PC13 配置为推挽输出 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* 默认关闭 LED，PC13 输出低电平 ，初始化时PC13亮*/
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);

    
}

/*PC13关闭，当WIFI 配网成功时*/
void LEDS_On(void){
        /* 打开 LED，PC13 输出低电平 */
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}


/*PC13闪烁，当蓝牙模块开始配网时*/
void LEDS_Blinking(void){
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
	Delay_S(1);
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
}
