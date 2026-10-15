//
// Created by RestRegular on 2025/8/26.
//

#ifndef RVM_VERSION_H
#define RVM_VERSION_H
#define VERSION_MAJOR 1
/*
 * 1.8.0：RSI 开始携带静态槽位表（帧表 + 每个参数的帧下标/槽位号）。
 * 旧版 RSI 没有这张表，反序列化后无法进行槽位寻址，因此不再兼容——
 * validate() 会以 "Unsupported version" 明确拒绝，请用新 RVM 重新 --comp。
 */
#define VERSION_MINOR 8
#define VERSION_PATCH 0
#endif //RVM_VERSION_H//
