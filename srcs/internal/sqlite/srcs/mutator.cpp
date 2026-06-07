#include "../include/mutator.h"

#include <assert.h>

#include <algorithm>
#include <cfloat>
#include <climits>
#include <cstdio>
#include <deque>
#include <fstream>

#include "../../common/include/mutator_helpers.h"
#include "../include/ast.h"
#include "../include/define.h"
#include "../include/utils.h"

using namespace std;
using mutator_common::pick_random_element;
using mutator_common::pick_random_or;

map<string, vector<string>> Mutator::m_tables;
vector<string> Mutator::v_table_names;

IR *Mutator::deep_copy_with_record(const IR *root, const IR *record) {
  IR *left = NULL, *right = NULL, *copy_res;

  if (root->left_) left = deep_copy_with_record(root->left_, record);
  if (root->right_) right = deep_copy_with_record(root->right_, record);

  if (root->op_ != NULL)
    copy_res =
        new IR(root->type_,
               OP3(root->op_->prefix_, root->op_->middle_, root->op_->suffix_),
               left, right, root->float_val_, root->str_val_, root->name_,
               root->mutated_times_);
  else
    copy_res = new IR(root->type_, NULL, left, right, root->float_val_,
                      root->str_val_, root->name_, root->mutated_times_);

  copy_res->id_type_ = root->id_type_;

  if (root == record && record != NULL) {
    this->record_ = copy_res;
  }

  return copy_res;
}

bool Mutator::check_node_num(IR *root, unsigned int limit) {
  auto v_statements = extract_statement(root);
  bool is_good = true;

  if (v_statements.size() > 5) {
    is_good = false;

  } else
    for (auto stmt : v_statements) {
      if (calc_node(stmt) > limit) {
        is_good = false;
        break;
      }
    }

  return is_good;
}

vector<IR *> Mutator::mutate_all(vector<IR *> &v_ir_collector) {
  vector<IR *> res;
  if (v_ir_collector.empty()) return res;
  IR *root = v_ir_collector[v_ir_collector.size() - 1];

  for (auto ir : v_ir_collector) {
    if (ir == root || ir->type_ == kProgram) continue;
    vector<IR *> v_mutated_ir = mutate(ir);

    for (auto i : v_mutated_ir) {
      IR *new_ir_tree = deep_copy_with_record(root, ir);
      replace(new_ir_tree, this->record_, i);

      if (!check_node_num(new_ir_tree, 100)) {
        deep_delete(new_ir_tree);
        continue;
      }  // TODO: remove or add for common

      extract_struct(new_ir_tree);
      string tmp = new_ir_tree->to_string();
      unsigned tmp_hash = hash(tmp);
      if (global_hash_.find(tmp_hash) != global_hash_.end()) {
        deep_delete(new_ir_tree);
        continue;
      }

      global_hash_.insert(tmp_hash);
      res.push_back(new_ir_tree);
    }
  }

  return res;
}

void Mutator::init_ir_library(const string &filename) {
  ifstream input_file(filename);
  if (!input_file.is_open()) {
    cerr << "[!] failed to open ir_library file: " << filename << endl;
    return;
  }
  string line;

  cout << "[*] init ir_library: " << filename << endl;
  while (getline(input_file, line)) {
    if (line.empty()) continue;
    auto p = parser(line);
    if (p == nullptr) continue;

    vector<IR *> v_ir;
    auto res = p->translate(v_ir);
    p->deep_delete();
    p = nullptr;

    add_ir_to_library(res);
    deep_delete(res);
  }
  return;
}

void Mutator::init_tables() {
  vector<string> v_tmp = {"haha1", "haha2", "haha3"};
  v_table_names.insert(v_table_names.end(), v_tmp.begin(), v_tmp.end());

  m_tables["haha1"] = {"ducking_column0_1", "ducking_column1_1",
                       "ducking_column2_1"};
  m_tables["haha2"] = {"ducking_column0_2", "ducking_column1_2",
                       "ducking_column2_2"};
  m_tables["haha3"] = {"ducking_column0_3", "ducking_column1_3",
                       "ducking_column2_3"};
}

