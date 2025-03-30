//
// Created by RestRegular on 2025/3/19.
//

#ifndef RVM_RVM_RIS_H
#define RVM_RVM_RIS_H

#include <iostream>
#include <vector>
#include "ra_core.h"

namespace core::components {
    // 全局变量
    extern std::unordered_map<std::string, RI> insMap;
    extern std::vector<RI *> ri_list;
    extern id::DataID store_SR_id;

    // 枚举类：执行状态
    enum class ExecutionStatus {
        // 执行完成
        Success,                        // 执行完成，且无异常
        FinishedWithError,              // 执行完成，但捕获到异常
        FinishedWithUnknownError,       // 执行完成，但捕获到未知异常

        // 执行失败
        FailedWithError,                // 执行失败，并捕获到异常
        FailedWithoutError,             // 执行失败，但未捕获到异常
        FailedWithUnknownError,         // 执行失败，并捕获到未知异常

        // 执行中断
        Aborted,                        // 正常中断执行
        AbortedWithError,               // 异常中断执行，并捕获到异常
        AbortedWithUnknownError,        // 异常中断执行，并捕获到未知异常

        // 其他
        Unknown,                        // 未知
        NotExecuted,                    // 未执行

        // 添加其他状态
        ExposedError                    // 暴露异常
    };

    // RI类：RVM Instruction指令
    struct RI {
        int ri_index = ri_list.size();
        RIID id = RIID();
        std::string name{};
        int arity{};
        ExecutorFunc executor{};
        bool hasScope{false};
        bool isDelayedReleaseScope{false}; // 是否延迟释放scope

        RI() = default;

        RI(std::string name, int arity,
           ExecutorFunc executor, bool isCreatedScope = false, bool isDelayedReleaseScope = false);

        bool operator==(const RI &other) const;

        std::string toString() const;

        void serialize(std::ostream &os, const utils::SerializationProfile &profile) const;

        static RI *deserialize(std::istream &is, const utils::SerializationProfile &profile);

        static std::optional<RI> getRIByStr(const std::string &content);
    };

    // Ins类：由RA代码解析出来的可执行指令
    struct Ins {
        utils::Pos pos{}; // 调用Pos的序列化函数和反序列化函数
        RI ri; // 调用RI的序列化函数和静态反序列化函数
        InsID insId = core::id::InsID(); // 无需序列化
        StdArgs args; // 调用utils::Arg的序列化函数和反序列化函数，注意vector的序列化
        std::shared_ptr<InsSet> scopeInsSet;
        std::string raw_code;

        bool is_delayed_release_scope{false};

        Ins() = default;

        Ins(utils::Pos pos, std::string raw_code, RI ri, StdArgs args);

        ExecutionStatus execute(int &pointer) const;

        void addIns(std::shared_ptr<Ins> ins) const;

        void serialize(std::ostream &os, const utils::SerializationProfile &profile) const;

        void deserialize(std::istream &is, const utils::SerializationProfile &profile);

    };

    struct DetectBlock {
        std::shared_ptr<data::CustomType> detected_type = nullptr;
        utils::Arg error_stored_arg;
        std::shared_ptr<data::CustomInst> error_data = nullptr;
        std::shared_ptr<InsSet> block_ins_set;

        DetectBlock(std::shared_ptr<data::CustomType> detected_type,
                    utils::Arg error_stored_data,
                    std::shared_ptr<InsSet> block_ins_set);
    };

    struct AtmpBlock {
        std::vector<std::shared_ptr<DetectBlock>> detect_blocks{};
        std::shared_ptr<InsSet> block_ins_set = nullptr;
        std::shared_ptr<InsSet> finally_block_ins_set = nullptr;

        // 命中的detect
        std::shared_ptr<DetectBlock> hit_detect_block = nullptr;

        bool is_hit{false};

        AtmpBlock(std::vector<std::shared_ptr<DetectBlock>> detectBlocks,
                  std::shared_ptr<InsSet> block_ins_set,
                  std::shared_ptr<InsSet> finally_block_ins_set);
    };

    enum class DebugMode {
        None, // 无调试
        Standard, // 标准调试
        NormalSkip, // 标准跳过无标注指令
        FastSkip // 快速跳过无标注指令
    };

    // InsSet 结构体：可执行指令集
    struct InsSet {
        explicit InsSet(std::string prefix) : scope_prefix(std::move(prefix)) {}

        [[nodiscard]] const base::InstID &getInstID() const { return instId; }

        void addIns(std::shared_ptr<Ins> ins);

        void insertInsSet(std::shared_ptr<InsSet> insSet);

        [[nodiscard]] const std::vector<std::shared_ptr<Ins>> &getInsSet() const { return ins_set; }

        void setLabel(const std::string &label, int pointer) { set_labels[label] = pointer; }

        [[nodiscard]] int getLabel(const std::string &label) const;

        void setScopeLeader(const std::string &leader) { scope_leader = leader; }

        void setScopeLeader(const Ins &leaderIns);

        void setScopeLeaderPos(const utils::Pos &leaderPos);

        void setScopeLeaderPos(const std::string &leaderPosStr, const std::string &file_path);

