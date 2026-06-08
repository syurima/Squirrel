#pragma once

#include <string>
#include <llama.h>

class LLM_Factory {
    public:
        virtual ~LLM_Factory() = default;
        virtual std::string fix_sql(const std::string& sql_query,
                                const std::string& error_message,
                                const std::string& dialect) {return "";};

    private:
        LlamaModel* model = nullptr;

        virtual std::string get_prompt(const std::string& sql_query,
                                const std::string& error_message,
                                const std::string& dialect) {return "";};
};