void Mutator::init_value_library() {
  vector<unsigned long> value_lib_init = {0,
                                          (unsigned long)LONG_MAX,
                                          (unsigned long)ULONG_MAX,
                                          (unsigned long)CHAR_BIT,
                                          (unsigned long)SCHAR_MIN,
                                          (unsigned long)SCHAR_MAX,
                                          (unsigned long)UCHAR_MAX,
                                          (unsigned long)CHAR_MIN,
                                          (unsigned long)CHAR_MAX,
                                          (unsigned long)MB_LEN_MAX,
                                          (unsigned long)SHRT_MIN,
                                          (unsigned long)INT_MIN,
                                          (unsigned long)INT_MAX,
                                          (unsigned long)SCHAR_MIN,
                                          (unsigned long)SCHAR_MIN,
                                          (unsigned long)UINT_MAX,
                                          (unsigned long)FLT_MAX,
                                          (unsigned long)DBL_MAX,
                                          (unsigned long)LDBL_MAX,
                                          (unsigned long)FLT_MIN,
                                          (unsigned long)DBL_MIN,
                                          (unsigned long)LDBL_MIN};

  value_library_.insert(value_library_.begin(), value_lib_init.begin(),
                        value_lib_init.end());
}

void Mutator::init_string_library() {
  string_library_.push_back("x");
  string_library_.push_back("v0");
  string_library_.push_back("v1");
}

void Mutator::init_pragma(const string &pragma) {
  ifstream input_pragma(pragma);
  assert(input_pragma.is_open());

  string s;
  cout << "[duck]start init pragma" << endl;
  while (getline(input_pragma, s)) {
    if (s.empty()) continue;

    auto pos = s.find('=');
    if (pos == string::npos) continue;

    string k = s.substr(0, pos - 1);
    string v = s.substr(pos + 2);
    if (find(cmds_.begin(), cmds_.end(), k) == cmds_.end()) {
      cmds_.push_back(k);
      cout << "Pushing: " << s << std::endl;
    }
    m_cmd_value_lib_[k].push_back(v);
  }

  assert(!cmds_.empty());
}

void Mutator::init_relationmap() {
  assert(!cmds_.empty());
  relationmap[id_column_name] = id_top_table_name;
  relationmap[id_table_name] = id_top_table_name;
  relationmap[id_index_name] = id_top_table_name;
  relationmap[id_create_column_name] = id_create_table_name;
  relationmap[id_pragma_value] = id_pragma_name;
  cross_map[id_top_table_name] = id_create_table_name;
}

void Mutator::init(const string &f_testcase, const string &f_common_string,
                   const string &pragma) {
  if (!f_testcase.empty()) init_ir_library(f_testcase);

  init_tables();

  init_value_library();

  init_string_library();

  init_pragma(pragma);

  init_relationmap();
}

vector<IR *> Mutator::mutate(IR *input) {
  vector<IR *> res;

  if (!lucky_enough_to_be_mutated(input->mutated_times_)) {
    return res;
  }

  MutationWeights seed_weights = get_seed_adaptive_weights(input);
  MutationWeights final_weights = get_feedback_adaptive_weights(seed_weights);

  MutationKind kind = choose_mutation_kind(final_weights);

  IR *mutated = NULL;

  switch (kind) {
    case MutationKind::Delete:
      mutated = strategy_delete(input);
      break;

    case MutationKind::Insert:
      mutated = strategy_insert(input);
      break;

    case MutationKind::Replace:
      mutated = strategy_replace(input);
      break;
  }

  update_mutation_stats(kind, mutated);

  if (mutated != NULL) {
    res.push_back(mutated);
  }

  input->mutated_times_ += res.size();

  for (auto i : res) {
    if (i == NULL) continue;
    i->mutated_times_ = input->mutated_times_;
  }

  return res;
}

bool Mutator::replace(IR *root, IR *old_ir, IR *new_ir) {
  auto parent_ir = locate_parent(root, old_ir);
  if (parent_ir == NULL) return false;
  if (parent_ir->left_ == old_ir) {
    deep_delete(old_ir);
    parent_ir->left_ = new_ir;
    return true;
  } else if (parent_ir->right_ == old_ir) {
    deep_delete(old_ir);
    parent_ir->right_ = new_ir;
    return true;
  }

  return false;
}

