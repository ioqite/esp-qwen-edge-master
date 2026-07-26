#include "ArduinoCalc.hpp"

// --- 辅助函数：获取运算符优先级 ---
int __getPrecedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    if (op == '#' || op == '@') return 4; // '#'代表一元负号, '@'代表一元正号，优先级最高
    return 0; 
}

// --- 辅助函数：执行一次二元运算 ---
bool __applyBinaryOp(double a, double b, char op, double& result, String& errMsg) {
    switch(op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) {
                errMsg += "#bf160d 错误: 除数不能为零 #\n";
                return false;
            }
            result = a / b; 
            break;
        case '^': 
            result = std::pow(a, b); 
            break;
        default:
            errMsg += "#bf160d 错误: 未知二元运算符 #\n";
            return false;
    }
    return true;
}

// --- 辅助函数：执行一次一元运算 ---
bool __applyUnaryOp(double a, char op, double& result, String& errMsg) {
    if (op == '#') { result = -a; return true; }
    if (op == '@') { result = a; return true; }
    errMsg += "#bf160d 错误: 未知一元运算符 #\n";
    return false;
}

// 计算表达式字符串的值, 自动检测 '=' 号, 并截取表达式
bool calculate(const String& input, double& result, String& errMsg) {
    size_t pos = input.indexOf('=');
    String expression = (pos != -1) ? input.substring(0, pos) : input;
        
    std::stack<double> values;
    std::stack<char> operators;
    
    // 1. 预处理：移除所有空格
    String expr;
    for (char c : expression) {
        if (!isspace(c)) {
            expr += c;
        }
    }

    int i = 0;
    while (i < expr.length()) {
        char currentChar = expr[i];

        // 2. 处理数字和小数点
        if (isdigit(currentChar) || currentChar == '.') {
            String numStr;
            while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) {
                numStr += expr[i];
                i++;
            }
            try {
                values.push(numStr.toDouble());
            } catch (const std::exception& e) {
                errMsg += "#bf160d 错误: 无效的数字格式 '" + numStr + "' #\n";
                return false;
            }
            continue; 
        }
        
        // 3. 处理左括号
        else if (currentChar == '(') {
            operators.push(currentChar);
        }
        
        // 4. 处理右括号
        else if (currentChar == ')') {
            while (!operators.empty() && operators.top() != '(') {
                char op = operators.top(); operators.pop();
                
                double res;
                if (op == '#' || op == '@') { // 一元运算符
                    if (values.empty()) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
                    double val = values.top(); values.pop();
                    if (!__applyUnaryOp(val, op, res, errMsg)) return false;
                } else { // 二元运算符
                    if (values.size() < 2) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
                    double val2 = values.top(); values.pop();
                    double val1 = values.top(); values.pop();
                    if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
                }
                values.push(res);
            }
            if (operators.empty()) {
                 errMsg += "#bf160d 错误: 括号不匹配 #\n";
                 return false;
            }
            operators.pop(); // 弹出左括号 '('
        }
        
        // 5. 处理运算符 (+, -, *, /, ^)
        else if (currentChar == '+' || currentChar == '-' || currentChar == '*' || currentChar == '/' || currentChar == '^') {
            // 处理一元正负号：若在首位，或在前导运算符/左括号之后
            if ((currentChar == '-' || currentChar == '+') && 
                (i == 0 || expr[i-1] == '(' || __getPrecedence(expr[i-1]) > 0)) {
                // 将一元负号记为 '#'，一元正号记为 '@'
                operators.push(currentChar == '-' ? '#' : '@');
            }
            else {
                // 二元运算符优先级与结合性处理
                while (!operators.empty() && operators.top() != '(') {
                    char topOp = operators.top();
                    if (currentChar == '^') {
                        if (__getPrecedence(topOp) <= __getPrecedence(currentChar)) break; // 右结合：只有栈顶严格大于才弹出
                    } else {
                        if (__getPrecedence(topOp) < __getPrecedence(currentChar)) break; // 左结合：栈顶大于等于则弹出
                    }
                    
                    // 执行栈顶运算
                    char op = topOp; operators.pop();
                    double res;
                    if (op == '#' || op == '@') {
                        if (values.empty()) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
                        double val = values.top(); values.pop();
                        if (!__applyUnaryOp(val, op, res, errMsg)) return false;
                    } else {
                        if (values.size() < 2) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
                        double val2 = values.top(); values.pop();
                        double val1 = values.top(); values.pop();
                        if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
                    }
                    values.push(res);
                }
                operators.push(currentChar);
            }
        }
        // 如果遇到无法识别的字符
        else {
             // 注意：char 类型不能直接用 + 和 string 拼接，需转为 string
             errMsg += "#bf160d 错误: 无效的字符 '" + String(currentChar) + "' #\n";
             return false;
        }
        i++;
    }

    // 6. 处理完所有字符后，计算栈中剩余的运算符
    while (!operators.empty()) {
        if (operators.top() == '(') {
             errMsg += "#bf160d 错误: 括号不匹配 #\n";
             return false;
        }
        char op = operators.top(); operators.pop();
        double res;
        if (op == '#' || op == '@') {
            if (values.empty()) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
            double val = values.top(); values.pop();
            if (!__applyUnaryOp(val, op, res, errMsg)) return false;
        } else {
            if (values.size() < 2) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
            double val2 = values.top(); values.pop();
            double val1 = values.top(); values.pop();
            if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
        }
        values.push(res);
    }

    if (values.size() != 1) {
         errMsg += "#bf160d 错误: 表达式格式不正确 #\n";
         return false;
    }
    
    result = values.top();
    return true;
}

