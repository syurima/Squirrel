#include <algorithm>
#include <cassert>
#include <cfloat>
#include <climits>
#include <cstdio>
#include <deque>
#include <fstream>

#include "ast.h"
#include "define.h"
#include "mutator.h"
#include "mutator_helpers.h"
#include "utils.h"

#define _NON_REPLACE_

using namespace std;
using mutator_common::pick_random_element;

vector<IR *> Mutator::mutate(IR *input) {
  vector<IR *> res;

  if (!lucky_enough_to_be_mutated(input->mutated_times_)) {
    return res;
  }
  auto tmp = strategy_delete(input);
  if (tmp != NULL) {
    res.push_back(tmp);
  }

  tmp = strategy_insert(input);
  if (tmp != NULL) {
    res.push_back(tmp);
  }

  tmp = strategy_replace(input);
  if (tmp != NULL) {
    res.push_back(tmp);
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
  if (!cur) return nullptr;

  // Preserve original node for mutation
  auto res = deep_copy(cur);
  if (!res) return nullptr;
  auto parent_type = cur->type_;  // used for left/right lib lookups

  // Case 1: Missing Right Child – try left_lib based on existing left child
  // type
  if (res->left_ && !res->right_) {
    auto &lib = left_lib[res->left_->type_];
    if (!lib.empty()) {
      res->right_ = deep_copy(pick_random_element(lib));
      return res;
    }
  }
  // Case 2: Missing Left Child – try right_lib based on existing right child type
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
  }

  deep_delete(res);
  return nullptr;
}

IR *Mutator::strategy_replace(IR *cur) {
  assert(cur);

  MUTATESTART

  DOLEFT
  if (cur->left_ != NULL) {
    res = deep_copy(cur);

    auto new_node = get_ir_from_library(res->left_->type_);
    new_node->data_type_ = res->left_->data_type_;
    deep_delete(res->left_);
    res->left_ = deep_copy(new_node);
  }

  DORIGHT
  if (cur->right_ != NULL) {
    res = deep_copy(cur);

    auto new_node = get_ir_from_library(res->right_->type_);
    new_node->data_type_ = res->right_->data_type_;
    deep_delete(res->right_);
    res->right_ = deep_copy(new_node);
  }

  DOBOTH
  if (cur->left_ != NULL && cur->right_ != NULL) {
    res = deep_copy(cur);

    auto new_left = get_ir_from_library(res->left_->type_);
    auto new_right = get_ir_from_library(res->right_->type_);
    new_left->data_type_ = res->left_->data_type_;
    new_right->data_type_ = res->right_->data_type_;
    deep_delete(res->right_);
    res->right_ = deep_copy(new_right);

    deep_delete(res->left_);
    res->left_ = deep_copy(new_left);
  }

  MUTATEEND

  return res;
}

bool Mutator::lucky_enough_to_be_mutated(unsigned int mutated_times) {
  if (get_rand_int(mutated_times + 1) < LUCKY_NUMBER) {
    return true;
  }
  return false;
}