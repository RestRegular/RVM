//
// Created by RestRegular on 2025/3/19.
//

#ifndef RVM_RVM_RIS_H
#define RVM_RVM_RIS_H

#include <iostream>
#include <vector>
#include "ra_core.h"

namespace core::components {
    // ȫ�ֱ���
    extern std::unordered_map<std::string, RI> insMap;
    extern std::vector<RI *> ri_list;
    extern id::DataID store_SR_id;

    // ö���ִࣺ��״̬
    enum class ExecutionStatus {
        // ִ�����
        Success,                        // ִ����ɣ������쳣
        FinishedWithError,              // ִ����ɣ��������쳣
        FinishedWithUnknownError,       // ִ����ɣ�������δ֪�쳣

        // ִ��ʧ��
        FailedWithError,                // ִ��ʧ�ܣ��������쳣
        FailedWithoutError,             // ִ��ʧ�ܣ���δ�����쳣
        FailedWithUnknownError,         // ִ��ʧ�ܣ�������δ֪�쳣

        // ִ���ж�
        Aborted,                        // �����ж�ִ��
        AbortedWithError,               // �쳣�ж�ִ�У��������쳣
        AbortedWithUnknownError,        // �쳣�ж�ִ�У�������δ֪�쳣

        // ����
        Unknown,                        // δ֪
        NotExecuted,                    // δִ��

        // �������״̬
        ExposedError                    // ��¶�쳣
    };

    // RI�ࣺRVM Instructionָ��
    struct RI {
        int ri_index = ri_list.size();
        RIID id = RIID();
        std::string name{};
        int arity{};
        ExecutorFunc executor{};
        bool hasScope{false};
        bool isDelayedReleaseScope{false}; // �Ƿ��ӳ��ͷ�scope

        RI() = default;

        RI(std::string name, int arity,
           ExecutorFunc executor, bool isCreatedScope = false, bool isDelayedReleaseScope = false);

        bool operator==(const RI &other) const;

        std::string toString() const;

        void serialize(std::ostream &os, const utils::SerializationProfile &profile) const;

        static RI *deserialize(std::istream &is, const utils::SerializationProfile &profile);

        static std::optional<RI> getRIByStr(const std::string &content);
    };

    // Ins�ࣺ��RA������������Ŀ�ִ��ָ��
    struct Ins {
        utils::Pos pos{}; // ����Pos�����л������ͷ����л�����
        RI ri; // ����RI�����л������;�̬�����л�����
        InsID insId = core::id::InsID(); // �������л�
        StdArgs args; // ����utils::Arg�����л������ͷ����л�������ע��vector�����л�
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

        // ���е�detect
        std::shared_ptr<DetectBlock> hit_detect_block = nullptr;

        bool is_hit{false};

        AtmpBlock(std::vector<std::shared_ptr<DetectBlock>> detectBlocks,
                  std::shared_ptr<InsSet> block_ins_set,
                  std::shared_ptr<InsSet> finally_block_ins_set);
    };

    enum class DebugMode {
        None, // �޵���
        Standard, // ��׼����
        NormalSkip, // ��׼�����ޱ�עָ��
        FastSkip // ���������ޱ�עָ��
    };

    // InsSet �ṹ�壺��ִ��ָ�
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

        static DebugMode debug_mode; // �������л�
        static std::string file_record; // �������л�
        static std::stack<std::shared_ptr<InsSet>> exe_stack; // �������л�
        static std::vector<std::shared_ptr<AtmpBlock>> atmp_stack; // �������л�

        bool is_delayed_release_scope{false};
        int end_pointer{-1};
        std::string scope_prefix;
        std::string scope_leader;
        std::string scope_leader_file;
        std::string scope_leader_pos;
        std::string scope_name;

    private:
        base::InstID instId = base::InstID(); // �������л�

        std::vector<std::shared_ptr<Ins>> ins_set{}; // ����Ins�����л�������ע��vector�����л�
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

// ָ�
namespace ris {
    using namespace core::components;
    // ��־ָ��
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
    extern const RI AND; // �߼���
    extern const RI OR; // �߼���

    // ��ʽָ��
    extern const RI PASS;
    extern const RI UNKNOWN;
    extern const RI BREAKPOINT;

    // ����ָ��

    /* �ڴ����ָ�� */
    extern const RI ALLOT;
    extern const RI DELETE;
    extern const RI PUT;
    extern const RI COPY;
    extern const RI SET;

    /* ��������ָ�� */
    extern const RI ADD;
    extern const RI OPP;
    extern const RI MUL;
    extern const RI DIV;
    extern const RI POW;
    extern const RI ROOT;

    /* �Ƚϲ���ָ�� */
    extern const RI CMP;
    extern const RI CALC_REL;

    /* ���Ʋ���ָ�� */
    extern const RI END;
    extern const RI EXIT;

    /* ��ת����ָ�� */
    extern const RI JMP;
    extern const RI JR;
    extern const RI JT;
    extern const RI JF;

    /* ��������ָ�� */
    extern const RI FUNC;
    extern const RI FUNI;
    extern const RI CALL;
    extern const RI IVOK;
    extern const RI RET;

    /* ѭ������ָ�� */
    extern const RI REPEAT;
    extern const RI UNTIL;

    /* IO����ָ�� */
    extern const RI SOUT;
    extern const RI SIN;