// #ifndef ARDUINO_CALC_HPP
// #define ARDUINO_CALC_HPP

// #include <Arduino.h>
// #include <stack>
// #include <cmath>

// #ifndef M_PI
// #define M_PI 3.14159265358979323846
// #endif

// #define DEG_TO_RAD (M_PI / 180.0)
// #define RAD_TO_DEG (180.0 / M_PI)

// #define MAX_CUSTOM_FUNCTIONS 10

// typedef double (*CalcCallback)(const String&);

// // --- 值类型封装：支持双精度浮点与超大整数 ---
// struct CalcValue {
//     enum Type { DOUBLE, BIGINT } type;
//     double dbl_val;
//     String big_val; // 正序存储，如 "-12345678901234567890"

//     CalcValue() : type(DOUBLE), dbl_val(0) {}
//     CalcValue(double d) : type(DOUBLE), dbl_val(d) {}
//     CalcValue(const String& s, Type t) : type(t), big_val(s) {}
// };

// class ArduinoCalc {
// private:
//     struct CustomFunc {
//         String name;
//         CalcCallback callback;
//     };
//     CustomFunc customFunctions[MAX_CUSTOM_FUNCTIONS];
//     int customFuncCount;

//     // --- 大数运算核心 (极简版，专为ESP32优化) ---
//     String trimZeros(String s) const;
//     int cmpAbs(String a, String b) const;
//     String addAbs(String a, String b) const;
//     String subAbs(String a, String b) const; // a >= b
//     String bigAdd(String a, String b) const;
//     String bigSub(String a, String b) const;
//     String bigMul(String a, String b) const;
//     String bigDiv(String a, String b) const;
//     String bigPow(String base, String exp) const;
//     String bigToBase(String num, int base) const; // 复用除法做进制转换

//     // --- 内置计算逻辑 ---
//     bool isCustomFunc(const String& name) const;
//     CalcCallback getCustomCallback(const String& name) const;
//     bool isBuiltinFunc(const String& name) const;
//     bool executeBuiltinFunc(const String& name, double arg, double& result, String& errMsg);
    
//     bool applyBinaryOp(const CalcValue& a, const CalcValue& b, const String& op, CalcValue& result, String& errMsg);
//     bool applyUnaryOp(const CalcValue& a, const String& op, CalcValue& result, String& errMsg);

// public:
//     ArduinoCalc() : customFuncCount(0) {}
//     bool registerFunction(const String& name, CalcCallback callback);
//     bool calculate(const String& input, double& dbl_result, String& errMsg, String& outStr);
// };

// // ==========================================
// // --- 大数运算实现 ---
// // ==========================================

// String ArduinoCalc::trimZeros(String s) const {
//     bool neg = s.startsWith("-");
//     if (neg) s = s.substring(1);
//     int start = 0;
//     while (start < s.length() - 1 && s[start] == '0') start++;
//     s = s.substring(start);
//     if (neg && s != "0") s = "-" + s;
//     return s;
// }

// int ArduinoCalc::cmpAbs(String a, String b) const {
//     if (a.startsWith("-")) a = a.substring(1);
//     if (b.startsWith("-")) b = b.substring(1);
//     a = trimZeros(a); b = trimZeros(b);
//     if (a.length() > b.length()) return 1;
//     if (a.length() < b.length()) return -1;
//     if (a > b) return 1;
//     if (a < b) return -1;
//     return 0;
// }

