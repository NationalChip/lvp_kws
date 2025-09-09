#!/usr/bin/env python3
# -*- coding: utf-8 -*-



import sys, re, pprint
from openpyxl import load_workbook
try:
    from modules.common.debugLogger import DebugLogger
    from modules.common.gxPinYing import lazy_pinyin
except:
    sys.path.append("../../")
    from modules.common.debugLogger import DebugLogger
    from modules.common.gxPinYing import lazy_pinyin

__version__="1.0.0"

class GetAlchemyWordsBasicInfo:
    """ 获取词阈值处理类"""
    def __init__(self, logger=None, internal_msg_bus={}):
        self.skip_list = []
        self.logger = logger
        if not self.logger:
            self.logger = DebugLogger()
        self.internal_msg_bus = internal_msg_bus

    def get_keyword_info(self, keywords_filename):
        """
            功能:
                    * 读取 BunKws 模型输出包中的 keyword.txt
                    * 根据该文件中指定的顺序组成有一个有序的 关键字字典
            参数:
                    * str: keyword.txt 文件路径 keywords_filename
            返回值:
                    * dict: 有序关键字列表, single_line
        """
        fail_flag = 0
        output_dict = {}
        event_id_count = 100
        cmd_info_list = []
        input_stride = None
        with open(keywords_filename, "r") as f:
            data = f.readlines()
        for single_line in data:
            single_line = single_line.replace("  ", " ").replace("\n", "")
            if re.findall("md5:", single_line):
                continue
            if re.findall("input_stride:", single_line):
                input_stride = int(single_line.split("input_stride:")[-1].strip())
                self.internal_msg_bus["模型信息"]["input_stride"] = input_stride
                continue

            if re.findall("\[.+\]", single_line) and len(single_line.replace(re.findall("\[.+\]", single_line)[0], "").split(",")) == 4:
                self.logger.log(f"[GetAlchemyWordsBasicInfo]: 调试！{single_line} 格式正确", "debug")
            else:
                self.logger.log(f"[GetAlchemyWordsBasicInfo]: 错误！{single_line} 格式不对，请检查文件: {keywords_filename}", "error")
                fail_flag = 1
                continue
            cmd_label = re.findall("\[.+\]", single_line)[0]
            single_line_list = single_line.replace(re.findall("\[.+\]", single_line)[0], "").split(",")
            is_main_wakeup = 0
            if int(single_line_list[-1]) != 100:
                event_id_count += 1
                event_id = event_id_count
            else:
                event_id = 100
                is_main_wakeup = 1
            tmp_dict = {
                        #"CTC模型阈值": 700,
                        "label": eval(cmd_label),
                        "label_length": len(eval(cmd_label)),
                        "事件ID": event_id,
                        "是否主唤醒": is_main_wakeup,
                        "词":single_line_list[0],
                        "阈值": single_line_list[-2],
                        }
            cmd_info_list.append(tmp_dict)

        if fail_flag:
            raise ValueError(f"[GetAlchemyWordsBasicInfo]: 错误！阈值、label、等配置格式不对，请检查文件: {keywords_filename}\033[0m")
        #print(cmd_info_list)
        sorted_data = sorted(cmd_info_list, key=lambda x: x['事件ID'])

        # 然后转换为有序字典，键从1开始
        for i, item in enumerate(sorted_data, 1):
            output_dict[i] = item
            #output_dict[index] = {cmd: tmp_dict}
        output_dict = {k: output_dict[k] for k in sorted(output_dict)}
        #pprint.pprint(output_dict)
        self.logger.log(f"[GetAlchemyWordsBasicInfo]: 调试！{keywords_filename} 文件解析后的数据 {output_dict}", "debug")
        return output_dict


    def GetThresholdData(self):
        """
            功能:
                    *
        """
        keywords_filename = self.internal_msg_bus["输入文件"]["词有序列表文件"]
        #report_filename = self.internal_msg_bus["输入文件"]["模型报告文件"]
        keywords_dict = self.get_keyword_info(keywords_filename)
        if not keywords_dict:
            self.logger.log(f"[GetAlchemyWordsBasicInfo]: 错误！未正确从 {keywords_filename} 文件中获取到有序关键字列表信息", "error")
            return {}
        #self.logger.log(keywords_dict, "debug")
        return keywords_dict