    /* �ļ�����ָ�� */
    extern const RI FILE_GET;
    extern const RI FILE_READ;
    extern const RI FILE_WRITE;
    extern const RI FILE_GET_PATH;
    extern const RI FILE_GET_MODE;
    extern const RI FILE_GET_SIZE;
    extern const RI FILE_SET_MODE;
    extern const RI FILE_SET_PATH;

    /* ���Ͳ���ָ�� */
    extern const RI TP_GET;
    extern const RI TP_SET;

    /* �Զ������Ͳ���ָ�� */
    extern const RI TP_DEF;                 // �����Զ������� TP_DEF: <custom tp name>, (<parent tp>)
    extern const RI TP_NEW;                 // �����Զ������� TP_NEW ʵ��: <custom tp name>, <stored data>
    extern const RI TP_ADD_INST_FIELD;      // �����Զ�������ʵ���е��ֶ� TP_ADD_INST_FIELD: <custom tp inst>, <field name>, (<field value>)
    extern const RI TP_ADD_TP_FIELD;        // �����Զ��������е��ֶ� TP_ADD_TP_FIELD: <custom tp name>, <field name>, (<field value>)
    extern const RI TP_SET_FIELD;           // �޸��Զ�������ʵ���е��ֶ�ֵ TP_SET_FIELD: <custom tp inst>, <field name>, <field value>
    extern const RI TP_GET_FIELD;           // ��ȡ�Զ��������е��ֶ�ֵ TP_GET_FIELD: <custom tp name / inst>, <field name>, <stored data>
    extern const RI TP_GET_SUPER_FIELD;     // ��ȡ�Զ��������е��ֶ�ֵ TP_GET_SUPER_FIELD: <custom tp name / inst>, <super tp name>, <field name>, <stored data>
    extern const RI TP_DERIVE;              // �����Զ������� TP_DERIVE: <custom tp inst>, <child tp>

    /* ���������ָ�� */
    extern const RI SP_SET;
    extern const RI SP_GET;
    extern const RI SP_NEW;
    extern const RI SP_DEL;

    /* ���ò���ָ�� */
    extern const RI QOT;
    extern const RI QOT_VAL;

    /* �쳣����ָ�� */
    extern const RI EXPOSE;
    extern const RI ATMP;
    extern const RI DETECT;

    /* ����������ָ�� */
    extern const RI ITER_APND;      // ��ɵ��������������
    extern const RI ITER_SUB;       // ��ȡ�ɵ�������ָ��������Χ������
    extern const RI ITER_SIZE;      // ��ȡ�ɵ������ݳ���
    extern const RI ITER_GET;       // ��ȡ�ɵ�������ָ��������������
    extern const RI ITER_TRAV;      // �����ɵ�������
    extern const RI ITER_REV_TRAV;  // ��������ɵ�������
    extern const RI ITER_SET;       // ���ÿɵ�������ָ��������������
    extern const RI ITER_DEL;       // ɾ���ɵ�������ָ��������������
    extern const RI ITER_INSERT;    // �ڿɵ�������ָ����������������
    extern const RI ITER_UNPACK;    // ���ɵ������ݲ��Ϊ�������

    /* ���ָ�� */
    extern const RI RAND_INT;     // �����������
    extern const RI RAND_FLOAT;   // �������������

    /* ʱ�����ڲ���ָ�� */
    extern const RI TIME_NOW;     // ��ȡ��ǰʱ���

    /* ִ�������������ָ�� */
    extern const RI EXE_RASM;

    /* ģ�黯��̲���ָ�� */
    extern const RI LOADIN;       // ��̬������չ�����������д�ָ��ʱ�Ż������չ

    // ��̬������չ���������ھ�̬���� RA �ļ��� RSI �ļ�
    // 1. ��ֱ������ RA �ļ�����ô LINK ָ����ڽ����׶�ֱ�����ӵ�ָ�����ļ���������Ӧ�����Ӳ���
    // 2. ���Ǳ��� RA �ļ�Ϊ RSI �ļ�����ô LINK ָ���ֱ�ӵݹ�ı�������Ҫ���ӵ��ļ����������ļ����������ӵ��ļ��Ŀ�ִ��ָ��
    extern const RI LINK;

    /* �����ָ�� */
    // TODO: ��ӵ��������ָ��
    extern const RI PAIR_NEW;
    extern const RI PAIR_SET_KEY;
    extern const RI PAIR_SET_VALUE;
    extern const RI PAIR_GET_KEY;
    extern const RI PAIR_GET_VALUE;

    // TODO: ��Ӷ��߳����ָ��
    extern const RI THREAD_NEW;
    extern const RI THREAD_RUN;

    // TODO: ��ӵ��� dll �ļ��������ָ��
    extern const RI DLL_CALL;
    extern const RI DLL_LOADIN;
    extern const RI DLL_LINK;
    extern const RI DLL_UNLOAD;

    // TODO: ������紦�����ָ��
    // ...
}

// ָ��ִ�к���
namespace exes {
    using namespace core;
    using namespace core::components;

    // �������ʵ��Ӱ���ָ��ִ�к���
    ExecutionStatus ri_flag(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_pass(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_unknown(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_breakpoint(const Ins &ins, int &pointer, const StdArgs &args);

    // �����ʵ��Ӱ���ָ��ִ�к���
    ExecutionStatus ri_allot(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_delete(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_put(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_copy(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_add(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_opp(const Ins &ins, int &pointer, const StdArgs &args);
    ExecutionStatus ri_repeat(const Ins &ins, int &pointer, const StdArgs &args); // repeat ָ�ѭ��ִ��ָ����ظ�ָ������
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