IR *Mutator::locate_parent(IR *root, IR *old_ir) {
  if (root->left_ == old_ir || root->right_ == old_ir) return root;

  if (root->left_ != NULL)
    if (auto res = locate_parent(root->left_, old_ir)) return res;
  if (root->right_ != NULL)
    if (auto res = locate_parent(root->right_, old_ir)) return res;

  return NULL;
}

string Mutator::validate(IR *root) {
  if (root == NULL) return "";
  try {
    string sql_str = root->to_string();
    auto parsed_ir = parser(sql_str);
    if (parsed_ir == NULL) return "";
    parsed_ir->deep_delete();

    reset_counter();
    vector<IR *> ordered_ir;
    auto graph = build_dependency_graph(root, ordered_ir);
    fix_graph(graph, root, ordered_ir);
    return fix(root);
  } catch (...) {
    // invalid sql , skip
  }
  return "";
}

static void collect_ir(IR *root, set<IDTYPE> &type_to_fix,
                       vector<IR *> &ir_to_fix) {
  auto idtype = root->id_type_;

  if (root->left_) {
    collect_ir(root->left_, type_to_fix, ir_to_fix);
  }

  if (type_to_fix.find(idtype) != type_to_fix.end()) {
    ir_to_fix.push_back(root);
  }

  if (root->right_) {
    collect_ir(root->right_, type_to_fix, ir_to_fix);
  }
}

static IR *search_mapped_ir(IR *ir, IDTYPE idtype) {
  vector<IR *> to_search;
  vector<IR *> backup;
  to_search.push_back(ir);
  while (!to_search.empty()) {
    for (auto i : to_search) {
      if (i->id_type_ == idtype) {
        return i;
      }
      if (i->left_) {
        backup.push_back(i->left_);
      }
      if (i->right_) {
        backup.push_back(i->right_);
      }
    }
    to_search = move(backup);
    backup.clear();
  }
  return NULL;
}

void Mutator::cross_stmt_map(map<IR *, set<IR *>> &graph,
                             vector<IR *> &ir_to_fix) {
  for (auto m : cross_map) {
    vector<IR *> value;
    vector<IR *> key;

    for (auto &k : graph) {
      if (k.first->id_type_ == m.first) {
        key.push_back(k.first);
      }
    }

    for (auto &k : ir_to_fix) {
      if (k->id_type_ == m.second) {
        value.push_back(k);
      }
    }

    if (key.empty()) return;
    for (auto val : value) {
      graph[pick_random_element(key)].insert(val);
    }
  }
}

void toptable_map(map<IR *, set<IR *>> &graph, vector<IR *> &ir_to_fix,
                  vector<IR *> &toptable) {
  vector<IR *> tablename;
  for (auto ir : ir_to_fix) {
    if (ir->id_type_ == id_table_name) {
      tablename.push_back(ir);
    } else if (ir->id_type_ == id_top_table_name) {
      toptable.push_back(ir);
    }
  }
  if (toptable.empty()) return;
  for (auto k : tablename) {
    auto r = get_rand_int(toptable.size());
    graph[toptable[r]].insert(k);
  }
}

vector<IR *> Mutator::extract_statement(IR *root) {
  vector<IR *> res;
  deque<IR *> bfs = {root};

  while (bfs.empty() != true) {
    auto node = bfs.front();
    bfs.pop_front();

    if (node->type_ == kStatement) res.push_back(node);
    if (node->left_) bfs.push_back(node->left_);
    if (node->right_) bfs.push_back(node->right_);
  }

  return res;
}

