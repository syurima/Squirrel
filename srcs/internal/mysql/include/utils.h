#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <vector>

#include "../parser/bison_parser.h"
#include "../parser/flex_lexer.h"
using std::string;
using std::vector;

#include "../common/include/mutator_helpers.h"

#define get_rand_int(range) rand() % (range)

void trim_string(string &);

string gen_string();

double gen_float();

long gen_long();

int gen_int();

uint64_t ducking_hash(const void *key, int len);
vector<string> get_all_files_in_dir(const char *dir_name);
Program *parser(string sql);
#endif
