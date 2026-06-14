#include <string>
#include <vector>
#include <regex>

#include <llama.h>

// ============================================================
// Llama Wrapper
// ============================================================

class LlamaModel {
public:
    LlamaModel(
        const std::string& model_path,
        int n_ctx = 4096,
        int n_threads = 8,
        int n_gpu_layers = 0)
    {
        llama_backend_init();

        llama_model_params model_params = llama_model_default_params();
        model_params.n_gpu_layers = n_gpu_layers;

        model = llama_model_load_from_file(
            model_path.c_str(),
            model_params
        );

        if (!model) {
            throw std::runtime_error("Failed to load model");
        }

        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = n_ctx;
        ctx_params.n_threads = n_threads;

        ctx = llama_init_from_model(model, ctx_params);

        if (!ctx) {
            throw std::runtime_error("Failed to create context");
        }
    }

    ~LlamaModel() {
        if (ctx)
            llama_free(ctx);

        if (model)
            llama_model_free(model);

        llama_backend_free();
    }

    std::string generate(
        const std::string& prompt,
        int max_tokens = 128,
        float temperature = 0.0f,
        float top_p = 0.95f,
        const std::vector<std::string>& stop_sequences = {";*", "<|*"})
    {
        const llama_vocab* vocab = llama_model_get_vocab(model);

        // Tokenize
        std::vector<llama_token> tokens(prompt.size() + 128);

        int n_tokens = llama_tokenize(
            vocab,
            prompt.c_str(),
            prompt.size(),
            tokens.data(),
            tokens.size(),
            true,
            false
        );

        tokens.resize(n_tokens);

        // Evaluate prompt
        llama_batch batch = llama_batch_get_one(
            tokens.data(),
            tokens.size()
        );

        if (llama_decode(ctx, batch) != 0) {
            throw std::runtime_error("llama_decode failed");
        }

        std::string result;

        // New sampler API
        llama_sampler* sampler = llama_sampler_chain_init(
            llama_sampler_chain_default_params()
        );

        if (temperature <= 0.0f) {

            llama_sampler_chain_add(
                sampler,
                llama_sampler_init_greedy()
            );

        } else {

            llama_sampler_chain_add(
                sampler,
                llama_sampler_init_top_k(40)
            );

            llama_sampler_chain_add(
                sampler,
                llama_sampler_init_top_p(top_p, 1)
            );

            llama_sampler_chain_add(
                sampler,
                llama_sampler_init_temp(temperature)
            );

            llama_sampler_chain_add(
                sampler,
                llama_sampler_init_dist(1234)
            );
        }

        for (int i = 0; i < max_tokens; ++i) {

            llama_token new_token =
                llama_sampler_sample(sampler, ctx, -1);

            llama_sampler_accept(sampler, new_token);

            if (new_token == llama_vocab_eos(vocab)) {
                break;
            }

            char buffer[256];

            int length = llama_token_to_piece(
                vocab,
                new_token,
                buffer,
                sizeof(buffer),
                0,
                true
            );

            result.append(buffer, length);

            // Check stop sequences
            bool should_stop = false;

            for (const auto& stop : stop_sequences) {
                if (ends_with_pattern(result, stop)) {

                    // Remove the stop text from output
                    // result.erase(result.size() - stop.size());

                    should_stop = true;
                    break;
                }
            }

            if (should_stop) {
                break;
            }

            llama_batch next_batch =
                llama_batch_get_one(&new_token, 1);

            if (llama_decode(ctx, next_batch) != 0) {
                break;
            }
        }

        llama_sampler_free(sampler);

        return trim_end(result);
    }

private:
    llama_model* model = nullptr;
    llama_context* ctx = nullptr;

    static bool ends_with_pattern(
        const std::string& text,
        const std::string& pattern)
    {
        // Fast path: exact match
        if (pattern.find('*') == std::string::npos) {

            return text.size() >= pattern.size() &&
                text.compare(
                    text.size() - pattern.size(),
                    pattern.size(),
                    pattern
                ) == 0;
        }

        // Convert wildcard pattern to regex
        std::string regex_pattern;

        for (char c : pattern) {

            if (c == '*') {

                regex_pattern += ".*";

            } else {

                // Escape regex special chars
                if (std::string(R"(\.^$|()[]{}+?)").find(c)
                    != std::string::npos)
                {
                    regex_pattern += '\\';
                }

                regex_pattern += c;
            }
        }

        // Match only at end of string
        regex_pattern += "$";

        return std::regex_search(
            text,
            std::regex(regex_pattern)
        );
    }

    static std::string trim_end(std::string& result){
        int n = result.size();
        while(n > 0){
            if(result[n-1] == ';'){
                break;
            }
            if(n > 1 && result[n-1] == '|' && result[n-2] == '<'){
                result.resize(n-2);
                n -= 2;
                while(result[n-1] == '<'){
                    n -= 1;
                    result.resize(n);
                }
                break;
            }
            n -= 1;
            result.resize(n);
        }
        return result;
    }
};