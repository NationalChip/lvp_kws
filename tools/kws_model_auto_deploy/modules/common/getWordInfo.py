#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import unicode_literals
import os, sys, re
try:
    from modules.common.debugLogger import DebugLogger
    from modules.common.gxPinYing import lazy_pinyin
    from modules.common.getLabeId import get_yinjie_label, is_all_chinese
    from modules.common.getBunKwsWordsBasicInfo import GetBunKwsWordsBasicInfo
    from modules.common.getAlchemyWordsBasicInfo import GetAlchemyWordsBasicInfo
    from modules.common.setGroups import SetGroups
except:
    sys.path.append("../../")
    from modules.common.debugLogger import DebugLogger
    from modules.common.gxPinYing import lazy_pinyin
    from modules.common.getLabeId import get_yinjie_label, is_all_chinese
    from modules.common.getBunKwsWordsBasicInfo import GetBunKwsWordsBasicInfo
    from modules.common.getAlchemyWordsBasicInfo import GetAlchemyWordsBasicInfo
    from modules.common.setGroups import SetGroups

import pprint

__version__="V2.1.0"

class GetWordsConfig:
    def __init__(self, logger=None, internal_msg_bus={}):
        self.logger = logger
        if not self.logger:
            self.logger = DebugLogger("info")
        self.internal_msg_bus = internal_msg_bus
        self.words_info_obj_map = {"bunkws": GetBunKwsWordsBasicInfo,
                            "alchemy": GetAlchemyWordsBasicInfo}
        self.get_words_thr_obj = self.words_info_obj_map.get(self.internal_msg_bus["部署模式"].lower())(self.logger, self.internal_msg_bus)
        self.words_info = self.get_words_thr_obj.GetThresholdData()


    def Info(self, acoustic_model_info= {"modeling_unit":65}):
        """
            功能:
                    * 获取词 label 信息
            参数:
                    * 模型信息
            返回值:
                    * dict: 词信息字典
        """
        if self.internal_msg_bus["部署模式"].lower() in ["bunkws"]:
            try:
                self.logger.log(f'GetWordsConfig]: 调试! 部署模式: {self.internal_msg_bus["部署模式"]} 词信息: {pprint.pformat(self.words_info)}', "debug")
                self.internal_msg_bus["词信息"] = self.words_info
                return True
            except Exception as e:
                self.logger.log(f"[GetWordsConfig]: 错误! 词信息获取失败: {e}", "error")
                return False
        try:
            for index in self.words_info:
                cmd = self.words_info[index]["词"]
                #print("索引:", index)
                #print(self.words_info[index])
                #print("词", cmd)
                assert is_all_chinese(cmd), '指令词包含非中文字符, 请检查. 注意: Alchemy 模型不支持英文'
                sym_labels, sym_label_id, syllable_labels, syllable_label_id, sym_tone_labels, sym_tone_label_id = get_yinjie_label(cmd)
                if acoustic_model_info["modeling_unit"] == 65:
                    label_id = sym_label_id
                elif acoustic_model_info["modeling_unit"] == 410:
                    label_id = syllable_label_id
                elif acoustic_model_info["modeling_unit"] == 191:
                    label_id = sym_tone_label_id
                else:
                    raise ValueError(f'\033[0;36;31m[GetWordsConfig]: 错误! modeling_unit 的值为 {acoustic_model_info["modeling_unit"]} 该值无效\033[0m')
                self.words_info[index]["CTC模型阈值"] = 700
                if "label" not in self.words_info[index]:
                    self.words_info[index]["label"] = label_id
                    self.words_info[index]["label_length"] = len(label_id)
                self.words_info[index]["词拼音"] = " ".join(lazy_pinyin(cmd))
            self.internal_msg_bus["词信息"] = self.words_info
            self.logger.log("[GetWordsConfig]: 提示! 开始词分组", "info")
            set_groups = SetGroups(self.internal_msg_bus, self.logger)
            self.internal_msg_bus["分组信息"] = set_groups.StartSetGroups()
            #根据分组，重新排序
            output_dict = {}
            for index in self.internal_msg_bus["词信息"]:
                new_index = self.internal_msg_bus["分组信息"]["分组列表"].index(self.internal_msg_bus["词信息"][index]["词"])
                output_dict[new_index] = self.internal_msg_bus["词信息"][index]
            self.internal_msg_bus["词信息"] = output_dict
        except Exception as e:
            self.logger.log(f"[GetWordsConfig]: 错误! 词信息获取失败: {e}", "error")
            return False
        self.logger.log(f'GetWordsConfig]: 调试! 部署模式: {self.internal_msg_bus["部署模式"]} 词信息: {pprint.pformat(self.internal_msg_bus["词信息"])}', "debug")
        return True


if __name__=="__main__":
    pass
