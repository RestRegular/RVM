//
// Created by RestRegular on 2025/1/16.
//

#ifndef RA_PARSER_H
#define RA_PARSER_H

#include <iostream>
#include "ra_base.h"
#include "ra_core.h"
#include "rvm_ris.h"
#include "lib/ra_utils.h"

namespace parser {
    using namespace core::components;

    struct InsSetPool;

    extern core::memory::RVM_Memory &data_space_pool;
    extern base::RVM_IO* io;
    extern InsSetPool &ins_set_pool;
    extern utils::StringManager &string_manager;
    using StdInsSet = std::shared_ptr<InsSet>;

    struct InsSetPool {
    public:
        static InsSetPool& getInstance();

        InsSetPool (const InsSetPool&) = delete;
        InsSetPool& operator= (const InsSetPool&) = delete;

        StdInsSet acquire(const std::string &prefix);

        StdInsSet getAppointedInsSet(const std::string& insSetId);

        void release(const std::string& insSetId);

        void release(const StdInsSet& insSet);

        void releaseAll();

        std::shared_ptr<Ins> findIns(const std::string& insSetId, const std::string& insId) const;

        void printInfo(std::ostream& out = std::cout) const;

    private:
        InsSetPool() = default;

        std::unordered_map<std::string, StdInsSet> insSetSpace_;
        std::list<StdInsSet> insSetOrder_;
        std::vector<StdInsSet> insSetPool_;
    };

    namespace components {
        extern StdInsSet INS_SET;
    }

    namespace env {
        using namespace core::components;

        extern utils::SerializationProfile program_serialization_profile_;

        extern bool precompiled_link_option;

        extern std::string precompiled_link_dir_path;

        void initialize(const std::string &executed_file_path, const std::string &working_directory);

        void addIns(const utils::Pos &pos, const std::string& raw_code, const RI *ri, const StdArgs &args);

        void serializeLinkedExtensions(std::ostream &out);

        void deserializeLinkedExtensions(std::istream &in);
    }

    namespace parse {
        using namespace core::components;

        enum class ParseType {
            File,
            Rasm,
            Extension,
        };

        std::tuple<std::vector<std::string>, std::vector<size_t>, std::vector<size_t>> preprocessCode(std::string &code);

        std::shared_ptr<InsSet>
        parseCode(const ParseType &type, std::string code_path, std::string &code, bool output_info,
                  const std::optional<utils::Pos> &fixed_pos, const std::string &ext = "");

        std::shared_ptr<InsSet> parseCodeFromPath(const std::string &code_file_path, bool output_info,
                                                  const std::string &extension_name = "");

        void serializeExecutableInsToBinaryFile(std::string binary_file_path, const std::shared_ptr<InsSet> &ins_set, const utils::SerializationProfile &profile);

        std::shared_ptr<InsSet> deserializeExecutableInsFromBinaryFile(std::string binary_file_path,
                                                                       const utils::SerializationProfile &profile = utils::SerializationProfile::Debug);

        std::shared_ptr<InsSet> getLinkedInsSet(const Ins &link_ins, bool check_exist = true, const std::string &path = "");
    }

}

#endif //RA_PARSER_H
