#ifndef __MUTATOR_H__
#define __MUTATOR_H__

#include <map>      //?
#include <set>      //?
#include <utility>  //?

#include "ast.h"
#include "define.h"
#include "utils.h"

#define LUCKY_NUMBER 500

using namespace std;

enum RELATIONTYPE {
  kRelationElement,
  kRelationSubtype,
  kRelationAlias,
};  //?

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

  void init(const string &f_testcase = "", const string &f_common_string = "",
            const string &file2d = "", const string &file1d = "",
            const string &f_gen_type = "");  //?

  void init_ir_library(const string &filename);
  void init_value_library();
  void init_data_library(const string &filename);        //?
  void init_data_library_2d(const string &filename);     //?
  void init_not_mutatable_type(const string &filename);  //?
  void init_mutationmap();                               //?
  void add_ir_to_library(IR *);

  IR *get_ir_from_library(IRTYPE);

  void extract_struct(IR *, bool use_unique_names = false);  //?

  bool fix(IR *root);  //?

  vector<IR *> split_to_stmt(IR *root, map<IR **, IR *> &m_save,
                             set<IRTYPE> &split_set);  //?
  bool connect_back(map<IR **, IR *> &m_save);         //?

  bool fix_one(IR *root);  //?

  void analyze_scope(IR *root);                                 //?
  map<IR *, vector<IR *>> build_dependency_graph(IR *root);                //?
  bool fill_stmt_graph(map<IR *, vector<IR *>> &graph);              //?
  bool fill_stmt_graph_one(map<IR *, vector<IR *>> &graph, IR *ir);  //?

  IR *find_closest_node(IR *root, IR *node, DATATYPE type);     //?
  bool fill_one(IR *parent);                                         //?
  bool fill_one_pair(IR *parent, IR *child);                         //?
  bool validate(IR *&root);                                          //?

  unsigned int calc_node(IR *root);
  bool replace_one_value_from_datalibray_2d(DATATYPE p_datatype,
                                            DATATYPE c_data_type,
                                            const string &p_key,
                                            const string &old_c_value,
                                            const string &new_c_value);  //?
  bool remove_one_pair_from_datalibrary_2d(DATATYPE p_datatype,
                                           DATATYPE c_data_type,
                                           const string &p_key);  //?
  bool remove_one_from_datalibrary(DATATYPE datatype, const string &key);  //?
  ~Mutator();

  int try_fix(char *buf, int len, char *&new_buf, int &new_len);

  void add_ir_to_library_no_deepcopy(IR *);

  IR *record_ = NULL;
  map<IRTYPE, vector<IR *>> ir_library_;
  map<IRTYPE, set<unsigned long>> ir_library_hash_;

  map<IRTYPE, vector<IR *>> left_lib;
  map<IRTYPE, vector<IR *>> right_lib;

  // vector<string> string_library_;
  // set<unsigned long> string_library_hash_;
  vector<unsigned long> value_library_;

  map<DATATYPE, map<DATATYPE, RELATIONTYPE>> relationmap_;  //?

  set<IRTYPE> not_mutatable_types_;  //?
  set<IRTYPE> string_types_;
  set<IRTYPE> int_types_;
  set<IRTYPE> float_types_;

  set<IRTYPE> split_stmt_types_;
  set<IRTYPE> split_substmt_types_;

  map<DATATYPE, vector<string>> data_library_;  //?
  map<DATATYPE, map<string, map<DATATYPE, vector<string>>>>
      data_library_2d_;  //?

  map<DATATYPE, vector<string>> g_data_library_;  //?
  map<DATATYPE, map<string, map<DATATYPE, vector<string>>>>
      g_data_library_2d_;  //?

  map<int, map<DATATYPE, vector<IR *>>> scope_library_;  //?

  set<unsigned long> global_hash_;
};

#endif
