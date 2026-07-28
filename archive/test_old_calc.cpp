// #include <Arduino.h>
// #include "ArduinoCalc.hpp"

// void setup() {
//     Serial.begin(115200);
    
//     for (String input : {
//             "3 + 5", 
//             "10 - 2 * 3", 
//             "2 ^ 3 ^ 2", 
//             "-5 + 3 =", 
//             "4 / 4 + 1", 
//             "3 + (4 - 1) * 2 = ", 
//             "3 + (4 - 1 * 2)"    }) {
        
//         double result;
//         String errMsg; // 用于接收错误信息的字符串
//         bool success = calculate(input, result, errMsg);
        
//         if (success) {
//             Serial.println("输入: " + input);
//             Serial.println("结果: " + String(result, 4)); // 输出结果，保留4位小数
//             Serial.println("-------------------------");
//         } else {
//             // 如果失败，统一输出累积的错误信息
//             Serial.println("输入: " + input);
//             Serial.print(errMsg);
//             Serial.println("-------------------------");
//         }
//     }
// }

// void loop() {}
