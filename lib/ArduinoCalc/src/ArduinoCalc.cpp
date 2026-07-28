#include "ArduinoCalc.hpp"


// ==========================================
// 变体类型：无缝容纳 double 或 mbedtls_mpi
// ==========================================

CalcValue::CalcValue() : type(VT_DOUBLE), d_val(0.0) { mbedtls_mpi_init(&mpi_val); }
CalcValue::CalcValue(const CalcValue& other) : type(other.type), d_val(other.d_val) {
    mbedtls_mpi_init(&mpi_val);
    mbedtls_mpi_copy(&mpi_val, &other.mpi_val);
}
CalcValue& CalcValue::operator=(const CalcValue& other) {
    if (this != &other) {
        // 防御性：先释放自身 mpi 缓冲，再重新 init+copy
        // 避免 ESP32 真机上 mbedtls_mpi_copy 在 dst 非空时的潜在内存问题
        mbedtls_mpi_free(&mpi_val);
        mbedtls_mpi_init(&mpi_val);
        type = other.type;
        d_val = other.d_val;
        mbedtls_mpi_copy(&mpi_val, &other.mpi_val);
    }
    return *this;
}
CalcValue::~CalcValue() { mbedtls_mpi_free(&mpi_val); }

CalcValue CalcValue::fromDouble(double d) { CalcValue v; v.type = VT_DOUBLE; v.d_val = d; return v; }
CalcValue CalcValue::fromMpiStr(const String& s) {
    CalcValue v; v.type = VT_MPI; v.d_val = 0;
    int ret = mbedtls_mpi_read_string(&v.mpi_val, 10, s.c_str());
    if (ret != 0) {
        // 解析失败，降级为 double
        v.type = VT_DOUBLE;
        v.d_val = s.toDouble();
    }
    return v;
}

bool CalcValue::promoteToMpi() {
    if (type == VT_MPI) return true;
    if (d_val != floor(d_val) || isinf(d_val) || isnan(d_val)) return false;
    type = VT_MPI;
    char buf[32]; snprintf(buf, sizeof(buf), "%.0f", d_val);
    int ret = mbedtls_mpi_read_string(&mpi_val, 10, buf);
    if (ret != 0) {
        // 解析失败，回退为 double
        type = VT_DOUBLE;
        return false;
    }
    return true;
}

// 大数强制降级为 double (用于对数等浮点专属运算)
double CalcValue::toDouble() const {
    if (type == VT_DOUBLE) return d_val;
    String s = mpiToString(&mpi_val, 10);
    if (s.length() == 0) return 0.0; // mpiToString 失败时返回 0
    return s.toDouble(); // 超出范围返回 Infinity，属于预期行为
}

// 优化的动态内存 MPI 转字符串 (支持 2, 8, 10, 16 进制无限长度)
String CalcValue::mpiToString(const mbedtls_mpi* X, int radix) {
    if (mbedtls_mpi_cmp_int(X, 0) == 0) return "0";
    size_t bits = mbedtls_mpi_bitlen(X);
    // 真机 mbedtls 的 mbedtls_mpi_write_string 实际需要更大缓冲
    // 保守估算：每位最坏情况占用 1 字节（10进制：bits*0.30103，但留余量）
    // +16 是 sign + null + mbedtls 内部余量
    size_t buf_size;
    if (radix == 2)       buf_size = bits + 16;
    else if (radix == 8)  buf_size = bits / 3 + 16;
    else if (radix == 16) buf_size = bits / 4 + 16;
    else                  buf_size = bits * 4 / 10 + 32; // 10进制，多留余量

    char* buf = (char*)malloc(buf_size);
    if (!buf) return "[OOM]";
    buf[0] = '\0';  // 防止 mbedtls_mpi_write_string 失败时读到未初始化数据
    size_t olen = 0;
    int ret = mbedtls_mpi_write_string(X, radix, buf, buf_size, &olen);
    if (ret != 0 || olen == 0) {
        // 失败时尝试更大缓冲重试一次
        free(buf);
        buf_size = buf_size * 2 + 32;
        buf = (char*)malloc(buf_size);
        if (!buf) return "[OOM]";
        buf[0] = '\0';
        olen = 0;
        ret = mbedtls_mpi_write_string(X, radix, buf, buf_size, &olen);
        if (ret != 0 || olen == 0) { free(buf); return "[ERR]"; }
    }
    String res = String(buf);
    free(buf);
    return res;
}


