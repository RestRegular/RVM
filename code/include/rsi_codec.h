//
// RSI 1.9.0 的变长编码与路径表。
//
// 设计说明（与文档 RSI-1.9.0-optimization-guide.md 的差异，见文件末尾）：
//   路径表**不走 thread_local 全局状态**，而是显式作为参数传递。
//   原因：ra_parser 在解析主文件的循环里，遇到 LINK + --precomp-link 会递归调用
//   serializeExecutableInsToBinaryFile。若用全局表，嵌套调用会把主文件已累积的路径清掉，
//   写进主文件的路径表就被链接文件的内容污染了。显式传参让嵌套调用各持一张表，天然隔离。
//

#ifndef RVM_RSI_CODEC_H
#define RVM_RSI_CODEC_H

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace utils {

    /* 单个字符串的长度上限。用于把损坏文件变成明确异常，而不是 bad_alloc。 */
    inline constexpr uint64_t RSI_MAX_STRING_LENGTH = 1ull << 28;

    /*
     * 校验收自文件的「数量」字段，再据此分配内存。
     *
     * 为什么必须有这层校验：
     *   这些数字来自 .rsi，**完全由输入决定**。若直接送进 vector::resize/reserve，
     *   一个几 KB 的畸形文件就能声明 10 亿个元素，逼迫进程去申请几十 GB——
     *   表现为整机卡死或 bad_alloc，而不是一条可读的错误。
     *   即便最终报出 bad_alloc，那也是内存已被吃光之后的事。
     *
     * 上限取值远大于任何真实产物（帧/槽位虽是 4 字节索引，实际规模在千级），
     * 同时远小于会耗尽内存的规模。超出即视为文件损坏，明确拒绝。
     */
    inline constexpr uint64_t RSI_MAX_FRAME_COUNT = 1ull << 16;   // 65536
    inline constexpr uint64_t RSI_MAX_SLOT_SPAN = 1ull << 16;     // 65536
    inline constexpr uint64_t RSI_MAX_NAME_COUNT = 1ull << 20;    // 1048576
    inline constexpr uint64_t RSI_MAX_PATH_COUNT = 1ull << 20;    // 1048576
    inline constexpr uint64_t RSI_MAX_INS_COUNT = 1ull << 24;     // 16777216
    inline constexpr uint64_t RSI_MAX_ARGS_COUNT = 1ull << 20;    // 1048576
    inline constexpr uint64_t RSI_MAX_LABEL_COUNT = 1ull << 20;   // 1048576

    /*
     * 无符号 LEB128。
     * 与字节序无关，7 位一组、低位在前，最高位为续读标志。
     */
    inline void writeVarint(std::ostream &out, uint64_t value) {
        while (value >= 0x80) {
            out.put(static_cast<char>((value & 0x7F) | 0x80));
            value >>= 7;
        }
        out.put(static_cast<char>(value));
    }

    inline uint64_t readVarint(std::istream &in) {
        uint64_t result = 0;
        for (int shift = 0; shift < 64; shift += 7) {
            const int c = in.get();
            if (c == std::istream::traits_type::eof()) {
                throw std::runtime_error("RSI: unexpected EOF while reading varint");
            }
            result |= static_cast<uint64_t>(c & 0x7F) << shift;
            if ((c & 0x80) == 0) {
                return result;
            }
        }
        throw std::runtime_error("RSI: varint too long");
    }

    /*
     * 读出数量字段并校验上限。
     *
     * 必须在 readVarint 之后定义（依赖它）。
     * what 用于错误信息（如 "frame count"），让用户知道是哪个字段坏了。
     */
    inline uint64_t readCount(std::istream &in, const uint64_t limit, const char *what) {
        const uint64_t value = readVarint(in);
        if (value > limit) {
            throw std::runtime_error(
                    std::string("RSI: ") + what + " is out of range (read " +
                    std::to_string(value) + ", limit " + std::to_string(limit) +
                    "). The file is corrupted or was not produced by this RVM version.");
        }
        return value;
    }

    /*
     * 有符号数：zigzag 后再走 LEB128。
     *
     * -1 必须走这里，不能用 writeVarint 直接转 uint64_t——那会占满 10 字节，
     * 且读回时类型容易搞错（end_pointer 初值就是 -1）。
     */
    inline void writeSVarint(std::ostream &out, int64_t value) {
        const auto u = static_cast<uint64_t>(value);
        writeVarint(out, (u << 1) ^ static_cast<uint64_t>(value >> 63));
    }

    inline int64_t readSVarint(std::istream &in) {
        const uint64_t u = readVarint(in);
        return static_cast<int64_t>((u >> 1) ^ (~(u & 1) + 1));
    }

    inline void writeString(std::ostream &out, const std::string &s) {
        writeVarint(out, s.size());
        if (!s.empty()) {
            out.write(s.data(), static_cast<std::streamsize>(s.size()));
        }
    }

    inline std::string readString(std::istream &in) {
        const uint64_t len = readVarint(in);
        if (len > RSI_MAX_STRING_LENGTH) {
            throw std::runtime_error("RSI: string length out of range");
        }
        std::string s(static_cast<size_t>(len), '\0');
        if (len != 0) {
            in.read(s.data(), static_cast<std::streamsize>(len));
            if (!in) {
                throw std::runtime_error("RSI: unexpected EOF while reading string");
            }
        }
        return s;
    }

    /* 带 +1 偏移的无符号编码：把 0xFFFFFFFF（无效下标哨兵）压成 1 字节的 0 */
    inline void writeIndexPlusOne(std::ostream &out, uint32_t index) {
        writeVarint(out, static_cast<uint32_t>(index + 1u));
    }

    inline uint32_t readIndexPlusOne(std::istream &in) {
        // 0 回绕回 0xFFFFFFFF：无符号回绕在 C++ 中有定义
        return static_cast<uint32_t>(readVarint(in)) - 1u;
    }

    /*
     * 路径表。
     *
     * Pos::filepath 与 InsSet::scope_leader_file 不再内联字符串，改存这张表的下标——
     * 同一个文件路径在每条指令、每个参数里都会重复出现，内联是 1.8.0 最大的单项开销。
     *
     * 序列化时必须先写完整张表再写指令流，因为表在指令流写完后才完整。
     */
    struct PathTable {
        std::vector<std::string> paths;
        std::unordered_map<std::string, uint32_t> indexById;

        void clear() {
            paths.clear();
            indexById.clear();
        }

        [[nodiscard]] bool empty() const { return paths.empty(); }
        [[nodiscard]] size_t size() const { return paths.size(); }

        /* 返回已存在路径的下标；新路径则追加。返回下标总是 < paths.size() */
        uint32_t intern(const std::string &path) {
            const auto [it, inserted] =
                    indexById.try_emplace(path, static_cast<uint32_t>(paths.size()));
            if (inserted) {
                paths.push_back(path);
            }
            return it->second;
        }

        [[nodiscard]] const std::string &at(const uint32_t id) const {
            if (id >= paths.size()) {
                throw std::runtime_error("RSI: invalid path table index");
            }
            return paths[static_cast<size_t>(id)];
        }

        void serialize(std::ostream &out) const {
            writeVarint(out, paths.size());
            for (const auto &path: paths) {
                writeString(out, path);
            }
        }

        void deserialize(std::istream &in) {
            clear();
            // 上限校验：path_count 来自文件，直接 reserve 会被畸形输入逼到 OOM
            const uint64_t count = readCount(in, RSI_MAX_PATH_COUNT, "path count");
            /*
             * 直接 push_back 而不建索引：文件里的顺序就是下标，
             * 若表内有重复项，用 intern 会把下标压歪，与写入端不一致。
             */
            paths.reserve(static_cast<size_t>(count));
            for (uint64_t i = 0; i < count; ++i) {
                paths.push_back(readString(in));
            }
        }
    };

} // namespace utils

#endif //RVM_RSI_CODEC_H
