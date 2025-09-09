# -*- coding: utf-8 -*-
from __future__ import unicode_literals
from pypinyin import pinyin, lazy_pinyin,load_phrases_dict
import pypinyin
import os
import numpy as np


#################################################################################
# pypinyin 版本为0.35.1,对于一些错的音的标注，需要自定义词典来解决,DuoYinDict模块
#################################################################################
class DuoYinDict(object):
    def __init__(self,dict_file):
        dy_dict=creat_dict(dict_file)
        load_phrases_dict(dy_dict)


def creat_dict(dict_file):
    with open(dict_file,'r')as f1:
        py_dict = {}
        for line in f1.readlines():
            l = line.strip().split(' ')
            key = l[0]
            py = l[1:]
            py = np.reshape(py,[-1,1])
            py = py.tolist()
            py_dict[key] = py

    return py_dict

try:
    absolute_path = os.path.dirname(os.path.abspath(__file__))
    DuoYinDict(os.path.join(absolute_path, 'units/revise_dict.txt'))
except:
    DuoYinDict("modules/common/units/revise_dict.txt")

# 在模块级别执行实例化代码
if __name__ == "__main__":
    pass