// ==========================================
// 快速幂算法补充
// ==========================================
static int mbedtls_mpi_pow(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi *E) {
    if (mbedtls_mpi_cmp_int(E, 0) == 0) {
        // X = 1，先清空 X 防止真机上写入已非空 mpi 损坏
        mbedtls_mpi_free(X); mbedtls_mpi_init(X);
        mbedtls_mpi_lset(X, 1); return 0;
    }
    if (mbedtls_mpi_cmp_int(E, 1) == 0) {
        mbedtls_mpi_free(X); mbedtls_mpi_init(X);
        mbedtls_mpi_copy(X, A); return 0;
    }
    mbedtls_mpi res, temp;
    mbedtls_mpi_init(&res); mbedtls_mpi_init(&temp);
    mbedtls_mpi_lset(&res, 1);
    size_t bits = mbedtls_mpi_bitlen(E);
    int err = 0;
    for (size_t i = bits; i > 0; i--) {
        // temp = res * res
        if (mbedtls_mpi_mul_mpi(&temp, &res, &res) != 0) { err = -1; break; }
        // res = temp (用 free+init+copy 替代直接 copy，避免真机潜在问题)
        mbedtls_mpi_free(&res); mbedtls_mpi_init(&res);
        mbedtls_mpi_copy(&res, &temp);
        if (mbedtls_mpi_get_bit(E, i - 1)) {
            // temp = res * A
            if (mbedtls_mpi_mul_mpi(&temp, &res, A) != 0) { err = -1; break; }
            mbedtls_mpi_free(&res); mbedtls_mpi_init(&res);
            mbedtls_mpi_copy(&res, &temp);
        }
    }
    if (err == 0) {
        mbedtls_mpi_free(X); mbedtls_mpi_init(X);
        mbedtls_mpi_copy(X, &res);
    }
    mbedtls_mpi_free(&res); mbedtls_mpi_free(&temp);
    return err;
}

// ==========================================
// UTF-8 辅助截取
// ==========================================
static int getUtf8CharLen(char c) {
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}





// ==========================================
// 类的实现
// ==========================================
bool ArduinoCalc::isCustomFunc(const String& name) const {
    for (int i = 0; i < customFuncCount; i++) if (customFunctions[i].name == name) return true;
    return false;
}
CalcCallback ArduinoCalc::getCustomCallback(const String& name) const {
    for (int i = 0; i < customFuncCount; i++) if (customFunctions[i].name == name) return customFunctions[i].callback;
    return nullptr;
}

bool ArduinoCalc::evaluate2Args(const String& argStr, double& v1, double& v2, String& errMsg) {
    int parenCount = 0, commaPos = -1;
    for (int k = 0; k < argStr.length(); k++) {
        if (argStr[k] == '(') parenCount++;
        else if (argStr[k] == ')') parenCount--;
        else if (argStr[k] == ',' && parenCount == 0) { commaPos = k; break; }
    }
    if (commaPos == -1) { errMsg += "错误: 缺少逗号分隔参数\n"; return false; }
    
    String arg1 = argStr.substring(0, commaPos);
    String arg2 = argStr.substring(commaPos + 1);
    
    CalcValue res1, res2; 
    String e1, e2;
    if (!evaluateExpression(arg1, res1, e1)) { errMsg += e1; return false; }
    if (!evaluateExpression(arg2, res2, e2)) { errMsg += e2; return false; }
    
    v1 = res1.toDouble(); v2 = res2.toDouble();
    return true;
}

