# 内存泄漏修复说明 - Memory Leak Fixes

本文档记录了本版本相对于原始 `zh_pinyin_decoder-arduino` 库所做的全部内存泄漏修复。
This document records all memory leak fixes applied to the original `zh_pinyin_decoder-arduino` library.

## 修复的内存泄漏 - Fixed Memory Leaks

### 1. `zh_pinyin_get_split()` - 分词失败时链表泄漏
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:** 当 `pinyin_dfs()` 返回失败或分词结果为空时，已分配的 `m_list` 未被释放。
When `pinyin_dfs()` fails or returns an empty result, the allocated `m_list` was not freed.

**修复 / Fix:** 在失败路径调用 `mlist_destroy(m_list)` 释放链表。
Call `mlist_destroy(m_list)` on the failure path.

---

### 2. `zh_match_word()` - 过滤失败时 `m_list` 泄漏
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:** 当 `zh_pinyin_filter_split()` 返回失败时，已分配的 `m_list` 未被释放。
When `zh_pinyin_filter_split()` fails, the allocated `m_list` was not freed.

**修复 / Fix:** 在过滤失败路径调用 `zh_pinyin_free_split(m_list)` 释放链表。
Call `zh_pinyin_free_split(m_list)` on the filter failure path.

---

### 3. `mlist_insert()` - 重复节点泄漏
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:** 当插入的节点与已有节点比较结果为 `0`（重复）时，函数直接 `return` 而不释放新分配的节点 `m`。
When the inserted node is a duplicate (`cmp == 0`), the function returned without freeing the newly allocated node `m`.

**修复 / Fix:** 在重复路径调用 `zh_buffer_free(m)` 释放重复节点。
Call `zh_buffer_free(m)` on duplicate paths.

---

### 4. `word_dict_search()` - `w1` / `buf` / `word_nbr` 多处泄漏
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:**
- 当 `w1 == NULL` 但 `buf` 已分配时，`buf` 泄漏。
- 当 `res_tmp != 0` 但 `w1` 已分配时，`w1` 泄漏。
- 当 `word_nbr` 分配成功但 `w2` 分配失败时，`word_nbr` 泄漏（因为 `wordblock_destroy(NULL)` 不会释放 `word_nbr`）。
- 当文件读取失败（`file_word_dict.read() == 0`）时，`w2` 和 `word_nbr` 泄漏。
- 当 `!FS_is_begin` 时提前返回，`res_str` 和 `w_res`（含 `w1`）泄漏。
- 当 cJSON 解析成功但 `item->child == NULL` 或 `item->child->string[0] > str[0]` 时，`item` 未被释放。

**修复 / Fix:** 在所有错误路径上正确释放所有已分配的资源（`buf`、`w1`、`w2`、`word_nbr`、`res_str`、`w_res`、`item`）。
Properly free all allocated resources (`buf`, `w1`, `w2`, `word_nbr`, `res_str`, `w_res`, `item`) on all error paths.

---

### 5. `wordblock_reshape()` - `w3` 与 `w_res` 泄漏
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:**
- 当 `new_buf1` 或 `new_buf2` 分配失败时，已分配的 `w3` 未被释放。
- 当 `w3` 分配失败时，函数返回 `NULL`，但调用方 `w_res = wordblock_reshape(w_res, ...)` 会用返回值覆盖 `w_res`，导致原 `w_res` 泄漏。

**修复 / Fix:** 在所有失败路径上释放 `w3` 和 `w_res`，并明确约定：返回 `NULL` 时 `w_res` 已被释放。
Free `w3` and `w_res` on all failure paths; documented that `NULL` return means `w_res` has been freed.

---

### 6. `zh_match_code_vague()` - `v_idx` 泄漏
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:** 当 `get_match_idx()` 返回失败时，已分配的 `v_idx` 未被释放。
When `get_match_idx()` fails, the allocated `v_idx` was not freed.

**修复 / Fix:** 在 `get_match_idx` 失败路径上调用 `zh_buffer_free(v_idx)`。
Call `zh_buffer_free(v_idx)` on the `get_match_idx` failure path.