vector<IR *> Mutator::cut_subquery(IR *program, map<IR **, IR *> &m_save) {
  vector<IR *> res;
  vector<IR *> v_statements;
  deque<IR *> dfs = {program};

  while (dfs.empty() != true) {
    auto node = dfs.front();
    dfs.pop_front();

    if (node->type_ == kStatement) v_statements.push_back(node);
    if (node->left_) dfs.push_back(node->left_);
    if (node->right_) dfs.push_back(node->right_);
  }

  reverse(v_statements.begin(), v_statements.end());
  for (auto &stmt : v_statements) {
    deque<IR *> q_bfs = {stmt};
    res.push_back(stmt);

    while (!q_bfs.empty()) {
      auto cur = q_bfs.front();
      q_bfs.pop_front();

      if (cur->left_) {
        q_bfs.push_back(cur->left_);
        if (cur->left_->type_ == kSelectNoParen) {
          res.push_back(cur->left_);
          m_save[&cur->left_] = cur->left_;
          cur->left_ = NULL;
        }
      }

      if (cur->right_) {
        q_bfs.push_back(cur->right_);
        if (cur->right_->type_ == kSelectNoParen) {
          res.push_back(cur->right_);
          m_save[&cur->right_] = cur->right_;
          cur->right_ = NULL;
        }
      }
    }
  }
  return res;
}

bool Mutator::fix_back(map<IR **, IR *> &m_save) {
  for (auto &i : m_save) {
    if (*(i.first) != NULL) return false;
    *(i.first) = i.second;
  }

  return true;
}

map<IR *, set<IR *>> Mutator::build_dependency_graph(IR *root,
                                                     vector<IR *> &ordered_ir) {
  map<IR *, set<IR *>> graph;
  set<IDTYPE> type_to_fix;
  map<IR **, IR *> m_save;
  for (auto &iter : relationmap) {
    type_to_fix.insert(iter.first);
    type_to_fix.insert(iter.second);
  }

  auto ir_list = cut_subquery(root, m_save);

  for (auto stmt : ir_list) {
    vector<IR *> ir_to_fix;
    collect_ir(stmt, type_to_fix, ir_to_fix);
    for (auto ii : ir_to_fix) {
      ordered_ir.push_back(ii);
    }
    cross_stmt_map(graph, ir_to_fix);
    vector<IR *> v_top_table;
    toptable_map(graph, ir_to_fix, v_top_table);
    for (auto ir : ir_to_fix) {
      auto idtype = ir->id_type_;
      graph[ir].clear();
      if (relationmap.find(idtype) == relationmap.end()) {
        continue;
      }

      IR *match_ir = find_closest_node(stmt, ir, relationmap[idtype]);

      if (match_ir != NULL) {
        if (ir->type_ == kColumnName && ir->left_ != NULL) {
          if (v_top_table.size() > 0)
            match_ir = pick_random_element(v_top_table);
          graph[match_ir].insert(ir->left_);
          if (ir->right_) {
            graph[match_ir].insert(ir->right_);
            ir->left_->id_type_ = id_table_name;
            ir->right_->id_type_ = id_column_name;
            ir->id_type_ = id_whatever;
          }
        } else
          graph[match_ir].insert(ir);
      }
    }
  }

  fix_back(m_save);
  return graph;
}

IR *Mutator::find_closest_node(IR *root, IR *current_ir, IDTYPE target_idtype) {
  IR *curptr = current_ir;
  while (true) {
    IR *pptr = locate_parent(root, curptr);
    if (pptr == NULL) break;

    bool flag = false;
    while (pptr->left_ == NULL || pptr->right_ == NULL) {
      curptr = pptr;
      pptr = locate_parent(root, curptr);
      if (pptr == NULL) {
        flag = true;
        break;
      }
    }
    if (flag) break;

    IR *to_search_child = pptr->left_;
    if (pptr->left_ == curptr) {
      to_search_child = pptr->right_;
    }

    IR *match_ir = search_mapped_ir(to_search_child, target_idtype);
    if (match_ir != NULL) {
      return match_ir;
    }
    curptr = pptr;  // Move up to the parent and continue searching
  }
  return NULL;  // No matching node found
}

IR *Mutator::strategy_delete(IR *cur) {
  assert(cur);
  MUTATESTART

  DOLEFT
  res = deep_copy(cur);
  if (res->left_ != NULL) deep_delete(res->left_);
  res->left_ = NULL;

  DORIGHT
  res = deep_copy(cur);
  if (res->right_ != NULL) deep_delete(res->right_);
  res->right_ = NULL;

  DOBOTH
  res = deep_copy(cur);
  if (res->left_ != NULL) deep_delete(res->left_);
  if (res->right_ != NULL) deep_delete(res->right_);
  res->left_ = res->right_ = NULL;

  MUTATEEND
}

