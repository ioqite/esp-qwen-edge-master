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
                errMsg += "错误: 除数不能为零\n";
                return false;
            }
            result = a / b; 
            break;
        case '^': 
            result = std::pow(a, b); 
            break;
        default:
            errMsg += "错误: 未知二元运算符\n";
            return false;
    }
    return true;
}

// --- 辅助函数：执行一次一元运算 ---
bool __applyUnaryOp(double a, char op, double& result, String& errMsg) {
    if (op == '#') { result = -a; return true; }
    if (op == '@') { result = a; return true; }
    errMsg += "错误: 未知一元运算符\n";
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
                errMsg += "错误: 无效的数字格式 '" + numStr + "'\n";
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
                    if (values.empty()) { errMsg += "错误: 表达式格式不正确\n"; return false; }
                    double val = values.top(); values.pop();
                    if (!__applyUnaryOp(val, op, res, errMsg)) return false;
                } else { // 二元运算符
                    if (values.size() < 2) { errMsg += "错误: 表达式格式不正确\n"; return false; }
                    double val2 = values.top(); values.pop();
                    double val1 = values.top(); values.pop();
                    if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
                }
                values.push(res);
            }
            if (operators.empty()) {
                 errMsg += "错误: 括号不匹配\n";
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
                        if (values.empty()) { errMsg += "错误: 表达式格式不正确\n"; return false; }
                        double val = values.top(); values.pop();
                        if (!__applyUnaryOp(val, op, res, errMsg)) return false;
                    } else {
                        if (values.size() < 2) { errMsg += "错误: 表达式格式不正确\n"; return false; }
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
             errMsg += "错误: 无效的字符 '" + String(1, currentChar) + "'\n";
             return false;
        }
        i++;
    }

    // 6. 处理完所有字符后，计算栈中剩余的运算符
    while (!operators.empty()) {
        if (operators.top() == '(') {
             errMsg += "错误: 括号不匹配\n";
             return false;
        }
        char op = operators.top(); operators.pop();
        double res;
        if (op == '#' || op == '@') {
            if (values.empty()) { errMsg += "错误: 表达式格式不正确\n"; return false; }
            double val = values.top(); values.pop();
            if (!__applyUnaryOp(val, op, res, errMsg)) return false;
        } else {
            if (values.size() < 2) { errMsg += "错误: 表达式格式不正确\n"; return false; }
            double val2 = values.top(); values.pop();
            double val1 = values.top(); values.pop();
            if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
        }
        values.push(res);
    }

    if (values.size() != 1) {
         errMsg += "错误: 表达式格式不正确\n";
         return false;
    }
    
    result = values.top();
    return true;
}

