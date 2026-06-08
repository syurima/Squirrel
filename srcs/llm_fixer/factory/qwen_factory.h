#pragma once

#include "llm_factory.cpp"

class Qwen_Factory : public LLM_Factory {
    public:
        Qwen_Factory();
        ~Qwen_Factory();
        std::string fix_sql(const std::string& sql_query,
                                const std::string& error_message = "",
                                const std::string& dialect = "PostgreSQL");
    
    private:
        LlamaModel* model = nullptr;

        std::string get_prompt(const std::string& sql_query,
                                const std::string& error_message = "",
                                const std::string& dialect = "PostgreSQL");
};