IR *Mutator::strategy_insert(IR *cur) {
  assert(cur);

  // Special Case: Append to Statement List
  if (cur->type_ == kStatementList) {
    auto &lib = left_lib[kStatementList];
    if (!lib.empty()) {
      auto new_right = deep_copy(pick_random_element(lib));
      return new IR(kStatementList, OPMID(";"), deep_copy(cur), new_right);
    }
  }

  auto res = deep_copy(cur);

  // Case 1: Missing Right Child
  if (res->left_ && !res->right_) {
    auto &lib = left_lib[res->left_->type_];
    if (!lib.empty()) {
      res->right_ = deep_copy(pick_random_element(lib));
      return res;
    }
  }
  // Case 2: Missing Left Child
  else if (!res->left_ && res->right_) {
    auto &lib = right_lib[res->right_->type_];
    if (!lib.empty()) {
      res->left_ = deep_copy(pick_random_element(lib));
      return res;
    }
  }

  // Case 3: Both Children Missing (Get children from random node of parenttype)
  else if (!res->left_ && !res->right_) {
    auto &lib = ir_library_[res->type_];
    if (!lib.empty()) {
      auto *blueprint = pick_random_element(lib);
      if (blueprint->left_ && blueprint->right_) {
        res->left_ = deep_copy(blueprint->left_);
        res->right_ = deep_copy(blueprint->right_);
        return res;
      }
    }
  }

  // Fallback: Replace the entire node with a variant from the main library
  auto &lib = ir_library_[res->type_];
  if (!lib.empty()) {
    auto *blueprint = pick_random_element(lib);
    if (blueprint->left_ && blueprint->right_) {
      res->left_ = deep_copy(blueprint->left_);
      res->right_ = deep_copy(blueprint->right_);
      return res;
    }
  } else {
    deep_delete(res);
    return nullptr;
  }
}

IR *Mutator::strategy_replace(IR *cur) {
  assert(cur);

  MUTATESTART

  DOLEFT
  res = deep_copy(cur);

  IR *new_node = NULL;
  if (res->left_ != NULL) {
    new_node = get_ir_from_library(res->left_->type_);
    if (new_node != NULL) {
      new_node = deep_copy(new_node);
      new_node->id_type_ = res->left_->id_type_;
    }
    deep_delete(res->left_);
  }
  res->left_ = new_node;

  DORIGHT
  res = deep_copy(cur);

  IR *new_node2 = NULL;
  if (res->right_ != NULL) {
    new_node2 = get_ir_from_library(res->right_->type_);
    if (new_node2 != NULL) {
      new_node2 = deep_copy(new_node2);
      new_node2->id_type_ = res->right_->id_type_;
    }
    deep_delete(res->right_);
  }
  res->right_ = new_node2;

  DOBOTH
  res = deep_copy(cur);

  IR *new_left = NULL;
  IR *new_right = NULL;
  if (res->left_ != NULL) {
    new_left = get_ir_from_library(res->left_->type_);
    if (new_left != NULL) {
      new_left = deep_copy(new_left);
      new_left->id_type_ = res->left_->id_type_;
    }
  }
  if (res->right_ != NULL) {
    new_right = get_ir_from_library(res->right_->type_);
    if (new_right != NULL) {
      new_right = deep_copy(new_right);
      new_right->id_type_ = res->right_->id_type_;
    }
  }

  if (res->left_) deep_delete(res->left_);
  if (res->right_) deep_delete(res->right_);
  res->left_ = new_left;
  res->right_ = new_right;

  MUTATEEND

  return res;
}

bool Mutator::lucky_enough_to_be_mutated(unsigned int mutated_times) {
  if (get_rand_int(mutated_times + 1) < LUCKY_NUMBER) {
    return true;
  }
  return false;
}

void Mutator::add_ir_to_library(IR *cur) {
  extract_struct(cur);
  IRTYPE p_type = cur->type_;
  unsigned long p_hash = hash(cur->to_string());
  if (ir_library_hash_[p_type].find(p_hash) != ir_library_hash_[p_type].end()) {
    return;
  }
  IR *ir_copy = deep_copy(cur);
  add_ir_to_library_no_deepcopy(ir_copy);
}