bool ArduinoCalc::applyBinaryOp(const CalcValue& aIn, const CalcValue& bIn, const String& op, CalcValue& result, String& errMsg) {
    // 用引用避免无谓拷贝（ESP32 上 mbedtls_mpi 反复 init/copy/free 易引发堆问题）
    bool needMpi = (aIn.type == VT_MPI || bIn.type == VT_MPI);
    if (!needMpi && op == "^" && fabs(aIn.d_val) >= 2 && fabs(bIn.d_val) >= 50) needMpi = true;
    if (!needMpi && op == "*" && fabs(aIn.d_val) > 1e15 && fabs(bIn.d_val) > 1e15) needMpi = true;

    if (needMpi) {
        // 需要时可修改的本地副本
        CalcValue a = aIn;
        CalcValue b = bIn;
        if (!a.promoteToMpi()) { errMsg += "错误: 无法将含小数的 " + String(a.d_val) + " 与大数混合运算\n"; return false; }
        if (!b.promoteToMpi()) { errMsg += "错误: 无法将含小数的 " + String(b.d_val) + " 与大数混合运算\n"; return false; }
        // result 先清空 mpi，防止真机上 mbedtls_mpi_xxx 写入已非空 mpi 时损坏
        mbedtls_mpi_free(&result.mpi_val);
        mbedtls_mpi_init(&result.mpi_val);
        result.type = VT_MPI;
        if (op == "+") { int r = mbedtls_mpi_add_mpi(&result.mpi_val, &a.mpi_val, &b.mpi_val); if (r != 0) { errMsg += "错误: 大数加法失败\n"; return false; } return true; }
        if (op == "-") { int r = mbedtls_mpi_sub_mpi(&result.mpi_val, &a.mpi_val, &b.mpi_val); if (r != 0) { errMsg += "错误: 大数减法失败\n"; return false; } return true; }
        if (op == "*") { int r = mbedtls_mpi_mul_mpi(&result.mpi_val, &a.mpi_val, &b.mpi_val); if (r != 0) { errMsg += "错误: 大数乘法失败\n"; return false; } return true; }
        if (op == "/") {
            if (mbedtls_mpi_cmp_int(&b.mpi_val, 0) == 0) { errMsg += "错误: 除数不能为零\n"; return false; }
            mbedtls_mpi rem; mbedtls_mpi_init(&rem);
            int ret = mbedtls_mpi_div_mpi(&result.mpi_val, &rem, &a.mpi_val, &b.mpi_val);
            mbedtls_mpi_free(&rem);
            if (ret != 0) { errMsg += "错误: 大数除法失败\n"; return false; }
            return true;
        }
        if (op == "^") { int r = mbedtls_mpi_pow(&result.mpi_val, &a.mpi_val, &b.mpi_val); if (r != 0) { errMsg += "错误: 大数幂失败\n"; return false; } return true; }
        errMsg += "错误: 大数不支持运算符 " + op + "\n"; return false;
    } 
    
    result.type = VT_DOUBLE;
    if (op == "+") result.d_val = aIn.d_val + bIn.d_val;
    else if (op == "-") result.d_val = aIn.d_val - bIn.d_val;
    else if (op == "*") result.d_val = aIn.d_val * bIn.d_val;
    else if (op == "/") {
        if (bIn.d_val == 0) { errMsg += "错误: 除数不能为零\n"; return false; }
        result.d_val = aIn.d_val / bIn.d_val; 
    }
    else if (op == "^") {
        result.d_val = std::pow(aIn.d_val, bIn.d_val);
        if (isinf(result.d_val)) { errMsg += "警告: 浮点幂溢出，建议使用大整数\n"; return false; }
    }
    else { errMsg += "错误: 未知运算符 " + op + "\n"; return false; }
    return true;
}

bool ArduinoCalc::applyUnaryOp(const CalcValue& a, const String& op, CalcValue& result, String& errMsg) {
    result = a;
    if (op == "#") {
        if (result.type == VT_DOUBLE) result.d_val = -result.d_val;
        else {
            // 0 - X = 取反
            mbedtls_mpi zero; mbedtls_mpi_init(&zero);
            mbedtls_mpi temp; mbedtls_mpi_init(&temp);
            mbedtls_mpi_sub_mpi(&temp, &zero, &result.mpi_val);
            mbedtls_mpi_free(&result.mpi_val);
            mbedtls_mpi_init(&result.mpi_val);
            mbedtls_mpi_copy(&result.mpi_val, &temp);
            mbedtls_mpi_free(&zero); mbedtls_mpi_free(&temp);
        }
        return true;
    }
    if (op == "@") return true;
    errMsg += "错误: 未知一元运算符\n"; return false;
}

bool ArduinoCalc::registerFunction(const String& name, CalcCallback callback) {
    for (int i = 0; i < customFuncCount; i++) if (customFunctions[i].name == name) { customFunctions[i].callback = callback; return true; }
    if (customFuncCount >= MAX_CUSTOM_FUNCTIONS) return false;
    customFunctions[customFuncCount++] = {name, callback};
    return true;
}

