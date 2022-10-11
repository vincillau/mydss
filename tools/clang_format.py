#!/usr/bin/env python3
# 使用 clang-format 格式化项目中的所有 C++ 代码

import hashlib
import os
import os.path
import uuid

# 获取路径为 path 的文件的 SHA1 摘要
def sha1_digest(path: str) -> str:
    with open(path, "rb") as f:
        data = f.read()
        sha1 = hashlib.sha1()
        sha1.update(data)
        digest = sha1.hexdigest()
        return digest


# 格式化路径为 file_path 的 C++ 代码
def format_file(file_path: str) -> None:
    old_digest = sha1_digest(file_path)
    tmp_file = f"/tmp/clang-format-{uuid.uuid4()}"
    os.system(f"clang-format {file_path} > {tmp_file}")
    new_digest = sha1_digest(tmp_file)

    if old_digest == new_digest:
        # 如果文件没有改变则输出 '✓ {path}'
        print(f"\033[01;32m✓\033[0m {file_path}")
    else:
        os.system(f"cat {tmp_file} > {file_path}")
        # 如果文件改变则输出 'X {path}'
        print(f"\033[01;31mX\033[0m {file_path}")

    os.remove(tmp_file)


# 判断是否是 C++ 文件
def is_cpp_file(file_path: str) -> bool:
    if file_path.endswith(".hpp"):
        return True
    if file_path.endswith(".cpp"):
        return True
    return False


# 格式化 dir_path 目录中的所有 C++ 代码
def format_dir(dir_path: str) -> None:
    for i in sorted(os.listdir(dir_path)):
        p = os.path.join(dir_path, i)
        if os.path.isdir(p):
            format_dir(p)
            continue
        if is_cpp_file(p):
            format_file(p)


if __name__ == "__main__":
    format_dir("include")
    format_dir("src")
    format_dir("test")
