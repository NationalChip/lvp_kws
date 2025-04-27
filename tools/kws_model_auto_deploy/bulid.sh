#!/bin/bash

echo "-----------> 开始打包 <-----------"
pyinstaller -F kwsModelAutoDeploy.py --clean
cp dist/* .
rm -rf __pycache__ build dist *.spec
echo "-----------> 打包完成 <-----------"