// String ArduinoCalc::addAbs(String a, String b) const {
//     String res = ""; int carry = 0;
//     int i = a.length() - 1, j = b.length() - 1;
//     while (i >= 0 || j >= 0 || carry) {
//         int sum = carry;
//         if (i >= 0) sum += a[i--] - '0';
//         if (j >= 0) sum += b[j--] - '0';
//         res = String(sum % 10) + res;
//         carry = sum / 10;
//     }
//     return trimZeros(res);
// }

// String ArduinoCalc::subAbs(String a, String b) const { // a >= b
//     String res = ""; int borrow = 0;
//     int i = a.length() - 1, j = b.length() - 1;
//     while (i >= 0) {
//         int diff = (a[i--] - '0') - borrow;
//         if (j >= 0) diff -= (b[j--] - '0');
//         if (diff < 0) { diff += 10; borrow = 1; } else borrow = 0;
//         res = String(diff) + res;
//     }
//     return trimZeros(res);
// }

// String ArduinoCalc::bigAdd(String a, String b) const {
//     bool negA = a.startsWith("-"); if(negA) a = a.substring(1);
//     bool negB = b.startsWith("-"); if(negB) b = b.substring(1);
//     a = trimZeros(a); b = trimZeros(b);
//     if(a == "0") negA = false; if(b == "0") negB = false;

//     if (!negA && !negB) return addAbs(a, b);
//     if (negA && negB) return "-" + addAbs(a, b);
//     int cmp = cmpAbs(a, b);
//     if (cmp == 0) return "0";
//     if (!negA && negB) return (cmp > 0) ? subAbs(a, b) : "-" + subAbs(b, a);
//     return (cmp > 0) ? "-" + subAbs(a, b) : subAbs(b, a);
// }

// String ArduinoCalc::bigSub(String a, String b) const {
//     if (b.startsWith("-")) b = b.substring(1); else b = "-" + b;
//     return bigAdd(a, b);
// }

// String ArduinoCalc::bigMul(String a, String b) const {
//     bool negA = a.startsWith("-"); if(negA) a = a.substring(1);
//     bool negB = b.startsWith("-"); if(negB) b = b.substring(1);
//     a = trimZeros(a); b = trimZeros(b);
//     bool negRes = negA ^ negB;
//     if (a == "0" || b == "0") return "0";

//     String res = "0";
//     for (int i = a.length() - 1; i >= 0; i--) {
//         String temp = "";
//         for (int k = 0; k < a.length() - 1 - i; k++) temp += "0";
//         int carry = 0;
//         for (int j = b.length() - 1; j >= 0; j--) {
//             int prod = (a[i] - '0') * (b[j] - '0') + carry;
//             temp = String(prod % 10) + temp;
//             carry = prod / 10;
//         }
//         if (carry) temp = String(carry) + temp;
//         res = bigAdd(res, temp);
//     }
//     if (negRes && res != "0") res = "-" + res;
//     return res;
// }

// String ArduinoCalc::bigDiv(String a, String b) const {
//     bool negA = a.startsWith("-"); if(negA) a = a.substring(1);
//     bool negB = b.startsWith("-"); if(negB) b = b.substring(1);
//     a = trimZeros(a); b = trimZeros(b);
//     bool negRes = negA ^ negB;
//     if (b == "0") return "ERROR_DIV_ZERO";
//     if (cmpAbs(a, b) < 0) return "0";

//     String res = ""; String curr = "0";
//     for (int i = 0; i < a.length(); i++) {
//         curr = curr + a[i]; curr = trimZeros(curr);
//         int q = 0;
//         while (cmpAbs(curr, b) >= 0) {
//             curr = subAbs(curr, b); q++;
//         }
//         res += String(q);
//     }
//     res = trimZeros(res);
//     if (negRes && res != "0") res = "-" + res;
//     return res;
// }

// String ArduinoCalc::bigPow(String base, String exp) const {
//     if (exp.startsWith("-")) return "ERROR_NEG_EXP";
//     long long e = exp.toDouble(); // 指数通常不大
//     if (e > 1000) return "ERROR_HUGE_EXP";
//     String res = "1";
//     for (long long i = 0; i < e; i++) {
//         res = bigMul(res, base);
//         if (res.length() > 5000) return "ERROR_HUGE_RES"; // 防内存爆炸
//     }
//     return res;
// }

