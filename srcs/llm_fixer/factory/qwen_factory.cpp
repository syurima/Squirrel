#include <string>

#include "qwen_factory.h"

Qwen_Factory::Qwen_Factory() {
    this->model = new LlamaModel(std::string(MODELS_PATH) + "/qwen2.5-coder-1.5b-instruct-q4_0.gguf");
}

Qwen_Factory::~Qwen_Factory() {
    delete this->model;
    this->model = nullptr;
}

std::string Qwen_Factory::fix_sql(const std::string& sql_query,
                                    const std::string& error_message,
                                    const std::string& dialect) {
    return this->model->generate(
        this->get_prompt(sql_query, error_message, dialect),
        128,
        0.0f,
        0.95f
        // {
        //     "<|*|>*", "<|*", ";*"
        // }
    );
}

std::string Qwen_Factory::get_prompt(const std::string& sql_query,
                                    const std::string& error_message,
                                    const std::string& dialect) {
    return
        "<|im_start|>system\n"
        "You are a SQL repair engine.\n"
        "Output ONLY the corrected SQL query.\n"
        "Do NOT explain.\n"
        "Do NOT format.\n"
        "Do NOT use markdown.\n"
        "Do NOT output comments.\n"
        "Do NOT output any text before or after the SQL.\n"
        "Do NOT output <|im_start|>, <|im_end|>, or any special tokens.\n"
        "Stop immediately after the final SQL token.\n"
        "<|im_end|>\n\n"
        "<|im_start|>user\n"
        "Fix this " + dialect + " SQL query.\n\n" +
        sql_query +
        "\n\nError:\n" +
        (error_message.empty() ? "Unknown" : error_message) +
        "\n<|im_end|>\n\n"
        "<|im_start|>assistant\n";
}