// ==========================================
// 核心引擎重构 (CalcValue 全链路大数穿透)
// ==========================================
bool ArduinoCalc::evaluateExpression(const String& expr, CalcValue& result, String& errMsg) {
    std::stack<CalcValue> values;
    std::stack<String> operators;

    int i = 0;
    while (i < expr.length()) {
        char currentChar = expr[i];

        // 【优化：UTF-8 拦截与智能提取】
        if (currentChar & 0x80) { 
            int len = getUtf8CharLen(currentChar);
            String badChar = (i + len <= expr.length()) ? expr.substring(i, i+len) : expr.substring(i);
            errMsg += "错误: 无效字符 '" + badChar + "' (算式需为纯英文/数字)\n";
            return false;
        }

        // 【优化：纯小数0省略支持 (.5 -> 0.5) + 多进制前缀 0x/0b/0o】
        if (isdigit(currentChar) || currentChar == '.') {
            String numStr;
            // 检测 0x / 0b / 0o 前缀（首字符为 '0' 且下一个是 x/b/o）
            if (currentChar == '0' && i + 1 < expr.length()) {
                char nx = expr[i + 1];
                if (nx == 'x' || nx == 'X' || nx == 'b' || nx == 'B' || nx == 'o' || nx == 'O') {
                    int radix = (nx == 'x' || nx == 'X') ? 16 : ((nx == 'b' || nx == 'B') ? 2 : 8);
                    numStr += '0'; numStr += nx; i += 2;
                    String digits;
                    while (i < expr.length()) {
                        char c = expr[i];
                        if (radix == 16 && ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) { digits += c; i++; }
                        else if (radix == 2 && (c == '0' || c == '1')) { digits += c; i++; }
                        else if (radix == 8 && (c >= '0' && c <= '7')) { digits += c; i++; }
                        else break;
                    }
                    if (digits.length() == 0) { errMsg += "错误: 进制前缀后无有效数字\n"; return false; }
                    // 解析为 mbedtls_mpi，再判断是否需要降为 double
                    CalcValue v; v.type = VT_MPI; v.d_val = 0;
                    int rret = mbedtls_mpi_read_string(&v.mpi_val, radix, digits.c_str());
                    if (rret != 0) {
                        // 解析失败，降级为 double
                        v.type = VT_DOUBLE;
                        v.d_val = (double)strtoull(digits.c_str(), nullptr, radix);
                    }
                    values.push(v);
                    continue;
                }
            }
            while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) { numStr += expr[i]; i++; }
            if (numStr.startsWith(".")) numStr = "0" + numStr; // 纯小数前置补0
            
            if (numStr.indexOf('.') != -1 || numStr.length() <= 15) {
                values.push(CalcValue::fromDouble(numStr.toDouble()));
            } else {
                values.push(CalcValue::fromMpiStr(numStr));
            }
            continue; 
        }
        
        // 处理字母与函数括号
        else if (isalpha(currentChar) || currentChar == '_') { // 支持 log_ 中的下划线
            String funcName;
            while (i < expr.length() && (isalpha(expr[i]) || expr[i] == '_')) { funcName += expr[i]; i++; }
            // 允许函数名尾部跟数字 (如 log2, log10)；首字符必须为字母/下划线
            while (i < expr.length() && isdigit(expr[i])) { funcName += expr[i]; i++; }
            funcName.toLowerCase();
            while (i < expr.length() && isspace(expr[i])) i++;
            if (i >= expr.length() || expr[i] != '(') { errMsg += "错误: 函数缺少括号\n"; return false; }
            i++; 
            int parenCount = 1; String argStr = "";
            while (i < expr.length() && parenCount > 0) {
                if (expr[i] == '(') parenCount++; else if (expr[i] == ')') { parenCount--; if (parenCount == 0) break; }
                argStr += expr[i]; i++;
            }
            if (parenCount != 0) { errMsg += "错误: 括号不匹配\n"; return false; }
            i++; 

            CalcValue res;
            // 【新增：abs() 支持大数绝对值】
            if (funcName == "abs") {
                CalcValue innerRes; String innerErr;
                if (!evaluateExpression(argStr, innerRes, innerErr)) { errMsg += innerErr; return false; }
                res = innerRes;
                if (res.type == VT_MPI) {
                    if (mbedtls_mpi_cmp_int(&res.mpi_val, 0) < 0) {
                        mbedtls_mpi zero; mbedtls_mpi_init(&zero);
                        mbedtls_mpi temp; mbedtls_mpi_init(&temp);
                        mbedtls_mpi_sub_mpi(&temp, &zero, &res.mpi_val);
                        mbedtls_mpi_free(&res.mpi_val);
                        mbedtls_mpi_init(&res.mpi_val);
                        mbedtls_mpi_copy(&res.mpi_val, &temp);
                        mbedtls_mpi_free(&zero); mbedtls_mpi_free(&temp);
                    }
                }
                else res.d_val = fabs(res.d_val);
            } 
            // 【新增：对数家族】
            else if (funcName == "log" || funcName == "log2" || funcName == "log10") {
                CalcValue innerRes; String innerErr;
                if (!evaluateExpression(argStr, innerRes, innerErr)) { errMsg += innerErr; return false; }
                double v = innerRes.toDouble();
                if (v <= 0) { errMsg += "错误: 对数定义域需 > 0\n"; return false; }
                res.type = VT_DOUBLE;
                if (funcName == "log") res.d_val = std::log(v);
                else if (funcName == "log2") res.d_val = std::log2(v);
                else res.d_val = std::log10(v);
            }
            // 【新增：任意底对数 log_(n, 底数)】
            else if (funcName == "log_") {
                double v1, v2;
                if (!evaluate2Args(argStr, v1, v2, errMsg)) return false;
                if (v1 <= 0 || v2 <= 0 || v2 == 1) { errMsg += "错误: 对数真数>0且底数>0,≠1\n"; return false; }
                res.type = VT_DOUBLE;
                res.d_val = std::log(v1) / std::log(v2); // 换底公式
            }
            // 三角函数与 sqrt (浮点域)
            else if (funcName == "sin" || funcName == "cos" || funcName == "tan" || funcName == "asin" || funcName == "acos" || funcName == "atan" || 
                     funcName == "sinr" || funcName == "cosr" || funcName == "tanr" || funcName == "asinr" || funcName == "acosr" || funcName == "atanr" || funcName == "sqrt") {
                CalcValue innerRes; String innerErr;
                if (!evaluateExpression(argStr, innerRes, innerErr)) { errMsg += innerErr; return false; }
                double v = innerRes.toDouble();
                res.type = VT_DOUBLE;
                if (funcName == "sin") res.d_val = sin(v*DEG_TO_RAD);
                else if (funcName == "cos") res.d_val = cos(v*DEG_TO_RAD);
                else if (funcName == "tan") res.d_val = tan(v*DEG_TO_RAD);
                else if (funcName == "asin") { if(v<-1||v>1){errMsg+="错误:asin域[-1,1]\n";return false;} res.d_val=asin(v)*RAD_TO_DEG; }
                else if (funcName == "acos") { if(v<-1||v>1){errMsg+="错误:acos域[-1,1]\n";return false;} res.d_val=acos(v)*RAD_TO_DEG; }
                else if (funcName == "atan") res.d_val = atan(v)*RAD_TO_DEG;
                else if (funcName == "sinr") res.d_val = sin(v);
                else if (funcName == "cosr") res.d_val = cos(v);
                else if (funcName == "tanr") res.d_val = tan(v);
                else if (funcName == "asinr") { if(v<-1||v>1){errMsg+="错误:asinr域[-1,1]\n";return false;} res.d_val=asin(v); }
                else if (funcName == "acosr") { if(v<-1||v>1){errMsg+="错误:acosr域[-1,1]\n";return false;} res.d_val=acos(v); }
                else if (funcName == "atanr") res.d_val = atan(v);
                else if (funcName == "sqrt") { if(v<0){errMsg+="错误:sqrt域[0,+∞)\n";return false;} res.d_val=sqrt(v); }
            }
            // 自定义函数
            else if (isCustomFunc(funcName)) {
                CalcCallback cb = getCustomCallback(funcName);
                res = CalcValue::fromDouble(cb(argStr)); 
                if (isnan(res.d_val)) { errMsg += "错误: 自定义函数执行失败\n"; return false; }
            } 
            else { errMsg += "错误: 未知函数 '" + funcName + "'\n"; return false; }
            
            values.push(res);
            continue;
        }
        
        else if (currentChar == '(') { operators.push(String(currentChar)); }
        else if (currentChar == ')') {
            while (!operators.empty() && operators.top() != "(") {
                String op = operators.top(); operators.pop();
                CalcValue res;
                if (op == "#" || op == "@") {
                    if (values.empty()) { errMsg += "错误: 格式不正确\n"; return false; }
                    CalcValue val = values.top(); values.pop();
                    if (!applyUnaryOp(val, op, res, errMsg)) return false;
                } else {
                    if (values.size() < 2) { errMsg += "错误: 格式不正确\n"; return false; }
                    CalcValue val2 = values.top(); values.pop();
                    CalcValue val1 = values.top(); values.pop();
                    if (!applyBinaryOp(val1, val2, op, res, errMsg)) return false;
                }
                values.push(res);
            }
            if (operators.empty()) { errMsg += "错误: 括号不匹配\n"; return false; }
            operators.pop(); 
        }
        else if (currentChar == '+' || currentChar == '-' || currentChar == '*' || currentChar == '/' || currentChar == '^') {
            if ((currentChar == '-' || currentChar == '+') && 
                (i == 0 || expr[i-1] == '(' || (expr[i-1] != ')' && !isdigit(expr[i-1]) && expr[i-1] != '.' && expr[i-1] != '_'))) {
                operators.push(currentChar == '-' ? "#" : "@");
            } else {
                String curOp = String(currentChar);
                while (!operators.empty() && operators.top() != "(") {
                    String topOp = operators.top();
                    int topPrec = (topOp == "#" || topOp == "@") ? 4 : (topOp == "^" ? 3 : (topOp == "*" || topOp == "/" ? 2 : 1));
                    int curPrec = (currentChar == '^') ? 3 : (currentChar == '*' || currentChar == '/') ? 2 : 1;
                    if (currentChar == '^') { if (topPrec <= curPrec) break; } else { if (topPrec < curPrec) break; }
                    operators.pop();
                    CalcValue res;
                    if (topOp == "#" || topOp == "@") {
                        if (values.empty()) { errMsg += "错误: 格式不正确\n"; return false; }
                        CalcValue val = values.top(); values.pop();
                        if (!applyUnaryOp(val, topOp, res, errMsg)) return false;
                    } else {
                        if (values.size() < 2) { errMsg += "错误: 格式不正确\n"; return false; }
                        CalcValue val2 = values.top(); values.pop();
                        CalcValue val1 = values.top(); values.pop();
                        if (!applyBinaryOp(val1, val2, topOp, res, errMsg)) return false;
                    }
                    values.push(res);
                }
                operators.push(curOp);
            }
        } else if (!isspace(currentChar)) {
            errMsg += "错误: 无效字符 '" + String(currentChar) + "'\n"; return false;
        }
        i++;
    }

    while (!operators.empty()) {
        if (operators.top() == "(") { errMsg += "错误: 括号不匹配\n"; return false; }
        String op = operators.top(); operators.pop();
        CalcValue res;
        if (op == "#" || op == "@") {
            if (values.empty()) { errMsg += "错误: 格式不正确\n"; return false; }
            CalcValue val = values.top(); values.pop();
            if (!applyUnaryOp(val, op, res, errMsg)) return false;
        } else {
            if (values.size() < 2) { errMsg += "错误: 格式不正确\n"; return false; }
            CalcValue val2 = values.top(); values.pop();
            CalcValue val1 = values.top(); values.pop();
            if (!applyBinaryOp(val1, val2, op, res, errMsg)) return false;
        }
        values.push(res);
    }

    if (values.size() != 1) { errMsg += "错误: 格式不正确\n"; return false; }
    result = values.top();
    return true;
}