// // --- 复用大数除法实现完美进制转换 ---
// String ArduinoCalc::bigToBase(String num, int base) const {
//     if (num.startsWith("-")) return "-" + bigToBase(num.substring(1), base);
//     if (num == "0") return "0";
//     String res = ""; String b = String(base);
//     while (num != "0") {
//         String div = bigDiv(num, b);
//         String mul = bigMul(div, b);
//         String rem = bigSub(num, mul);
//         int r = rem.toInt();
//         char c = (r < 10) ? ('0' + r) : ('A' + r - 10);
//         res = String(c) + res;
//         num = div;
//     }
//     return res;
// }

// // ==========================================
// // --- 类核心逻辑实现 ---
// // ==========================================

// bool ArduinoCalc::isCustomFunc(const String& name) const {
//     for (int i = 0; i < customFuncCount; i++) if (customFunctions[i].name == name) return true;
//     return false;
// }
// CalcCallback ArduinoCalc::getCustomCallback(const String& name) const {
//     for (int i = 0; i < customFuncCount; i++) if (customFunctions[i].name == name) return customFunctions[i].callback;
//     return nullptr;
// }
// bool ArduinoCalc::isBuiltinFunc(const String& name) const {
//     return name == "sin" || name == "cos" || name == "tan" || name == "asin" || name == "acos" || name == "atan" || 
//            name == "sinr" || name == "cosr" || name == "tanr" || name == "asinr" || name == "acosr" || name == "atanr" || name == "sqrt";
// }
// bool ArduinoCalc::executeBuiltinFunc(const String& name, double arg, double& result, String& errMsg) {
//     if (name == "sin") { result = std::sin(arg * DEG_TO_RAD); return true; }
//     if (name == "cos") { result = std::cos(arg * DEG_TO_RAD); return true; }
//     if (name == "tan") { result = std::tan(arg * DEG_TO_RAD); return true; }
//     if (name == "asin") { if (arg < -1 || arg > 1) { errMsg="Error: asin domain"; return false; } result = std::asin(arg) * RAD_TO_DEG; return true; }
//     if (name == "acos") { if (arg < -1 || arg > 1) { errMsg="Error: acos domain"; return false; } result = std::acos(arg) * RAD_TO_DEG; return true; }
//     if (name == "atan") { result = std::atan(arg) * RAD_TO_DEG; return true; }
//     if (name == "sinr") { result = std::sin(arg); return true; } if (name == "cosr") { result = std::cos(arg); return true; }
//     if (name == "tanr") { result = std::tan(arg); return true; }
//     if (name == "asinr") { if (arg < -1 || arg > 1) { errMsg="Error: asinr domain"; return false; } result = std::asin(arg); return true; }
//     if (name == "acosr") { if (arg < -1 || arg > 1) { errMsg="Error: acosr domain"; return false; } result = std::acos(arg); return true; }
//     if (name == "atanr") { result = std::atan(arg); return true; }
//     if (name == "sqrt") { if (arg < 0) { errMsg="Error: sqrt domain"; return false; } result = std::sqrt(arg); return true; }
//     errMsg = "Error: Unknown func " + name; return false;
// }

// bool ArduinoCalc::applyBinaryOp(const CalcValue& cv1, const CalcValue& cv2, const String& op, CalcValue& resCV, String& errMsg) {
//     // 只要有一个是大数，就强制拉入大数运算 (保证绝对精确，舍弃小数部分)
//     if (cv1.type == CalcValue::BIGINT || cv2.type == CalcValue::BIGINT) {
//         String a = (cv1.type == CalcValue::DOUBLE) ? String((long long)cv1.dbl_val) : cv1.big_val;
//         String b = (cv2.type == CalcValue::DOUBLE) ? String((long long)cv2.dbl_val) : cv2.big_val;
//         String resStr;
//         if (op == "+") resStr = bigAdd(a, b);
//         else if (op == "-") resStr = bigSub(a, b);
//         else if (op == "*") resStr = bigMul(a, b);
//         else if (op == "/") {
//             resStr = bigDiv(a, b);
//             if (resStr == "ERROR_DIV_ZERO") { errMsg = "错误: 除数不能为零\n"; return false; }
//         }
//         else if (op == "^") {
//             resStr = bigPow(a, b);
//             if (resStr.startsWith("ERROR")) { errMsg = "错误: " + resStr + "\n"; return false; }
//         }
//         else { errMsg = "错误: 大数不支持运算符 " + op + "\n"; return false; }
//         resCV = CalcValue(resStr, CalcValue::BIGINT);
//     } else {
//         double a = cv1.dbl_val, b = cv2.dbl_val, r;
//         if (op == "+") r = a + b;
//         else if (op == "-") r = a - b;
//         else if (op == "*") r = a * b;
//         else if (op == "/") { if(b==0){ errMsg="错误: 除数不能为零\n"; return false;} r = a / b; }
//         else if (op == "^") r = std::pow(a, b);
//         else { errMsg = "错误: 未知运算符 " + op + "\n"; return false; }
//         resCV = CalcValue(r);
//     }
//     return true;
// }

