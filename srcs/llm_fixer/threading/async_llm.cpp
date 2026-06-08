#include <string>
#include <iostream>

#include "async_queue.cpp"
#include "../factory/llm_factory.cpp"

LLM_Factory* MODEL = nullptr;
std::string DIALECT = "PostgreSQL";

void llm_worker()
{
    // std::cout << "\n\n=== STARTED LLM THREAD ===\n\n";

    while (true)
    {
        std::string request = requests.wait_and_pop();

        if (request == "QUIT")
            break;

        std::string result = MODEL->fix_sql(request, "", DIALECT);
        // std::cout << "\n\n=== Fixed SQL ===\n\n" << result << "\n=== " << requests.size() << " LEFT ===\n\n";

        responses.push(result);
    }
    
    // std::cout << "\n\n=== FINISHED LLM THREAD ===\n\n";
}