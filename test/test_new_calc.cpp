// #include "ArduinoCalc.hpp"

// // 测试框架
// class TestRunner {
// private:
//     int passed = 0;
//     int failed = 0;
//     std::vector<String> failures;

// public:
//     void test(const String& name, bool condition, const String& detail = "") {
//         if (condition) {
//             passed++;
//             Serial.println("[PASS] " + name);
//         } else {
//             failed++;
//             String msg = name + (detail.isEmpty() ? "" : " - " + detail);
//             failures.push_back(msg);
//             Serial.println("[FAIL] " + msg);
//         }
//     }

//     void printSummary() {
//         Serial.println("\n========================================");
//         Serial.println("测试结果: " + String(passed) + " 通过, " + String(failed) + " 失败");
//         if (failed > 0) {
//             Serial.println("\n失败详情:");
//             for (const auto& f : failures) {
//                 Serial.println("  - " + f);
//             }
//         }
//         Serial.println("========================================");
//     }
// };

// TestRunner runner;


// // ==================== 测试用例 ====================

// void testBasicArithmetic() {
//     Serial.println("\n--- 基础算术运算测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 加法
//     calc.calculate("1+2", result, errMsg, outStr);
//     runner.test("加法: 1+2", result == 3.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 减法
//     calc.calculate("5-3", result, errMsg, outStr);
//     runner.test("减法: 5-3", result == 2.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 乘法
//     calc.calculate("4*5", result, errMsg, outStr);
//     runner.test("乘法: 4*5", result == 20.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 除法
//     calc.calculate("10/2", result, errMsg, outStr);
//     runner.test("除法: 10/2", result == 5.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 混合运算
//     calc.calculate("2+3*4", result, errMsg, outStr);
//     runner.test("混合运算: 2+3*4", result == 14.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 带括号的运算
//     calc.calculate("(2+3)*4", result, errMsg, outStr);
//     runner.test("括号: (2+3)*4", result == 20.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testPowerOperator() {
//     Serial.println("\n--- 乘方运算测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 基本乘方
//     calc.calculate("2^3", result, errMsg, outStr);
//     runner.test("乘方: 2^3", result == 8.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 乘方优先级
//     calc.calculate("2^3+1", result, errMsg, outStr);
//     runner.test("乘方优先级: 2^3+1", result == 9.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 负指数
//     calc.calculate("2^-1", result, errMsg, outStr);
//     runner.test("负指数: 2^-1", std::abs(result - 0.5) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testParentheses() {
//     Serial.println("\n--- 括号测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 嵌套括号
//     calc.calculate("((2+3)*4)", result, errMsg, outStr);
//     runner.test("嵌套括号: ((2+3)*4)", result == 20.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 多层嵌套
//     calc.calculate("(((1+2)*(3+4)))", result, errMsg, outStr);
//     runner.test("多层嵌套: (((1+2)*(3+4)))", result == 21.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testNegativeNumbers() {
//     Serial.println("\n--- 负数测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 负数开头
//     calc.calculate("-5+3", result, errMsg, outStr);
//     runner.test("负数开头: -5+3", result == -2.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 负数在括号内
//     calc.calculate("(-5)+3", result, errMsg, outStr);
//     runner.test("括号内负数: (-5)+3", result == -2.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 减去负数
//     calc.calculate("5-(-3)", result, errMsg, outStr);
//     runner.test("减去负数: 5-(-3)", result == 8.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testDecimals() {
//     Serial.println("\n--- 小数测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 基本小数
//     calc.calculate("1.5+2.5", result, errMsg, outStr);
//     runner.test("小数加法: 1.5+2.5", result == 4.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 纯小数省略0 (.5 形式)
//     calc.calculate(".5+.5", result, errMsg, outStr);
//     runner.test("纯小数省略0: .5+.5", result == 1.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 复杂小数运算
//     calc.calculate("0.1*0.2", result, errMsg, outStr);
//     runner.test("小数乘法: 0.1*0.2", std::abs(result - 0.02) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testEqualsSign() {
//     Serial.println("\n--- 等号截取测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 带等号的表达式
//     calc.calculate("1+2=3", result, errMsg, outStr);
//     runner.test("等号截取: 1+2=3", result == 3.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 复杂表达式带等号
//     calc.calculate("(2+3)*4=x", result, errMsg, outStr);
//     runner.test("等号截取复杂: (2+3)*4=x", result == 20.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testMultiBaseOutput() {
//     Serial.println("\n--- 多进制输出测试 (|前缀) ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 十进制多进制输出
//     calc.calculate("|10", result, errMsg, outStr);
//     bool hasDec = outStr.indexOf("10") != -1;
//     bool hasHex = outStr.indexOf("0xA") != -1;
//     bool hasBin = outStr.indexOf("0b1010") != -1;
//     bool hasOct = outStr.indexOf("0o12") != -1;
//     runner.test("多进制输出: |10", hasDec && hasHex && hasBin && hasOct, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 十六进制输入 (0x)
//     calc.calculate("|0xFF", result, errMsg, outStr);
//     // 应该输出 255 的多种进制表示
//     runner.test("十六进制输入: |0xFF", outStr.indexOf("255") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 二进制输入 (0b)
//     calc.calculate("|0b1010", result, errMsg, outStr);
//     runner.test("二进制输入: |0b1010", result == 10.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 八进制输入 (0o)
//     calc.calculate("|0o10", result, errMsg, outStr);
//     runner.test("八进制输入: |0o10", result == 8.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testAbsFunction() {
//     Serial.println("\n--- abs() 绝对值函数测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     calc.calculate("abs(-5)", result, errMsg, outStr);
//     runner.test("abs(-5)", result == 5.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     calc.calculate("abs(5)", result, errMsg, outStr);
//     runner.test("abs(5)", result == 5.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     calc.calculate("abs(-3.14)", result, errMsg, outStr);
//     runner.test("abs(-3.14)", std::abs(result - 3.14) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testLogFunctions() {
//     Serial.println("\n--- 对数函数测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // log (自然对数 ln)
//     calc.calculate("log(2.718281828)", result, errMsg, outStr);
//     runner.test("log(e)≈1", std::abs(result - 1.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // log2
//     calc.calculate("log2(8)", result, errMsg, outStr);
//     runner.test("log2(8)=3", result == 3.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // log10
//     calc.calculate("log10(100)", result, errMsg, outStr);
//     runner.test("log10(100)=2", result == 2.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // log_ (任意底)
//     calc.calculate("log_(8,2)", result, errMsg, outStr);
//     runner.test("log_(8,2)=3", result == 3.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     calc.calculate("log_(100,10)", result, errMsg, outStr);
//     runner.test("log_(100,10)=2", result == 2.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testSqrtFunction() {
//     Serial.println("\n--- sqrt() 平方根函数测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     calc.calculate("sqrt(16)", result, errMsg, outStr);
//     runner.test("sqrt(16)=4", result == 4.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     calc.calculate("sqrt(2)", result, errMsg, outStr);
//     runner.test("sqrt(2)≈1.414", std::abs(result - 1.41421356) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testTrigFunctionsDegrees() {
//     Serial.println("\n--- 三角函数 (角度制) 测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // sin (角度制)
//     calc.calculate("sin(30)", result, errMsg, outStr);
//     runner.test("sin(30°)=0.5", std::abs(result - 0.5) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // cos (角度制)
//     calc.calculate("cos(60)", result, errMsg, outStr);
//     runner.test("cos(60°)=0.5", std::abs(result - 0.5) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // tan (角度制)
//     calc.calculate("tan(45)", result, errMsg, outStr);
//     runner.test("tan(45°)=1", std::abs(result - 1.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testTrigFunctionsRadians() {
//     Serial.println("\n--- 三角函数 (弧度制) 测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // sinr (弧度制)
//     calc.calculate("sinr(1.5707963267948966)", result, errMsg, outStr); // π/2
//     runner.test("sinr(π/2)≈1", std::abs(result - 1.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // cosr (弧度制)
//     calc.calculate("cosr(0)", result, errMsg, outStr);
//     runner.test("cosr(0)=1", std::abs(result - 1.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // tanr (弧度制)
//     calc.calculate("tanr(0.7853981633974483)", result, errMsg, outStr); // π/4
//     runner.test("tanr(π/4)≈1", std::abs(result - 1.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testInverseTrigFunctions() {
//     Serial.println("\n--- 反三角函数测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // asin (角度制输出)
//     calc.calculate("asin(0.5)", result, errMsg, outStr);
//     runner.test("asin(0.5)=30°", std::abs(result - 30.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // acos (角度制输出)
//     calc.calculate("acos(0.5)", result, errMsg, outStr);
//     runner.test("acos(0.5)=60°", std::abs(result - 60.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // atan (角度制输出)
//     calc.calculate("atan(1)", result, errMsg, outStr);
//     runner.test("atan(1)=45°", std::abs(result - 45.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // asinr (弧度制输出)
//     calc.calculate("asinr(1)", result, errMsg, outStr);
//     runner.test("asinr(1)=π/2", std::abs(result - M_PI/2) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // acosr (弧度制输出)
//     calc.calculate("acosr(0)", result, errMsg, outStr);
//     runner.test("acosr(0)=π/2", std::abs(result - M_PI/2) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // atanr (弧度制输出)
//     calc.calculate("atanr(1)", result, errMsg, outStr);
//     runner.test("atanr(1)=π/4", std::abs(result - M_PI/4) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testCustomFunction() {
//     Serial.println("\n--- 自定义函数注册测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 注册自定义函数
//     double (*myFunc)(const String& arg) = [](const String& arg) -> double { return arg.toDouble() * 2; };

