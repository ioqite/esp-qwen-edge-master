// ArduinoCalc 真机调试程序：详细输出大数运算的每一步内部状态
// 烧录到 ESP32 后通过串口(115200)查看输出
#include "ArduinoCalc.hpp"

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n========== ArduinoCalc 真机调试 ==========\n");

    ArduinoCalc calc;
    double result;
    String outStr, errMsg;

    // ---------- 用例 1: |0xFF ----------
    Serial.println("--- 用例 1: |0xFF ---");
    {
        ArduinoCalc c; double r; String o, e;
        bool ok = c.calculate("|0xFF", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  errMsg 长度: " + String(e.length()) + " 内容: [" + e + "]");
        Serial.println("  outStr 长度: " + String(o.length()) + " 内容: [" + o + "]");
        // 逐字节 dump outStr 前 64 字节
        Serial.print("  outStr hex dump: ");
        for (int i = 0; i < o.length() && i < 64; i++) {
            char buf[8]; snprintf(buf, sizeof(buf), "%02X ", (unsigned char)o[i]);
            Serial.print(buf);
        }
        Serial.println();
    }

    // ---------- 用例 2: |0b1010 ----------
    Serial.println("\n--- 用例 2: |0b1010 ---");
    {
        ArduinoCalc c; double r; String o, e;
        bool ok = c.calculate("|0b1010", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  errMsg 长度: " + String(e.length()) + " 内容: [" + e + "]");
        Serial.println("  outStr 长度: " + String(o.length()) + " 内容: [" + o + "]");
    }

    // ---------- 用例 3: |0o10 ----------
    Serial.println("\n--- 用例 3: |0o10 ---");
    {
        ArduinoCalc c; double r; String o, e;
        bool ok = c.calculate("|0o10", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  errMsg 长度: " + String(e.length()) + " 内容: [" + e + "]");
        Serial.println("  outStr 长度: " + String(o.length()) + " 内容: [" + o + "]");
    }

    // ---------- 用例 4: 大数加法 ----------
    Serial.println("\n--- 用例 4: 99999999999999999999+1 ---");
    {
        ArduinoCalc c; double r; String o, e;
        Serial.println("  调用前: outStr 长度=" + String(o.length()) + " errMsg 长度=" + String(e.length()));
        bool ok = c.calculate("99999999999999999999+1", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  errMsg 长度: " + String(e.length()) + " 内容: [" + e + "]");
        Serial.println("  outStr 长度: " + String(o.length()) + " 内容: [" + o + "]");
    }

    // ---------- 用例 5: 大数乘法 ----------
    Serial.println("\n--- 用例 5: 12345678901234567890*2 ---");
    {
        ArduinoCalc c; double r; String o, e;
        bool ok = c.calculate("12345678901234567890*2", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  errMsg 长度: " + String(e.length()) + " 内容: [" + e + "]");
        Serial.println("  outStr 长度: " + String(o.length()) + " 内容: [" + o + "]");
    }

    // ---------- 用例 6: 大数幂 ----------
    Serial.println("\n--- 用例 6: 2^100 ---");
    {
        ArduinoCalc c; double r; String o, e;
        bool ok = c.calculate("2^100", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  errMsg 长度: " + String(e.length()) + " 内容: [" + e + "]");
        Serial.println("  outStr 长度: " + String(o.length()) + " 内容: [" + o + "]");
    }

    // ---------- 用例 7: 简单大数加法（无 +1） ----------
    Serial.println("\n--- 用例 7: 99999999999999999999 (单数字) ---");
    {
        ArduinoCalc c; double r; String o, e;
        bool ok = c.calculate("99999999999999999999", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  errMsg 内容: [" + e + "]");
        Serial.println("  outStr 内容: [" + o + "]");
    }

    // ---------- 用例 8: 2^10 (小幂) ----------
    Serial.println("\n--- 用例 8: 2^10 ---");
    {
        ArduinoCalc c; double r; String o, e;
        bool ok = c.calculate("2^10", r, e, o);
        Serial.println("  calculate 返回: " + String(ok ? "true" : "false"));
        Serial.println("  result (double): " + String(r, 6));
        Serial.println("  outStr 内容: [" + o + "]");
    }

    // ---------- 内存统计 ----------
    Serial.println("\n--- 内存统计 ---");
    Serial.print("  ESP32 free heap: "); Serial.println(ESP.getFreeHeap());
    Serial.print("  ESP32 max alloc: "); Serial.println(ESP.getMaxAllocHeap());

    Serial.println("\n========== 调试结束 ==========");
}

void loop() {
    vTaskDelete(nullptr);
}
