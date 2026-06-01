#ifndef __MUTATOR_H__
#define __MUTATOR_H__

#include "ast.h" //?
#include "define.h" //?
#include "utils.h" //?

#define LUCKY_NUMBER 500

using std::string; //?
using std::vector; //?
using std::map; //?
using std::set; //?

enum class MutationKind {
    Delete,
    Insert,
    Replace
}; //?

struct MutationWeights {
    int delete_weight = 20;
    int insert_weight = 40;
    int replace_weight = 40;
}; //?

struct MutationStats {
  unsigned long used = 0;
  unsigned long success = 0;
  unsigned long failed = 0;
}; //?

class Mutator {
 public:
  Mutator() {}

  IR *deep_copy_with_record(const IR *root, const IR *record);
  unsigned long hash(IR *);
  unsigned long hash(const string &);

  vector<IR *> mutate_all(vector<IR *> &v_ir_collector);
  vector<IR *> mutate(IR *input);
  IR *strategy_delete(IR *cur);
  IR *strategy_insert(IR *cur);
  IR *strategy_replace(IR *cur);
  bool lucky_enough_to_be_mutated(unsigned int mutated_times);

  bool replace(IR *root, IR *old_ir, IR *new_ir);
  IR *locate_parent(IR *root, IR *old_ir);

  void init(const string &f_testcase, const string &f_common_string = "", const string &pragma = ""); //?

  void init_ir_library(const string &filename);
  void init_value_library();
  void init_common_string(const string &f_common_string);
  void init_string_library(); //?
  void init_pragma(const string &pragma); //?
  void init_relationmap(); //?
  void init_tables(); //?


  void add_ir_to_library(IR *);
  void add_ir_to_library_no_deepcopy(IR *);
  IR *get_ir_from_library(IRTYPE);

  string fix(IR *root); //?
  string validate(IR *root); //?
  string extract_struct(IR *root, bool use_unique_names = false);
  void add_new_table(IR *root, string &table_name); //?
  void reset_database(); //?

  void minimize(vector<IR *> &);
  int try_fix(char *buf, int len, char *&new_buf, int &new_len);

  bool check_node_num(IR *root, unsigned int limit);
  vector<IR *> extract_statement(IR *root);

  map<IR *, set<IR *>> build_dependency_graph(IR *root,
                                              map<IDTYPE, IDTYPE> &relationmap,
                                              map<IDTYPE, IDTYPE> &crssmap,
                                              vector<IR *> &ordered_ir); //?
  vector<IR *> cut_subquery(IR *program, map<IR **, IR *> &m_save); //?
  bool fix_back(map<IR **, IR *> &m_save); //?
  void fix_one(IR *fixed_key, map<IR *, set<IR *>> &graph, set<IR *> &visited); //?
  void fix_graph(map<IR *, set<IR *>> &graph, IR *root,
                 vector<IR *> &ordered_ir); //?
  unsigned int calc_node(IR *root);

  static map<string, vector<string>> m_tables; //?
  static vector<string> v_table_names; //?
  ~Mutator();

 private:
  IR *record_ = NULL;
  map<IRTYPE, vector<IR *>> ir_library_;
  map<IRTYPE, set<unsigned long>> ir_library_hash_;
  map<IRTYPE, vector<IR *>> left_lib; //?
  map<IRTYPE, vector<IR *>> right_lib; //?
  map<IDTYPE, IDTYPE> relationmap; //?
  map<IDTYPE, IDTYPE> cross_map; //?

  vector<string> string_library_;
  set<unsigned long> string_library_hash_;
  vector<unsigned long> value_library_;
  vector<string> common_string_library_;

  vector<string> cmds_; //?
  map<string, vector<string>> m_cmd_value_lib_; //?

  string s_table_name; //?

  map<IRTYPE, int> type_counter_; //?

  MutationWeights base_weights_; //?
  MutationStats delete_stats_; //?
  MutationStats insert_stats_; //?
  MutationStats replace_stats_; //?
  
  MutationWeights get_seed_adaptive_weights(IR *input); //?
  MutationWeights get_feedback_adaptive_weights(const MutationWeights &seed_weights); //?
  MutationKind choose_mutation_kind(const MutationWeights &weights); //?
  void update_mutation_stats(MutationKind kind, IR *result); //?
};

#endif