// ==========================================
// 外部包装：前缀检测与格式化输出
// ==========================================
bool ArduinoCalc::calculate(const String& input, double& result, String& errMsg, String& outStr) {
    String expression = input;
    bool isMultiBase = false;
    
    // 【新增：自动截取等号前的算式】
    int eqPos = expression.indexOf('=');
    if (eqPos != -1) {
        expression = expression.substring(0, eqPos);
    }
    
    // 移除空格与判断 | 前缀
    String cleanExpr;
    for (char c : expression) {
        if (!isspace(c)) {
            if (c == '|' && cleanExpr.length() == 0) { isMultiBase = true; continue; }
            cleanExpr += c;
        }
    }
    
    CalcValue finalVal;
    if (!evaluateExpression(cleanExpr, finalVal, errMsg)) return false;

    result = finalVal.toDouble();

    if (isMultiBase) {
        // 多进制输出：用 concat 增量构建，避免长链式 String + 临时对象在 ESP32 上引发内存损坏
        if (finalVal.type == VT_MPI) {
            mbedtls_mpi abs_val; mbedtls_mpi_init(&abs_val);
            mbedtls_mpi_copy(&abs_val, &finalVal.mpi_val);

            if (mbedtls_mpi_cmp_int(&abs_val, 0) < 0) {
                mbedtls_mpi zero; mbedtls_mpi_init(&zero);
                mbedtls_mpi temp; mbedtls_mpi_init(&temp);
                mbedtls_mpi_sub_mpi(&temp, &zero, &abs_val);
                mbedtls_mpi_free(&abs_val); mbedtls_mpi_init(&abs_val);
                mbedtls_mpi_copy(&abs_val, &temp);
                mbedtls_mpi_free(&zero); mbedtls_mpi_free(&temp);
            } // 提取绝对值用于拼接

            String s_dec = CalcValue::mpiToString(&abs_val, 10);
            String s_hex = CalcValue::mpiToString(&abs_val, 16); s_hex.toUpperCase();
            String s_bin = CalcValue::mpiToString(&abs_val, 2);
            String s_oct = CalcValue::mpiToString(&abs_val, 8);
            mbedtls_mpi_free(&abs_val);

            const char *sign = (mbedtls_mpi_cmp_int(&finalVal.mpi_val, 0) < 0) ? "-" : "";

            // 安全构建：concat
            outStr =      sign;                       outStr.concat(s_dec); outStr.concat("\n");
            outStr.concat(sign); outStr.concat("0x"); outStr.concat(s_hex); outStr.concat("\n");
            outStr.concat(sign); outStr.concat("0b"); outStr.concat(s_bin); outStr.concat("\n");
            outStr.concat(sign); outStr.concat("0o"); outStr.concat(s_oct);
        } else {
            long intVal = lround(finalVal.d_val);
            unsigned long uVal = (intVal < 0) ? (unsigned long)(-intVal) : (unsigned long)intVal;
            String sign = (intVal < 0) ? "-" : "";
            String decStr = String((long)intVal);
            String hexStr = String(uVal, HEX); hexStr.toUpperCase();
            String binStr = String(uVal, BIN);
            String octStr = String(uVal, OCT);

            outStr = "";
            outStr.concat(sign); outStr.concat(decStr);
            outStr.concat(" ");
            outStr.concat(sign); outStr.concat("0x"); outStr.concat(hexStr);
            outStr.concat(" ");
            outStr.concat(sign); outStr.concat("0b"); outStr.concat(binStr);
            outStr.concat(" ");
            outStr.concat(sign); outStr.concat("0o"); outStr.concat(octStr);
        }
    } else {
        if (finalVal.type == VT_MPI) {
            outStr = ""; // 先清空，防止 ESP32 SSO 缓冲残留
            outStr.concat(CalcValue::mpiToString(&finalVal.mpi_val, 10));
        } else {
            outStr = "";
            outStr.concat(String(finalVal.d_val));
        }
    }
    
    return true;
}