void Mutator::add_ir_to_library_no_deepcopy(IR *cur) {
  auto left = cur->left_;
  auto right = cur->right_;

  auto type = cur->type_;
  auto h = hash(cur);
  if (find(ir_library_hash_[type].begin(), ir_library_hash_[type].end(), h) !=
      ir_library_hash_[type].end())
    return;

  ir_library_hash_[type].insert(h);
  ir_library_[type].push_back(cur);

  if (left) add_ir_to_library_no_deepcopy(left);
  if (right) add_ir_to_library_no_deepcopy(right);

  // update right_lib, left_lib
  auto left_type = left ? left->type_ : kUnknown;
  auto right_type = right ? right->type_ : kUnknown;

  if (right && left) {
    right_lib[right_type].push_back(left);
    left_lib[left_type].push_back(right);
  }

  return;
}

unsigned long Mutator::hash(const string &sql) {
  return ducking_hash(sql.c_str(), sql.size());
}

unsigned long Mutator::hash(IR *root) { return this->hash(root->to_string()); }

Mutator::~Mutator() {}

void Mutator::fix_one(IR *fixed_key, map<IR *, set<IR *>> &graph,
                      set<IR *> &visited) {
  if (fixed_key->id_type_ == id_create_table_name) {
    string tablename = fixed_key->str_val_;
    auto &columns = m_tables[tablename];
    for (auto &val : graph[fixed_key]) {
      if (val->id_type_ == id_create_column_name) {
        string new_column = gen_id_name();
        columns.push_back(new_column);
        val->str_val_ = new_column;
        visited.insert(val);
      } else if (val->id_type_ == id_top_table_name) {
        val->str_val_ = tablename;
        visited.insert(val);
        fix_one(val, graph, visited);
      }
    }
  } else if (fixed_key->id_type_ == id_top_table_name) {
    string tablename = fixed_key->str_val_;
    auto &columns = m_tables[tablename];

    for (auto &val : graph[fixed_key]) {
      if (val->id_type_ == id_column_name) {
        val->str_val_ = pick_random_or(columns, gen_id_name());
        visited.insert(val);
      } else if (val->id_type_ == id_table_name) {
        val->str_val_ = tablename;
        visited.insert(val);
      } else if (val->id_type_ == id_index_name) {
        string new_index = gen_id_name();
        val->str_val_ = new_index;
        m_tables[new_index] = m_tables[tablename];
        v_table_names.push_back(new_index);
      }
    }
  }
}

void Mutator::fix_graph(map<IR *, set<IR *>> &graph, IR *root,
                        vector<IR *> &ordered_ir) {
  set<IR *> visited;

  reset_database();
  for (auto ir : ordered_ir) {
    auto iter = make_pair(ir, graph[ir]);

    if (visited.find(iter.first) != visited.end()) {
      continue;
    }
    visited.insert(iter.first);
    if (iter.second.empty()) {
      if (iter.first->id_type_ == id_column_name) {
        string tablename = pick_random_or(v_table_names, gen_id_name());
        auto &columns = m_tables[tablename];
        iter.first->str_val_ = pick_random_or(columns, gen_id_name());
        continue;
      }
    }
    if (iter.first->id_type_ == id_create_table_name ||
        iter.first->id_type_ == id_top_table_name) {
      if (iter.first->id_type_ == id_create_table_name) {
        string new_table_name = gen_id_name();
        v_table_names.push_back(new_table_name);
        iter.first->str_val_ = new_table_name;
      } else {
        iter.first->str_val_ = pick_random_or(v_table_names, gen_id_name());
      }
      fix_one(iter.first, graph, visited);
    }
  }
}

