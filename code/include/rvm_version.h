//
// Created by RestRegular on 2025/8/26.
//

#ifndef RVM_VERSION_H
#define RVM_VERSION_H
#define VERSION_MAJOR 1
/*
 * 1.8.0：RSI 开始携带静态槽位表（帧表 + 每个参数的帧下标/槽位号）。
 * 1.9.0：整数字段改用变长编码（LEB128 / zigzag），字符串改走路径表与变长长度前缀，
 *        ArgType 由 4 字节收窄为 1 字节。布局已变，旧文件不兼容——
 *        validate() 会以 "Unsupported version" 明确拒绝，请用新 RVM 重新 --comp。
 */
#define VERSION_MINOR 9
#define VERSION_PATCH 0
#endif //RVM_VERSION_H//