// // --- 辅助函数：获取运算符优先级 ---
// int __getPrecedence(char op) {
//     if (op == '+' || op == '-') return 1;
//     if (op == '*' || op == '/') return 2;
//     if (op == '^') return 3;
//     if (op == '#' || op == '@') return 4; // '#'代表一元负号, '@'代表一元正号，优先级最高
//     return 0; 
// }

// // --- 辅助函数：执行一次二元运算 ---
// bool __applyBinaryOp(double a, double b, char op, double& result, String& errMsg) {
//     switch(op) {
//         case '+': result = a + b; break;
//         case '-': result = a - b; break;
//         case '*': result = a * b; break;
//         case '/':
//             if (b == 0) {
//                 errMsg += "#bf160d 错误: 除数不能为零 #\n";
//                 return false;
//             }
//             result = a / b; 
//             break;
//         case '^': 
//             result = std::pow(a, b); 
//             break;
//         default:
//             errMsg += "#bf160d 错误: 未知二元运算符 #\n";
//             return false;
//     }
//     return true;
// }

// // --- 辅助函数：执行一次一元运算 ---
// bool __applyUnaryOp(double a, char op, double& result, String& errMsg) {
//     if (op == '#') { result = -a; return true; }
//     if (op == '@') { result = a; return true; }
//     errMsg += "#bf160d 错误: 未知一元运算符 #\n";
//     return false;
// }