//     bool registered = calc.registerFunction("myfunc", myFunc);
//     runner.test("注册自定义函数", registered, "");

//     if (registered) {
//         calc.calculate("myfunc(5)", result, errMsg, outStr);
//         runner.test("调用自定义函数: myfunc(5)=10", result == 10.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
//     }
// }

// void testBigNumber() {
//     Serial.println("\n--- 大数运算测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 大整数加法
//     calc.calculate("99999999999999999999+1", result, errMsg, outStr);
//     runner.test("大数加法: 99...99+1", outStr.indexOf("100000000000000000000") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 大整数乘法
//     calc.calculate("12345678901234567890*2", result, errMsg, outStr);
//     runner.test("大数乘法: 123...90*2", outStr.indexOf("24691357802469135780") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 大数幂运算
//     calc.calculate("2^100", result, errMsg, outStr);
//     // 2^100 = 1267650600228229401496703205376
//     runner.test("大数幂: 2^100", outStr.indexOf("1267650600228229401496703205376") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }



// void testInvalidCharacter() {
//     Serial.println("\n--- 无效字符与 UTF-8 测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // UTF-8 中文字符
//     calc.calculate("1+2+你好", result, errMsg, outStr);
//     runner.test("UTF-8 无效字符检测", errMsg.indexOf("无效字符") != -1 || errMsg.indexOf("错误") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 特殊符号
//     calc.calculate("1+2@3", result, errMsg, outStr);
//     runner.test("特殊符号检测", !calc.calculate("1+2@3", result, errMsg, outStr) || errMsg.indexOf("错误") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testErrorHandling() {
//     Serial.println("\n--- 错误处理测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 除零错误
//     calc.calculate("1/0", result, errMsg, outStr);
//     runner.test("除零错误检测", errMsg.indexOf("零") != -1 || errMsg.indexOf("错误") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 括号不匹配
//     calc.calculate("(1+2", result, errMsg, outStr);
//     runner.test("括号不匹配检测", errMsg.indexOf("括号") != -1 || errMsg.indexOf("错误") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 未知函数
//     calc.calculate("unknown(1)", result, errMsg, outStr);
//     runner.test("未知函数检测", errMsg.indexOf("未知函数") != -1 || errMsg.indexOf("错误") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 对数域错误
//     calc.calculate("log(-1)", result, errMsg, outStr);
//     runner.test("对数域错误检测", errMsg.indexOf("错误") != -1, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void testComplexExpressions() {
//     Serial.println("\n--- 复杂表达式测试 ---");
//     ArduinoCalc calc;
//     double result;
//     String outStr, errMsg;