        [[nodiscard]] static std::string makeFileIdentiFromPath(const std::string &pos);

        static void debug_process(const std::shared_ptr<Ins> &ins);

        std::string getTraceInfo(
                const std::string &file_record_, const std::string &error_pos_filepath,
                const std::string &utils_getPosStrFromFilePath, const std::string &makeFileIdentiFromPath,
                const std::string &trace_info, const std::string &error_pos_str,
                const std::string &ins_set_raw_code, const std::string &scope_leader_pos_,
                const std::string &scope_leader_);

        ExecutionStatus execute();

        void serialize(std::ostream &os, const utils::SerializationProfile &profile) const;

        void deserialize(std::istream &is, const utils::SerializationProfile &profile);

        static DebugMode debug_mode; // 无需序列化
        static std::string file_record; // 无需序列化
        static std::stack<std::shared_ptr<InsSet>> exe_stack; // 无需序列化
        static std::vector<std::shared_ptr<AtmpBlock>> atmp_stack; // 无需序列化

        bool is_delayed_release_scope{false};
        int end_pointer{-1};
        std::string scope_prefix;
        std::string scope_leader;
        std::string scope_leader_file;
        std::string scope_leader_pos;
        std::string scope_name;

    private:
        base::InstID instId = base::InstID(); // 无需序列化

        std::vector<std::shared_ptr<Ins>> ins_set{}; // 调用Ins的序列化函数，注意vector的序列化
        mutable std::unordered_map<std::string, int> set_labels{};
    };

}

namespace tools {
    using namespace core;

    std::shared_ptr<base::RVM_Data> getArgOriginDataImpl(const utils::Arg &arg) noexcept;

    std::shared_ptr<base::RVM_Data> getArgOriginData(const utils::Arg &arg);

    std::shared_ptr<base::RVM_Data> getArgNewData(const utils::Arg &arg);

    std::shared_ptr<base::RVM_Data> processQuoteData(const std::shared_ptr<base::RVM_Data> &data);

    bool isNumericData(const std::shared_ptr<base::RVM_Data> &data);

    bool isIterableData(const base::RVM_ID &data_id);

    std::pair<bool, std::string> check_extension_exist(const std::string &extension_path);
}

// 指令集
namespace ris {
    using namespace core::components;
    // 标志指令
    extern const RI S_M;
    extern const RI S_L;
    extern const RI S_F;
    extern const RI S_N;
    extern const RI RL;
    extern const RI RLE;
    extern const RI RNE;
    extern const RI RE;
    extern const RI RSE;
    extern const RI RS;
    extern const RI RT;
    extern const RI RF;
    extern const RI AND; // 逻辑与
    extern const RI OR; // 逻辑或

    // 虚式指令
    extern const RI PASS;
    extern const RI UNKNOWN;
    extern const RI BREAKPOINT;

    // 操作指令

    /* 内存操作指令 */
    extern const RI ALLOT;
    extern const RI DELETE;
    extern const RI PUT;
    extern const RI COPY;
    extern const RI SET;

    /* 算术操作指令 */
    extern const RI ADD;
    extern const RI OPP;
    extern const RI MUL;
    extern const RI DIV;
    extern const RI POW;
    extern const RI ROOT;

    /* 比较操作指令 */
    extern const RI CMP;
    extern const RI CALC_REL;

    /* 控制操作指令 */
    extern const RI END;
    extern const RI EXIT;

    /* 跳转操作指令 */
    extern const RI JMP;
    extern const RI JR;
    extern const RI JT;
    extern const RI JF;

    /* 函数操作指令 */
    extern const RI FUNC;
    extern const RI FUNI;
    extern const RI CALL;
    extern const RI IVOK;
    extern const RI RET;

    /* 循环操作指令 */
    extern const RI REPEAT;
    extern const RI UNTIL;

    /* IO操作指令 */
    extern const RI SOUT;
    extern const RI SIN;

    /* 文件操作指令 */
    extern const RI FILE_GET;
    extern const RI FILE_READ;
    extern const RI FILE_WRITE;
    extern const RI FILE_GET_PATH;
    extern const RI FILE_GET_MODE;
    extern const RI FILE_GET_SIZE;
    extern const RI FILE_SET_MODE;
    extern const RI FILE_SET_PATH;

    /* 类型操作指令 */
    extern const RI TP_GET;
    extern const RI TP_SET;

    /* 自定义类型操作指令 */
    extern const RI TP_DEF;                 // 创建自定义类型 TP_DEF: <custom tp name>, (<parent tp>)
    extern const RI TP_NEW;                 // 创建自定义类型 TP_NEW 实体: <custom tp name>, <stored data>
    extern const RI TP_ADD_INST_FIELD;      // 增加自定义类型实例中的字段 TP_ADD_INST_FIELD: <custom tp inst>, <field name>, (<field value>)
    extern const RI TP_ADD_TP_FIELD;        // 增加自定义类型中的字段 TP_ADD_TP_FIELD: <custom tp name>, <field name>, (<field value>)
    extern const RI TP_SET_FIELD;           // 修改自定义类型实例中的字段值 TP_SET_FIELD: <custom tp inst>, <field name>, <field value>
    extern const RI TP_GET_FIELD;           // 获取自定义类型中的字段值 TP_GET_FIELD: <custom tp name / inst>, <field name>, <stored data>
    extern const RI TP_GET_SUPER_FIELD;     // 获取自定义类型中的字段值 TP_GET_SUPER_FIELD: <custom tp name / inst>, <super tp name>, <field name>, <stored data>
    extern const RI TP_DERIVE;              // 派生自定义类型 TP_DERIVE: <custom tp inst>, <child tp>