// // 计算表达式字符串的值, 自动检测 '=' 号, 并截取表达式
// bool calculate(const String& input, double& result, String& errMsg) {
//     size_t pos = input.indexOf('=');
//     String expression = (pos != -1) ? input.substring(0, pos) : input;
        
//     std::stack<double> values;
//     std::stack<char> operators;
    
//     // 1. 预处理：移除所有空格
//     String expr;
//     for (char c : expression) {
//         if (!isspace(c)) {
//             expr += c;
//         }
//     }

//     int i = 0;
//     while (i < expr.length()) {
//         char currentChar = expr[i];

//         // 2. 处理数字和小数点
//         if (isdigit(currentChar) || currentChar == '.') {
//             String numStr;
//             while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) {
//                 numStr += expr[i];
//                 i++;
//             }
//             try {
//                 values.push(numStr.toDouble());
//             } catch (const std::exception& e) {
//                 errMsg += "#bf160d 错误: 无效的数字格式 '" + numStr + "' #\n";
//                 return false;
//             }
//             continue; 
//         }
        
//         // 3. 处理左括号
//         else if (currentChar == '(') {
//             operators.push(currentChar);
//         }
        
//         // 4. 处理右括号
//         else if (currentChar == ')') {
//             while (!operators.empty() && operators.top() != '(') {
//                 char op = operators.top(); operators.pop();
                