// bool ArduinoCalc::applyUnaryOp(const CalcValue& cv, const String& op, CalcValue& resCV, String& errMsg) {
//     if (op == "@") { resCV = cv; return true; }
//     if (op == "#") {
//         if (cv.type == CalcValue::BIGINT) {
//             String s = cv.big_val;
//             if (s.startsWith("-")) s = s.substring(1); else s = "-" + s;
//             resCV = CalcValue(s, CalcValue::BIGINT);
//         } else {
//             resCV = CalcValue(-cv.dbl_val);
//         }
//         return true;
//     }
//     errMsg = "错误: 未知一元运算符\n"; return false;
// }

// bool ArduinoCalc::registerFunction(const String& name, CalcCallback callback) {
//     for (int i = 0; i < customFuncCount; i++) {
//         if (customFunctions[i].name == name) { customFunctions[i].callback = callback; return true; }
//     }
//     if (customFuncCount >= MAX_CUSTOM_FUNCTIONS) return false;
//     customFunctions[customFuncCount] = {name, callback};
//     customFuncCount++;
//     return true;
// }

// bool ArduinoCalc::calculate(const String& input, double& dbl_result, String& errMsg, String& outStr) {
//     String expression = input;
//     bool isMultiBase = false;
//     if (expression.length() > 0 && expression.charAt(0) == '|') {
//         isMultiBase = true; expression = expression.substring(1);
//     }
//     size_t pos = expression.indexOf('=');
//     if (pos != -1) expression = expression.substring(0, pos);
        
//     std::stack<CalcValue> values;
//     std::stack<String> operators;
    
//     String expr;
//     for (char c : expression) if (!isspace(c)) expr += c;

//     int i = 0;
//     while (i < expr.length()) {
//         if (isdigit(expr[i]) || expr[i] == '.') {
//             String numStr;
//             while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) { numStr += expr[i]; i++; }
//             // 核心判断：如果有小数点，走 DOUBLE；如果纯整数，走 BIGINT 绝不截断！
//             if (numStr.indexOf('.') != -1) {
//                 values.push(CalcValue(numStr.toDouble()));
//             } else {
//                 values.push(CalcValue(trimZeros(numStr), CalcValue::BIGINT));
//             }
//             continue; 
//         }
//         else if (isalpha(expr[i])) {
//             String funcName;
//             while (i < expr.length() && isalpha(expr[i])) { funcName += expr[i]; i++; }
//             funcName.toLowerCase();
//             while (i < expr.length() && isspace(expr[i])) i++;
//             if (i >= expr.length() || expr[i] != '(') { errMsg="错误: 函数缺少括号\n"; return false; }
//             i++; // skip '('
//             int parenCount = 1; String argStr = "";
//             while (i < expr.length() && parenCount > 0) {
//                 if (expr[i] == '(') parenCount++; else if (expr[i] == ')') { parenCount--; if (parenCount == 0) break; }
//                 argStr += expr[i]; i++;
//             }
//             if (parenCount != 0) { errMsg="错误: 括号不匹配\n"; return false; }
//             i++; // skip ')'