//     // 混合函数和运算符
//     calc.calculate("sqrt(abs(-16))+log10(100)", result, errMsg, outStr);
//     runner.test("混合函数: sqrt(abs(-16))+log10(100)", std::abs(result - 6.0) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 嵌套函数
//     calc.calculate("sin(acos(0.5))", result, errMsg, outStr);
//     // acos(0.5) = 60°, sin(60°) = √3/2 ≈ 0.866
//     runner.test("嵌套函数: sin(acos(0.5))", std::abs(result - 0.8660254) < 0.0001, "got: errMsg=" + errMsg + ", outStr=" + outStr);

//     // 复杂括号和运算符
//     calc.calculate("((1+2)*(3+4))/(5-2)", result, errMsg, outStr);
//     runner.test("复杂括号: ((1+2)*(3+4))/(5-2)", result == 7.0, "got: errMsg=" + errMsg + ", outStr=" + outStr);
// }

// void setup() {
//     Serial.begin(115200);
//     Serial.println("========================================");
//     Serial.println("======= ArduinoCalc 计算器库全面测试 ======");
//     Serial.println("========================================");

//     testBasicArithmetic();
//     testPowerOperator();
//     testParentheses();
//     testNegativeNumbers();
//     testDecimals();
//     testEqualsSign();
//     testMultiBaseOutput();
//     testAbsFunction();
//     testLogFunctions();
//     testSqrtFunction();
//     testTrigFunctionsDegrees();
//     testTrigFunctionsRadians();
//     testInverseTrigFunctions();
//     testCustomFunction();
//     testBigNumber();
//     testInvalidCharacter();
//     testErrorHandling();
//     testComplexExpressions();

//     runner.printSummary();
// }

// void loop() {
//     vTaskDelete(nullptr);
// }