---

### 7. `zh_pinyin_begin()` - 文件句柄泄漏 + Bug 修复
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:**
- **Bug:** 打开 `file_word_dict` 后，错误地检查 `!file_code_table` 而非 `!file_word_dict`。如果词库文件打开失败，已打开的 `file_code_table` 不会被关闭（资源泄漏）。
- 当文件打开失败时，`FS_is_begin` 仍为 `true`，导致状态不一致。

**修复 / Fix:**
- 修正为检查 `!file_word_dict`。
- 在文件打开失败路径上关闭已打开的 `file_code_table`，并重置 `FS_is_begin = false`。
- Fix the check to `!file_word_dict`.
- Close the already-opened `file_code_table` on file open failure, and reset `FS_is_begin = false`.

---

### 8. `word_dict_exit()` - 修复无效的 `res_str = NULL`
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:** `res_str = NULL;` 修改的是局部参数副本，对调用方无影响。
`res_str = NULL;` modified a local copy of the parameter, having no effect on the caller.

**修复 / Fix:** 改为 `*res_str = NULL;` 真正置空调用方的指针。同时增加 NULL 检查。
Change to `*res_str = NULL;` to actually null out the caller's pointer. Added NULL check.

---

### 9. `zh_pinyin_free_split()` - 移除无效的 `m_list = NULL`
**文件 / File:** `src/zh_pinyin_decoder.cpp`

**问题 / Problem:** `m_list = NULL;` 修改的是局部参数副本，对调用方无影响。
`m_list = NULL;` modified a local copy of the parameter, having no effect on the caller.

**修复 / Fix:** 移除该无效语句，并在注释中提醒调用方自行将指针置空。
Removed the no-op statement and added a comment reminding callers to null their own pointer.

---

### 10. `example/test_ffat-arduino/test_ffat-arduino.ino` - 分区迭代器与文件句柄泄漏
**文件 / File:** `example/test_ffat-arduino/test_ffat-arduino.ino`

**问题 / Problem:**
- `partloop()`: 由 `esp_partition_find()` 返回的迭代器从未通过 `esp_partition_iterator_release()` 释放。
- `listDir()`: `root` 文件句柄在函数返回前未被关闭。另外 `root.isDirectory()` 为 false 的提前返回路径也未关闭 `root`。

**修复 / Fix:**
- 在 `partloop()` 中保存头迭代器，循环结束后调用 `esp_partition_iterator_release(head)` 释放整条链表。同时把 `iterator = esp_partition_next(iterator)` 移出 `if` 块以避免潜在的死循环。
- 在 `listDir()` 的所有退出路径上调用 `root.close()`。
- In `partloop()`, save the head iterator and call `esp_partition_iterator_release(head)` after the loop. Also move `iterator = esp_partition_next(iterator)` out of the `if` block to avoid a potential infinite loop.
- In `listDir()`, call `root.close()` on all exit paths.

---

### 11. `example/test_all-platformio/test/test_ffat_fs.cpp` - 同上
**文件 / File:** `example/test_all-platformio/test/test_ffat_fs.cpp`

与 `test_ffat-arduino.ino` 相同的修复。
Same fixes as `test_ffat-arduino.ino`.

---

## 验证 - Verification

所有修改后的源文件已通过 `g++ -fsyntax-only` 语法检查（在 FatFS / LittleFS、USE_ZH_WORD_MATCH=0/1、USE_ZH_HASH_BOOST=0/1 多种配置组合下均无错误）。
All modified source files pass `g++ -fsyntax-only` syntax checks under multiple configuration combinations (FatFS / LittleFS, USE_ZH_WORD_MATCH=0/1, USE_ZH_HASH_BOOST=0/1).

## 注意 - Notes

- `CJSON/cJSON.c` 是第三方标准库，未做修改。
- `CJSON/cJSON.c` is a third-party standard library and was not modified.
- 修复仅涉及内存管理，不改变原有功能逻辑。
- Fixes only concern memory management; original functional logic is preserved.
