#!/bin/bash

if [ $# -lt 3 ]; then
#    echo "Usage: $0 <json-path> <new-version>"
    exit 1
fi

# json路径 新版本号
JSON_FILE="$1"
newVersion="$2"
env="$3"

# 检查 JSON 文件是否存在
if [ ! -f "$JSON_FILE" ]; then
    echo "Error: JSON file not found."
    exit 1
fi

# 更新 JSON 文件中的 currentVersion 值
sed -i "s/\"currentVersion\": \"[^\"]*\"/\"currentVersion\": \"$newVersion$env\"/" "$JSON_FILE"

# 检查 sed 命令是否成功
if [ $? -eq 0 ]; then
    echo "Version updated to: $newVersion$env"
else
    echo "Error: Failed to update version in JSON."
    exit 1
fi
