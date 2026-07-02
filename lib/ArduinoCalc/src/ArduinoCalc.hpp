#pragma once

#include <Arduino.h>
#include <stack>

// --- 辅助函数：获取运算符优先级 ---
int __getPrecedence(char op);

// --- 辅助函数：执行一次二元运算 ---
bool __applyBinaryOp(double a, double b, char op, double& result, String& errMsg);

// --- 辅助函数：执行一次一元运算 ---
bool __applyUnaryOp(double a, char op, double& result, String& errMsg);

// 计算表达式字符串的值, 自动检测 '=' 号, 并截取表达式
bool calculate(const String& input, double& result, String& errMsg);