/* tranverse ir in the order: _right ==> root ==> left_ */
string Mutator::fix(IR *root) {
  string res;
  auto *right_ = root->right_, *left_ = root->left_;
  auto *op_ = root->op_;
  auto type_ = root->type_;
  auto str_val_ = root->str_val_;
  auto float_val_ = root->float_val_;
  auto int_val_ = root->int_val_;
  auto id_type_ = root->id_type_;

  string tmp_right;
  if (right_ != NULL) tmp_right = fix(right_);

  if (type_ == kIdentifier &&
      (id_type_ == id_database_name || id_type_ == id_schema_name)) {
    if (get_rand_int(2) == 1)
      return string("main");
    else
      return string("temp");
  }

  if (type_ == kCmdPragma) {
    string res = "PRAGMA ";
    const string &key = pick_random_element(cmds_);
    res += key;

    string value = pick_random_element(m_cmd_value_lib_[key]);
    if (!value.compare("_int_")) {
      value = string("=") + to_string(pick_random_element(value_library_));
    } else if (!value.compare("_empty_")) {
      value = "";
    } else if (!value.compare("_boolean_")) {
      if (get_rand_int(2) == 0)
        value = "=false";
      else
        value = "=true";
    } else {
      value = "=" + value;
    }
    if (!value.empty()) res += value + ";";
    return res;
  }

  if (type_ == kFilePath || type_ == kPrepareTargetQuery ||
      type_ == kOptOrderType || type_ == kColumnType || type_ == kSetType ||
      type_ == kOptJoinType || type_ == kOptDistinct || type_ == kNullLiteral)
    return str_val_;
  if (type_ == kStringLiteral) {
    auto s = pick_random_element(string_library_);
    return "'" + s + "'";
  }
  if (type_ == kIntLiteral)
    return std::to_string(pick_random_element(value_library_));
  if (type_ == kFloatLiteral || type_ == kconst_float)
    return std::to_string(float(pick_random_element(value_library_)) + 0.1);
  if (type_ == kconst_str) return pick_random_element(string_library_);
  ;
  if (type_ == kconst_int)
    return std::to_string(pick_random_element(value_library_));

  if (!str_val_.empty()) return str_val_;

  if (op_ != NULL) res += op_->prefix_ + " ";
  if (left_ != NULL) res += fix(left_) + " ";
  if (op_ != NULL) res += op_->middle_ + " ";
  if (right_ != NULL) res += tmp_right + " ";
  if (op_ != NULL) res += op_->suffix_;

  trim_string(res);
  return res;
}
unsigned int Mutator::calc_node(IR *root) {
  if (root == NULL) return 0;
  return 1 + calc_node(root->left_) + calc_node(root->right_);
}

void Mutator::extract_struct(IR *root, bool use_unique_names) {
  static int counter = 0;

  if (root == nullptr) return;

  if (root->left_) extract_struct(root->left_, use_unique_names);
  if (root->right_) extract_struct(root->right_, use_unique_names);

  auto type_ = root->type_;

  if (type_ == kColumnName && root->str_val_ == "*") return;

  if (type_ == kOptOrderType || type_ == kNullLiteral || type_ == kColumnType ||
      type_ == kSetType || type_ == kOptJoinType || type_ == kOptDistinct)
    return;

  if (root->id_type_ != id_whatever && root->id_type_ != id_module_name) {
    root->str_val_ = use_unique_names ? "x" + to_string(counter++) : "x";
    return;
  }

  if (type_ == kPrepareTargetQuery || type_ == kStringLiteral) {
    string str_val = root->str_val_;

    str_val.erase(std::remove(str_val.begin(), str_val.end(), '\''),
                  str_val.end());
    str_val.erase(std::remove(str_val.begin(), str_val.end(), '"'),
                  str_val.end());

    string magic_string = magic_string_generator(str_val);
    unsigned long h = hash(magic_string);

    if (string_library_hash_.find(h) == string_library_hash_.end()) {
      string_library_.push_back(magic_string);
      string_library_hash_.insert(h);
    }

    root->str_val_ = "'y'";
    return;
  }

  if (type_ == kIntLiteral) {
    root->int_val_ = 10;
    return;
  }

  if (type_ == kFloatLiteral || type_ == kconst_float) {
    root->float_val_ = 0.1;
    return;
  }

  if (type_ == kconst_int) {
    root->int_val_ = 11;
    return;
  }

  if (type_ == kFilePath) {
    root->str_val_ = "'file_name'";
    return;
  }
}

