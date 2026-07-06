/**
 ***************************** 声明 - Declaration ********************************
 * @file           : main.cpp
 * @original_author: FriedParrot (https://github.com/FriedParrot)
 * @author         : Ported by Ioqit (https://github.com/Ioqite)
 * @version        : v1.6-arduino
 * @original_date  : 2024-09-20
 * @date           : 2026-05-04 (last modified)
 * @derived_from   : by FriedParrot/zh_pinyin_decoder(v1.6) (https://github.com/FriedParrot/zh_pinyin_decoder)
 * @brief          : 中文拼音输入法的解码器示例程序 - Decoder example program for chinese pinyin inputting method
 * @copyright      : Copyright (c) 2024 FriedParrot
 *                   Copyright (c) 2026 Ioqit (Arduino Port)
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @attention
 * 此文件是整个输入法的示例程序, 包含了所有测试用例。
 * This is the c++ example program for the whole inputting method.
 * 
 * test1 : 词库完整性测试 (library integrity test)
 * test2 : 单个汉字模糊匹配测试 (vague match test: input string for vague match)
 * test3 : 拼音分词测试 (pinyin split test : mixed pinyin string (no space))
 * test4 : 带词库的完整的输入法测试 (word match test : input mixed pinyin string (no space)) 
 * 所有测试用例的详细信息都可以在 https://github.com/FRIEDparrot/zh_pinyin_decoder 找到
 * All the test cases can be found in detail at https://github.com/FRIEDparrot/zh_pinyin_decoder
 * 
 * @note 原 note: 
 * 这种输入法是一种轻量的、可移植的中文拼音输入法。
 * 你可以将它移植到你的嵌入式平台（例如，stm32、esp32 或 Arduino）用于提供中文拼音输入识别支持。
 * This inputting method is a lightweight, portable Chinese pinyin inputting method.
 * You can port it to your embedded platform (for example, stm32, esp32 or audrino) 
 * to give support for Chinese pinyin input recognition. 
 *****************************************************************************
 */

#include <Arduino.h>
#include "zh_pinyin_decoder.hpp"

void test1();
void test2();
void test3();
void test4();

void setup() {
    Serial.begin(115200);
    zh_pinyin_begin();
    Serial.println("\n============================= Test Start ==================================");
    test1(); // 词库完整性测试
    test2(); // 单个汉字模糊匹配测试
    test3(); // 拼音分词测试 (无空格)
#if (USE_ZH_WORD_MATCH == 1)
    test4(); // 带词库的完整的输入法测试 (无空格)
#endif
    zh_pinyin_end();
    Serial.println("\n============================= Test END ==================================\n");
}

void loop() {}


// 词库完整性测试 (library integrity test)
void test1() {
    Serial.println("\n********** Test 1 : 词库完整性测试 (library integrity test) ***********");
    
    clock_t start_time = clock();
    char res_str[MAX_CODE_BUFF_SZ];
    for (int i = 0; i < 26; i++) {
        for (int j = 0; j < code_index[i].table_length; j++) {
            const char* str = code_index[i].code_table[j];
            memset(res_str, 0, sizeof(res_str));
            uint8_t br;
            if (!zh_match_code_prec(str, res_str, MAX_CODE_SEARCH_TYPES, &br)) {
                Serial.printf("%s : %d %s\n", str, br, String(res_str).c_str());
            } else {
                Serial.println("No file to match !");
                break;
            }
        }
    }
    Serial.printf("executing time : %d ms \n", clock() - start_time);
}

// 单个汉字模糊匹配测试 (vague match test: input string for vague match)
void test2() {
    Serial.println("\n*********** Test2: 单个汉字模糊匹配测试 (vague match test) ******************");
    // Serial.printf("===================    enter \"exit()\" to exit  ====================================\n");

    for (String input_str : {"de", "di", "du", "hao", "shi", "jie", "exit()"}) {
    // while (1) {
        // String input_str = Serial.readStringUntil('\n');
        // input_str.trim();
        // input_str.toLowerCase();
        if (input_str == "exit()") break;

        // 计时
        clock_t start_time = clock();

        uint8_t br; // 匹配结果数量
        char res_str2[MAX_CODE_BUFF_SZ]; // 所有匹配结果组成的字符串

        // 模糊匹配
        uint8_t res = zh_match_code_vague(input_str.c_str(), res_str2, MAX_CODE_SEARCH_TYPES, &br);
        
        Serial.printf("Search %s, Time: %d ms\n", input_str.c_str(), clock() - start_time);
        if (res) {
            Serial.println("match failed : nothing to match");
        } else {
            // 分割所有结果并逆序输出, 显示匹配结果
            for (int i = br - 1, j = 0; i >= 0; i--, j++) {
                // 分割每个字符
                std::string str_u8 = res_str2; // String -> std::string
                std::u32string str_u32 = u8_to_u32(str_u8); // std::string -> std::u32string
                std::u32string str_u32_subed = str_u32.substr(i, 1); // 截取单个字符
                String s2 = u32_to_u8(str_u32_subed).c_str(); // std::u32string -> std::string -> String
                Serial.printf("%d:%s", j + 1, s2.c_str());
                if (i != 0) {
                    Serial.print(", ");
                }
            }
            Serial.println();
        }
    }
}

