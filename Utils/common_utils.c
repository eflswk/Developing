#include "common_utils.h"

/*
 * 函数功能：解析蓝牙发送的 WiFi 配置字符串
 * 字符串格式：!NAME=PASSWORD!
 * 例如：!MyWiFi=12345678!
 * 解析结果：
 *      Name     -> MyWiFi
 *      Password -> 12345678
 *
 * 实现思路：
 *      第一个 for 循环提取 '=' 左侧的 WiFi Name
 *      第二个 for 循环提取 '=' 与 '!' 之间的密码
 */
void SplitWiFiInfo(char *Src, char *Name, char *Password) {
    int i;
    int j;

    /* 提取 WiFi 名称，第一个字符是感叹号，从第二个字符开始切 */
    for (i = 1; Src[i] != '='; i++) {
        Name[i - 1] = Src[i];
    }

    Name[i - 1] = '\0';

    /* 提取 WiFi 密码 */
    for (j = 0; Src[i + 1 + j] != '!'; j++) {
        Password[j] = Src[i + 1 + j];
    }

    Password[j] = '\0';
}
