#pragma once

#include <Arduino.h>
#include <stack>
#include <cmath>
#include <mbedtls/bignum.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (M_PI / 180.0)
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0 / M_PI)
#endif

#define MAX_CUSTOM_FUNCTIONS 10

typedef double (*CalcCallback)(const String&);
enum ValueType { VT_DOUBLE, VT_MPI };


class CalcValue {
public:
    ValueType type;
    double d_val;
    mbedtls_mpi mpi_val;

    CalcValue();
    CalcValue(const CalcValue& other);
    CalcValue& operator=(const CalcValue& other);
    ~CalcValue();
    
    static CalcValue fromDouble(double d);
    static CalcValue fromMpiStr(const String& s);
    bool promoteToMpi();

    // 大数强制降级为 double (用于对数等浮点专属运算)
    double toDouble() const;

    // 优化的动态内存 MPI 转字符串 (支持 2, 8, 10, 16 进制无限长度)
    static String mpiToString(const mbedtls_mpi* X, int radix);
};




// ==========================================
// 核心计算器类
// ==========================================
class ArduinoCalc {
private:
    struct CustomFunc { String name; CalcCallback callback; };
    CustomFunc customFunctions[MAX_CUSTOM_FUNCTIONS];
    int customFuncCount;

    bool isCustomFunc(const String& name) const;
    CalcCallback getCustomCallback(const String& name) const;
    
    // 全新重构的底层纯值计算引擎 (支撑大数穿透)
    bool evaluateExpression(const String& expr, CalcValue& result, String& errMsg);
    
    bool applyBinaryOp(const CalcValue& a, const CalcValue& b, const String& op, CalcValue& result, String& errMsg);
    bool applyUnaryOp(const CalcValue& a, const String& op, CalcValue& result, String& errMsg);

    // 双参数安全拆解 (用于 log_(n, base))
    bool evaluate2Args(const String& argStr, double& v1, double& v2, String& errMsg);

public:
    ArduinoCalc() : customFuncCount(0) {}
    bool registerFunction(const String& name, CalcCallback callback);
    
    // 对外公开接口 (负责处理多进制前缀 '|' 与最终格式化)
    bool calculate(const String& input, double& result, String& errMsg, String& outStr);
};









// // --- 辅助函数：获取运算符优先级 ---
// int __getPrecedence(char op);

// // --- 辅助函数：执行一次二元运算 ---
// bool __applyBinaryOp(double a, double b, char op, double& result, String& errMsg);

// // --- 辅助函数：执行一次一元运算 ---
// bool __applyUnaryOp(double a, char op, double& result, String& errMsg);

// // 计算表达式字符串的值, 自动检测 '=' 号, 并截取表达式
// bool calculate(const String& input, double& result, String& errMsg);



