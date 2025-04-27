#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re, os

class GetModelInfor:
    def __init__(self):
        # 初始化全局变量
        self.lines = []  # 存储文件的每一行
        self.res = ""  # 初始化结果字符串
        self.info = {}  # 存储结构体信息的字典

    def read_normmalized(self, normalized_path):
        """
            功能:
                    * 读取声学模型归一化参数
            参数:
                    * 模型包中归一化参数文件路径： normalized_path
            返回值:
                    * 归一化参数字典
        """
        if not os.path.exists(normalized_path):
            return {}
        with open(normalized_path, "r") as f:
            data = f.read()
        output_dict = eval(data)
        if not isinstance(output_dict, dict):
            print("\033[0;36;31m[GetModelInfor]: 错误: 声学模型归一化读取失败, 请检查:%s\033[0m"%normalized_path)
            return {}
        return output_dict

    def get_npu_size(self):
        """
            功能:
                    * 获取 model.h 中 npu_size 的值
            参数:
                    * /
            返回值:
                    * int
        """
        try:
            for line in self.lines:
                if re.findall("int npu_size\s*=\s*\d+\s*;", line):
                    npu_size = re.findall("int npu_size\s*=\s*\d+\s*;", line)[0]
                    npu_size = int(npu_size.split("=")[-1].split(";")[0])
                    return npu_size
        except:
            print("\033[0;36;31m[GetModelInfor]: 错误! 声学模型 npu_size 读取失败, 请检查\033[0m")
            return False

    def get_npu_sram_size(self):
        """
            功能:
                    
                    * 获取 model.h 中 ops_content + data_content 的值
            参数:
                    * /
            返回值:
                    * int
        """
        ops_content_size = None
        data_content_szie = None
        try:
            for line in self.lines:
                if re.findall("const unsigned char ops_content\[\d+\]", line):
                    ops_content_size = re.findall("const unsigned char ops_content\[\d+\]", line)[0]
                    ops_content_size = float(ops_content_size.split("[")[-1].split("]")[0])
                    #print(ops_content_size)
                elif re.findall("unsigned char data_content\[\d+\]", line):
                    data_content_szie = re.findall("unsigned char data_content\[\d+\]", line)[0]
                    data_content_szie = float(re.findall("\d+", data_content_szie)[0])
                    #print(data_content_szie)
        except:
            print("\033[0;36;31m[GetModelInfor]: 错误! 声学模型 pu_sram_size 读取失败, 请检查\033[0m")
            return False
        if ops_content_size == None or data_content_szie == None:
            print("\033[0;36;31m[GetModelInfor]: 错误! 声学模型 pu_sram_size 读取失败, 请检查\033[0m")
            return False
        return round(((data_content_szie+ops_content_size)/1024)+1)






    def get_model_info(self, model_h_path, normalized_path):
        """
            功能:
                    * 读取模型信息
            参数:
                    * 模型 model.h 文件路径
            返回值:
                    * 模型信息
        """
        with open(model_h_path, 'r') as f:
            model = f.read()

        self.lines = model.split("\n")  # 将文件内容按行分割
        self.info["npu_size"] = self.get_npu_size()

        self.info["NPU_SRAM_SIZE"] = self.get_npu_sram_size()

        # 获取input和output相关信息
        self.query_struct("input")  # 获取input结构体信息
        self.query_struct("output")  # 获取output结构体信息
        self.query_struct("in_out")  # 获取in_out结构体信息

        self.res += "\n".join(self.lines)
        return {
            "info": self.info,  # 返回结构体信息
            "res": self.res,  # 返回结果字符串
            "normmalized infor": self.read_normmalized(normalized_path),
        }

    def query_struct(self, name):
        """
            功能:
                    * 查询数据结构
            参数:
                    * 数据结构名
            返回值:
                    * /
        """
        t = ""  # 初始化当前行
        rule = ""  # 初始化正则表达式
        last_t = ""  # 初始化上一行
        last_str = "" #初始化上一行字符
        input_next = 0  # 初始化全局变量

        # 先找到结构体的头部
        rule = re.compile(rf'\s*struct\s+.*\b{name}\b\s*.*{{')  # 定义正则表达式
        while self.lines:  # 遍历文件行
            t = self.lines.pop(0)  # 取出第一行
            if t and not re.match(r"//", t):  # 如果当前行不为空且不匹配 "//"
                self.res += t + "\n"  # 添加当前行到结果字符串
            else:
                self.res += "\n"  # 添加换行符
            if rule.search(t):  # 如果匹配结构体头部
                break  # 退出循环

        # 初始化该字段size
        line = ""
        self.info[f"{name}_size"] = 0  # 初始化结构体大小
        while self.lines:  # 遍历文件行
            type_size = 0  # 初始化数据类型大小
            last_str = line  # 更新上一行字符
            last_t = t  # 更新上一行
            t = self.lines.pop(0)  # 取出第一行
            line = t
            self.res += t + "\n"  # 添加当前行到结果字符串

            # 获取基本数据类型字节数
            if re.search(r"npu_data_t|short|unsigned short", t):  # 如果匹配数据类型
                type_size = 2  # 设置数据类型大小为2
            elif re.search(r"float|int|unsigned int", t):  # 如果匹配数据类型
                type_size = 4  # 设置数据类型大小为4
            elif re.search(r"signed char", t):  # 如果匹配数据类型
                type_size = 1  # 设置数据类型大小为1

            # 取出输入和输出数组名, 默认为input结构体的第一个数组为输入, output结构体最后一个数组为输出
            if name == "input" and re.search(r"input", last_t):  # 如果是input结构体
                self.info[f"{name}_name"] = t[t.rfind(" ") + 1:t.index("[")].strip()  # 提取数组名
                l1 = t.index(']') + 1  # 获取第一个括号位置
                l2 = t.rfind('[') - 1  # 获取最后一个括号位置
                if l2 > l1:  # 如果括号位置有效
                    self.info['window_length'] = t[l1 + 1:l2].strip()  # 提取窗口长度
                input_next = 1  # 设置标志位

            elif name == "output" and re.search(r'.*\s*__attribute__', self.lines[0]):  # 如果是output结构体
                if re.search("padding", t):
                    self.info[f"{name}_name"] = last_str[last_str.rfind(" ") + 1:last_str.index("[")].strip()  # 提取数组名
                else:
                    self.info[f"{name}_name"] = t[t.rfind(" ") + 1:t.index("[")].strip()  # 提取数组名

            #elif re.search(r"State|_copy|state", t):  # 如果匹配 "State"
            elif re.search(r"State|state", t):  # 如果匹配 "State"
                self.info['isRnn'] = True  # 设置RNN标志
                if input_next:  # 如果标志位有效
                    type_keyword = re.search(r"npu_data_t|short|char", t).group()
                    start = t.index(type_keyword) + len(type_keyword)  # 获取起始位置
                    end = t.index('[')  # 获取结束位置
                    self.info['stateAddr'] = t[start:end].strip()  # 提取状态地址
                    input_next = 0  # 重置标志位

            # 计算结构体大小
            element_num = type_size  # 初始化元素数量
            while "[" in t:  # 如果当前行包含数组
                p = t.index("[")  # 获取第一个括号位置
                q = t.index("]")  # 获取第二个括号位置
                element_num *= int(t[p + 1:q])  # 计算元素数量
                if name == "input" and re.search(r"input", last_t):  # 如果是input结构体
                    self.info['feats_dim'] = int(t[p + 1:q])  # 提取特征维度
                if name == "output" and re.search(r'.*\s*__attribute__', self.lines[0]):  # 如果是output结构体
                    if re.search("padding", t):
                        left_last_addr = last_str.rindex("[") #最后一个[位置
                        right_last_addr = last_str.rindex("]") #最后一个}位置
                        self.info['output_length'] = int(last_str[left_last_addr + 1:right_last_addr])  # 提取输出长度
                    else:
                        self.info['output_length'] = int(t[p + 1:q])  # 提取输出长度
                t = t[q + 1:]  # 更新当前行

            self.info[f"{name}_size"] += element_num  # 更新结构体大小

            if re.search(r'.*\s*__attribute__', t):  # 如果匹配结构体结尾
                break  # 退出循环

if __name__ == "__main__":
    pass