//             CalcValue resCV;
//             if (isCustomFunc(funcName)) {
//                 double res = getCustomCallback(funcName)(argStr);
//                 if (isnan(res)) { errMsg="错误: 自定义函数失败\n"; return false; }
//                 resCV = CalcValue(res); // 自定义函数目前只返回浮点
//             } else if (isBuiltinFunc(funcName)) {
//                 double innerRes; String innerErr, innerOut;
//                 if (!calculate(argStr, innerRes, innerErr, innerOut)) { errMsg = innerErr; return false; }
//                 double funcRes;
//                 if (!executeBuiltinFunc(funcName, innerRes, funcRes, errMsg)) return false;
//                 resCV = CalcValue(funcRes); // 内置函数只返回浮点
//             } else { errMsg="错误: 未知函数 " + funcName + "\n"; return false; }
//             values.push(resCV);
//             continue;
//         }
//         else if (expr[i] == '(') { operators.push("("); }
//         else if (expr[i] == ')') {
//             while (!operators.empty() && operators.top() != "(") {
//                 String op = operators.top(); operators.pop();
//                 if (values.size() < (op=="#"||op=="@" ? 1 : 2)) { errMsg="格式错误\n"; return false; }
//                 CalcValue resCV;
//                 if (op == "#" || op == "@") {
//                     CalcValue a = values.top(); values.pop();
//                     if (!applyUnaryOp(a, op, resCV, errMsg)) return false;
//                 } else {
//                     CalcValue b = values.top(); values.pop(); CalcValue a = values.top(); values.pop();
//                     if (!applyBinaryOp(a, b, op, resCV, errMsg)) return false;
//                 }
//                 values.push(resCV);
//             }
//             if (operators.empty()) { errMsg="括号不匹配\n"; return false; }
//             operators.pop();
//         }
//         else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/' || expr[i] == '^') {
//             if ((expr[i] == '-' || expr[i] == '+') && (i == 0 || expr[i-1] == '(' || (expr[i-1]!=')' && !isdigit(expr[i-1]) && expr[i-1]!='.'))) {
//                 operators.push(expr[i] == '-' ? "#" : "@");
//             } else {
//                 String curOp = String(1, expr[i]);
//                 while (!operators.empty() && operators.top() != "(") {
//                     String topOp = operators.top();
//                     int tp = (topOp=="#"||topOp=="@")?4:(topOp=="^"?3:(topOp=="*"||topOp=="/"?2:1));
//                     int cp = (curOp=="^")?3:(curOp=="*"||curOp=="/"?2:1);
//                     if (curOp=="^") { if(tp<=cp) break; } else { if(tp<cp) break; }
//                     operators.pop();
//                     CalcValue resCV;
//                     if (topOp=="#"||topOp=="@") { if(values.empty()){errMsg="格式错误\n";return false;} CalcValue a=values.top();values.pop(); if(!applyUnaryOp(a,topOp,resCV,errMsg))return false; }
//                     else { if(values.size()<2){errMsg="格式错误\n";return false;} CalcValue b=values.top();values.pop(); CalcValue a=values.top();values.pop(); if(!applyBinaryOp(a,b,topOp,resCV,errMsg))return false; }
//                     values.push(resCV);
//                 }
//                 operators.push(curOp);
//             }
//         }
//         else { errMsg="错误: 无效字符 '"+String(1, expr[i])+"'\n"; return false; }
//         i++;
//     }

//     while (!operators.empty()) {
//         if (operators.top() == "(") { errMsg="括号不匹配\n"; return false; }
//         String op = operators.top(); operators.pop();
//         CalcValue resCV;
//         if (op=="#"||op=="@") { if(values.empty()){errMsg="格式错误\n";return false;} CalcValue a=values.top();values.pop(); if(!applyUnaryOp(a,op,resCV,errMsg))return false; }
//         else { if(values.size()<2){errMsg="格式错误\n";return false;} CalcValue b=values.top();values.pop(); CalcValue a=values.top();values.pop(); if(!applyBinaryOp(a,b,op,resCV,errMsg))return false; }
//         values.push(resCV);
//     }

//     if (values.size() != 1) { errMsg="格式错误\n"; return false; }
//     CalcValue finalCV = values.top();
//     dbl_result = (finalCV.type == CalcValue::BIGINT) ? finalCV.big_val.toDouble() : finalCV.dbl_val;

//     if (isMultiBase) {
//         String numStr = (finalCV.type == CalcValue::BIGINT) ? finalCV.big_val : String((long long)lround(finalCV.dbl_val));
//         bool neg = numStr.startsWith("-");
//         if (neg) numStr = numStr.substring(1);
        
//         String decStr = numStr;
//         String hexStr = bigToBase(numStr, 16);
//         String binStr = bigToBase(numStr, 2);
//         String octStr = bigToBase(numStr, 8);
        
//         String sign = neg ? "-" : "";
//         outStr = sign + decStr + " " + sign + "0x" + hexStr + " " + sign + "0b" + binStr + " " + sign + "0o" + octStr;
//     } else {
//         outStr = (finalCV.type == CalcValue::BIGINT) ? finalCV.big_val : String(finalCV.dbl_val);
//     }
//     return true;
// }

// #endif // ARDUINO_CALC_HPP