//                 double res;
//                 if (op == '#' || op == '@') { // 一元运算符
//                     if (values.empty()) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
//                     double val = values.top(); values.pop();
//                     if (!__applyUnaryOp(val, op, res, errMsg)) return false;
//                 } else { // 二元运算符
//                     if (values.size() < 2) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
//                     double val2 = values.top(); values.pop();
//                     double val1 = values.top(); values.pop();
//                     if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
//                 }
//                 values.push(res);
//             }
//             if (operators.empty()) {
//                  errMsg += "#bf160d 错误: 括号不匹配 #\n";
//                  return false;
//             }
//             operators.pop(); // 弹出左括号 '('
//         }
        
//         // 5. 处理运算符 (+, -, *, /, ^)
//         else if (currentChar == '+' || currentChar == '-' || currentChar == '*' || currentChar == '/' || currentChar == '^') {
//             // 处理一元正负号：若在首位，或在前导运算符/左括号之后
//             if ((currentChar == '-' || currentChar == '+') && 
//                 (i == 0 || expr[i-1] == '(' || __getPrecedence(expr[i-1]) > 0)) {
//                 // 将一元负号记为 '#'，一元正号记为 '@'
//                 operators.push(currentChar == '-' ? '#' : '@');
//             }
//             else {
//                 // 二元运算符优先级与结合性处理
//                 while (!operators.empty() && operators.top() != '(') {
//                     char topOp = operators.top();
//                     if (currentChar == '^') {
//                         if (__getPrecedence(topOp) <= __getPrecedence(currentChar)) break; // 右结合：只有栈顶严格大于才弹出
//                     } else {
//                         if (__getPrecedence(topOp) < __getPrecedence(currentChar)) break; // 左结合：栈顶大于等于则弹出
//                     }
                    
//                     // 执行栈顶运算
//                     char op = topOp; operators.pop();
//                     double res;
//                     if (op == '#' || op == '@') {
//                         if (values.empty()) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
//                         double val = values.top(); values.pop();
//                         if (!__applyUnaryOp(val, op, res, errMsg)) return false;
//                     } else {
//                         if (values.size() < 2) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
//                         double val2 = values.top(); values.pop();
//                         double val1 = values.top(); values.pop();
//                         if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
//                     }
//                     values.push(res);
//                 }
//                 operators.push(currentChar);
//             }
//         }
//         // 如果遇到无法识别的字符
//         else {
//              // 注意：char 类型不能直接用 + 和 string 拼接，需转为 string
//              errMsg += "#bf160d 错误: 无效的字符 '" + String(currentChar) + "' #\n";
//              return false;
//         }
//         i++;
//     }

//     // 6. 处理完所有字符后，计算栈中剩余的运算符
//     while (!operators.empty()) {
//         if (operators.top() == '(') {
//              errMsg += "#bf160d 错误: 括号不匹配 #\n";
//              return false;
//         }
//         char op = operators.top(); operators.pop();
//         double res;
//         if (op == '#' || op == '@') {
//             if (values.empty()) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
//             double val = values.top(); values.pop();
//             if (!__applyUnaryOp(val, op, res, errMsg)) return false;
//         } else {
//             if (values.size() < 2) { errMsg += "#bf160d 错误: 表达式格式不正确 #\n"; return false; }
//             double val2 = values.top(); values.pop();
//             double val1 = values.top(); values.pop();
//             if (!__applyBinaryOp(val1, val2, op, res, errMsg)) return false;
//         }
//         values.push(res);
//     }

//     if (values.size() != 1) {
//          errMsg += "#bf160d 错误: 表达式格式不正确 #\n";
//          return false;
//     }
    
//     result = values.top();
//     return true;
// }
