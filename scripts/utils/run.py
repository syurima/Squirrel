"""
Run a fuzzing instance.
"""
import os
import fire
import uuid
from pathlib import Path

DBMS = ["sqlite", "mysql", "mariadb", "postgresql"]
ROOTPATH = Path(os.path.dirname(os.path.realpath(__file__))).parent.parent


def get_mutator_so_path(database):
  if database == "mariadb":
    database = "mysql"
  return f"{ROOTPATH}/build/lib{database}_mutator.so"


def get_config_path(database):
  return f"{ROOTPATH}/data/config_{database}.yml"


def set_env(database):
  os.environ["AFL_CUSTOM_MUTATOR_ONLY"] = "1"
  os.environ["AFL_DISABLE_TRIM"] = "1"
  os.environ["AFL_FAST_CAL"] = "1"
  os.environ["AFL_CUSTOM_MUTATOR_LIBRARY"] = get_mutator_so_path(database)
  os.environ["SQUIRREL_CONFIG"] = get_config_path(database)


import subprocess

def run(database, input_dir, output_dir=None, config_file=None, fuzzer=None, seed=None, workers: int = 0):
  # Precondition checks
  if database not in DBMS:
    print(f"Unsupported database. The supported ones are {DBMS}")
    return

  if not output_dir:
    output_dir = "/tmp/fuzz"

  if not config_file:
    config_file = get_config_path(database)
  if not fuzzer:
    fuzzer = f"{ROOTPATH}/AFLplusplus/afl-fuzz"
  if not os.path.exists(config_file):
    print("Invalid path for config file")
  if not os.path.exists(fuzzer):
    print("Invalid path for afl-fuzz")

  set_env(database)

  # Allow optionally setting a deterministic seed from the harness.
  if seed is not None:
    os.environ["SQUIRREL_SEED"] = str(seed)

  output_id = str(uuid.uuid4())[:10]

  # Build the base command (master) for the selected DBMS
  if database == "sqlite":
    base_cmd = [fuzzer, "-i", input_dir, "-o", output_dir, "-M", output_id, "--", "/home/ossfuzz", "@@"]
  else:
    base_cmd = [fuzzer, "-i", input_dir, "-o", output_dir, "-M", output_id, "-t", "60000", "--", f"{ROOTPATH}/build/db_driver"]

  # Launch the master instance
  processes = []
  processes.append(subprocess.Popen(base_cmd, env=os.environ.copy()))

  # If workers are requested, launch secondary instances using -S
  for i in range(workers):
    worker_id = f"worker{i+1}"
    worker_cmd = base_cmd.copy()
    # replace the -M flag with -S for workers
    # Find index of "-M" and replace it with "-S" and the worker id
    if "-M" in worker_cmd:
      idx = worker_cmd.index("-M")
      worker_cmd[idx] = "-S"
      worker_cmd[idx + 1] = worker_id
    else:
      # fallback: just prepend -S flag
      worker_cmd = [fuzzer, "-i", input_dir, "-o", output_dir, "-S", worker_id] + worker_cmd[4:]
    processes.append(subprocess.Popen(worker_cmd, env=os.environ.copy()))

  # Wait for all processes to finish
  for p in processes:
    p.wait()


if __name__ == "__main__":
  fire.Fire(run)