void Mutator::add_new_table(IR *root, string &table_name) {
  if (root->left_ != NULL) add_new_table(root->left_, table_name);

  if (root->right_ != NULL) add_new_table(root->right_, table_name);

  // add to table_name_lib_
  if (root->type_ == kTableName) {
    if (root->operand_num_ == 1) {
      table_name = root->left_->str_val_;
    } else if (root->operand_num_ == 2) {
      table_name = root->left_->str_val_ + "." + root->right_->str_val_;
    }
  }

  // add to column_name_lib_
  if (root->type_ == kColumnDef) {
    auto tmp = root->left_;
    if (tmp->type_ == kIdentifier) {
      if (!table_name.empty() && !tmp->str_val_.empty())
        ;
      m_tables[table_name].push_back(tmp->str_val_);
      if (find(v_table_names.begin(), v_table_names.end(), table_name) !=
          v_table_names.end())
        v_table_names.push_back(table_name);
    }
  }
}

void Mutator::reset_database() {
  m_tables.clear();
  v_table_names.clear();
}

int Mutator::try_fix(char *buf, int len, char *&new_buf, int &new_len) {
  string sql(buf);
  auto ast = parser(sql);

  new_buf = buf;
  new_len = len;
  if (ast == NULL) return 0;

  vector<IR *> v_ir;
  auto ir_root = ast->translate(v_ir);
  ast->deep_delete();

  if (ir_root == NULL) return 0;
  auto fixed = validate(ir_root);
  deep_delete(ir_root);
  if (fixed.empty()) return 0;

  char *sfixed = (char *)malloc(fixed.size() + 1);
  memcpy(sfixed, fixed.c_str(), fixed.size());
  sfixed[fixed.size()] = 0;

  new_buf = sfixed;
  new_len = fixed.size();

  return 1;
}

MutationWeights Mutator::get_seed_adaptive_weights(IR *input) {
  MutationWeights weights = base_weights_;

  unsigned int node_count = calc_node(input);

  if (node_count < 10) {
    weights.delete_weight = 10;
    weights.insert_weight = 55;
    weights.replace_weight = 35;
  } else if (node_count > 60) {
    weights.delete_weight = 40;
    weights.insert_weight = 20;
    weights.replace_weight = 40;
  }

  if (input->mutated_times_ > 1000) {
    weights.delete_weight += 10;
    weights.insert_weight = max(1, weights.insert_weight - 10);
  }

  return weights;
}

MutationKind Mutator::choose_mutation_kind(const MutationWeights &weights) {
  int total =
      weights.delete_weight + weights.insert_weight + weights.replace_weight;

  int r = get_rand_int(total);

  if (r < weights.delete_weight) {
    return MutationKind::Delete;
  }

  r -= weights.delete_weight;

  if (r < weights.insert_weight) {
    return MutationKind::Insert;
  }

  return MutationKind::Replace;
}

static int success_bonus(const MutationStats &stats) {
  if (stats.used < 20) return 0;

  double ratio = static_cast<double>(stats.success) / stats.used;

  if (ratio > 0.70) return 15;
  if (ratio > 0.50) return 8;
  if (ratio < 0.20) return -10;

  return 0;
}

MutationWeights Mutator::get_feedback_adaptive_weights(
    const MutationWeights &seed_weights) {
  MutationWeights weights = seed_weights;

  weights.delete_weight += success_bonus(delete_stats_);
  weights.insert_weight += success_bonus(insert_stats_);
  weights.replace_weight += success_bonus(replace_stats_);

  weights.delete_weight = max(1, weights.delete_weight);
  weights.insert_weight = max(1, weights.insert_weight);
  weights.replace_weight = max(1, weights.replace_weight);

  return weights;
}

void Mutator::update_mutation_stats(MutationKind kind, IR *result) {
  MutationStats *stats = NULL;

  switch (kind) {
    case MutationKind::Delete:
      stats = &delete_stats_;
      break;

    case MutationKind::Insert:
      stats = &insert_stats_;
      break;

    case MutationKind::Replace:
      stats = &replace_stats_;
      break;
  }

  stats->used++;

  if (result != NULL) {
    stats->success++;
  } else {
    stats->failed++;
  }
}

IR *Mutator::get_ir_from_library(IRTYPE type) {
  auto &bucket = ir_library_[type];
  if (bucket.empty()) return NULL;
  return pick_random_element(bucket);
}