    /* 作用域操作指令 */
    extern const RI SP_SET;
    extern const RI SP_GET;
    extern const RI SP_NEW;
    extern const RI SP_DEL;

    /* 引用操作指令 */
    extern const RI QOT;
    extern const RI QOT_VAL;

    /* 异常处理指令 */
    extern const RI EXPOSE;
    extern const RI ATMP;
    extern const RI DETECT;

    /* 迭代器操作指令 */
    extern const RI ITER_APND;      // 向可迭代数据添加数据
    extern const RI ITER_SUB;       // 获取可迭代数据指定索引范围的数据
    extern const RI ITER_SIZE;      // 获取可迭代数据长度
    extern const RI ITER_GET;       // 获取可迭代数据指定索引处的数据
    extern const RI ITER_TRAV;      // 遍历可迭代数据
    extern const RI ITER_REV_TRAV;  // 倒序遍历可迭代数据
    extern const RI ITER_SET;       // 设置可迭代数据指定索引处的数据
    extern const RI ITER_DEL;       // 删除可迭代数据指定索引处的数据
    extern const RI ITER_INSERT;    // 在可迭代数据指定索引处插入数据
    extern const RI ITER_UNPACK;    // 将可迭代数据拆分为多个数据

    /* 随机指令 */
    extern const RI RAND_INT;     // 生成随机整数
    extern const RI RAND_FLOAT;   // 生成随机浮点数

    /* 时间日期操作指令 */
    extern const RI TIME_NOW;     // 获取当前时间戳

    /* 执行内联代码操作指令 */
    extern const RI EXE_RASM;

    /* 模块化编程操作指令 */
    extern const RI LOADIN;       // 动态加载扩展：程序在运行此指令时才会加载扩展

    // 静态链接扩展：可以用于静态链接 RA 文件和 RSI 文件
    // 1. 若直接运行 RA 文件，那么 LINK 指令会在解析阶段直接链接到指定的文件并进行相应的链接操作
    // 2. 若是编译 RA 文件为 RSI 文件，那么 LINK 指令会直接递归的编译所需要链接的文件，并在主文件中生成链接的文件的可执行指令
    extern const RI LINK;

    /* 待完成指令 */
    // TODO: 添加迭代器相关指令
    extern const RI PAIR_NEW;
    extern const RI PAIR_SET_KEY;
    extern const RI PAIR_SET_VALUE;
    extern const RI PAIR_GET_KEY;
    extern const RI PAIR_GET_VALUE;

    // TODO: 添加多线程相关指令
    extern const RI THREAD_NEW;
    extern const RI THREAD_RUN;

    // TODO: 添加调用 dll 文件函数相关指令
    extern const RI DLL_CALL;
    extern const RI DLL_LOADIN;
    extern const RI DLL_LINK;
    extern const RI DLL_UNLOAD;

    // TODO: 添加网络处理相关指令
    // ...
}

// 指令执行函数
namespace exes {
    using namespace core;
    using namespace core::components;

    // 不能造成实际影响的指令执行函数
    ExecutionStatus ri_flag(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_pass(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_unknown(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_breakpoint(const Ins &ins, int &pointer, const StdArgs &args);

    // 可造成实际影响的指令执行函数
    ExecutionStatus ri_allot(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_delete(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_put(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_copy(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_add(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_opp(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_repeat(const Ins &ins, int &pointer, const StdArgs &args); // repeat 指令：循环执行指令集，重复指定次数
    ExecutionStatus ri_end(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_func(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_funi(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_call(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_ivok(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_until(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_exit(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_sout(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_sin(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_get(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_read(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_write(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_get_path(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_get_mode(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_get_size(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_set_mode(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_file_set_path(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_jmp(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_jr(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_jt(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_jf(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_cmp(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_calc_rel(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_mul(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_apnd(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_sub(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_size(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_get(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_trav(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_rev_trav(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_set(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_del(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_insert(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_iter_unpack(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_rand_int(const Ins &check_arg, int &pointer, const StdArgs &args);
    ExecutionStatus ri_rand_float(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_loadin(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_link(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_exe_rasm(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_get(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_set(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_div(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_pow(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_root(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_def(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_new(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_add_tp_field(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_add_inst_field(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_set_field(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_get_field(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_get_super_field(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_tp_derive(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_sp_set(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_sp_get(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_sp_new(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_sp_del(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_qot(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_qot_val(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_atmp(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_expose(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_ret(const Ins &ins, int &pointer, const StdArgs &args);
}


#endif //RVM_RVM_RIS_H
