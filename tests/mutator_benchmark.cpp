#include <chrono>
#include <iostream>
#include <set>

#include "../srcs/internal/sqlite/include/mutator.h"
#include "../srcs/internal/sqlite/include/ast.h"
#include "../srcs/internal/sqlite/include/utils.h"

using namespace std;

int main() {
  cerr << "BENCHMARK STARTED" << endl;
  Mutator mutator;

  mutator.init(
      "/home/Squirrel/data/fuzz_root/init_lib/1.txt",
      "",
      "/home/Squirrel/data/fuzz_root/pragma"
  );

  string seed_sql = "CREATE TABLE t1(a INT); SELECT * FROM t1 WHERE a > 10;";
  auto ast = parser(seed_sql);

  vector<IR *> v_ir;
  IR *root = ast->translate(v_ir);

  const int N = 100000;

  int success = 0;
  int failed = 0;
  set<unsigned long> unique_hashes;

  auto start = chrono::high_resolution_clock::now();

  for (int i = 0; i < N; i++) {
    if (i % 10000 == 0) {
      cout << "Progress: " << i << "/" << N << endl;
    }
    IR *target = v_ir[i % v_ir.size()];
    vector<IR *> mutated = mutator.mutate(target);

    if (mutated.empty()) {
      failed++;
      continue;
    }

    for (auto ir : mutated) {
      if (ir != NULL) {
        success++;
        unique_hashes.insert(mutator.hash(ir));
        deep_delete(ir);
      } else {
        failed++;
      }
    }
  }

  auto end = chrono::high_resolution_clock::now();

  auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

  cout << "mutations=" << N << endl;
  cout << "time_ms=" << ms << endl;
  cout << "mutations_per_sec=" << (N * 1000.0 / ms) << endl;
  cout << "success=" << success << endl;
  cout << "failed=" << failed << endl;
  cout << "success_rate=" << (success * 1.0 / (success + failed)) << endl;
  cout << "unique_outputs=" << unique_hashes.size() << endl;

  deep_delete(root);
  ast->deep_delete();

  return 0;
}