// 拼音分词测试 (无空格) (pinyin split test : mixed pinyin string (no space))
void test3() {
    Serial.println("\n*********** Test3: 拼音分词测试 (pinyin split test) ******************");
    // Serial.printf("===================    enter \"exit()\" to exit  ====================================\n");

    // while (1) {
    for (String input_str : {"de", "di", "du", "hao", "shi", "jie", "exit()"}) {
		// String input_str = Serial.readStringUntil('\n');
        // input_str.trim();
        // input_str.toLowerCase();

        // string input_str;
        // std::getline(std::cin, input_str);
		
        if (input_str == "exit()") break;

        const char* s = input_str.c_str();
        uint32_t start_time = clock();
        __split_method_list_t* m_list = zh_pinyin_get_split(s);
        zh_pinyin_filter_split(m_list);  /* use the filter option to eliminate unwanted result */
        uint32_t end_time = clock();
        zh_pinyin_show_split(s, m_list);
        Serial.printf("splitting string take time : %d ms\n", end_time - start_time);
        zh_pinyin_free_split(m_list);  /* free the space */
    }
}

#if (USE_ZH_WORD_MATCH == 1)
// 带词库的完整的输入法测试 (无空格) (word match test : input mixed pinyin string (no space))
void test4() {
    Serial.println("\n******************* Test4: 带词库的完整的输入法测试 (word match test) *******************");
    // Serial.printf("===================    enter \"exit()\" to exit  ====================================\n");
    
    for (String input_str : {"nihao", "shijie", "duqu", "shiwu", "weishenme", "exit()"}) {
    // while (1) {
		// String input_str = Serial.readStringUntil('\n');
		// input_str.trim();
        // input_str.toLowerCase();

        if (input_str == "exit()") break;

        // 这是一个用于声明 __word_block_t 存储结构的简单代码
        // This is a simple code for declare the storge structure of __word_block_t
        match_case_node_t sp;
        uint16_t idx = 0;

        clock_t start_time = clock();
        word_dict_search_result_t *blk = zh_match_word(input_str.c_str(), &sp);
        clock_t end_time = clock();

        uint8_t loc = 0;
        Serial.println("\n");
        for (int i = 0; i < strlen(input_str.c_str()); i++) {
            if (i == sp.spm[loc]) {
                Serial.print("'");
                loc++;
            }
            Serial.print(input_str.c_str()[i]);
        }
        Serial.printf(", %d ms\n", end_time - start_time);
        char code_str[3 * MAX_WORD_LENGTH + 1];
        for (word_dict_search_result_t *w = blk; w != NULL; w = w->next) {
            if (w->type == WORD_BLK_TYPE_CODES) {
                for (int i = 0; i < w->num.code_nbr; i++) {
                    idx++;
                    strncpy(code_str, w->buf + 3 * i, 3);
                    code_str[3] = '\0';
                    Serial.printf("%d: %s ", idx, code_str);
                }
            } else { /* WORD_BLK_TYPE_WORDS */
                uint16_t buf_idx = 0;
                for (int i = 0; w->num.word_nbr[i] != 0; i++) {
                    idx++;
                    // Serial.println();
                    // Serial.println(3 * w->num.word_nbr[i]);
                    strncpy(code_str, w->buf + buf_idx, 3 * w->num.word_nbr[i]);
                    buf_idx += 3 * w->num.word_nbr[i];
                    code_str[3 * w->num.word_nbr[i]] = '\0';
                    Serial.printf("%d: %s ", idx, code_str);
                }
            }
        }
        zh_word_free_match(blk);
    }
}
#endif


