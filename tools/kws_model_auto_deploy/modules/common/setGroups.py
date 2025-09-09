#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, sys
import traceback
from collections import OrderedDict
import difflib

class SetGroups:
    """ 这是一个对词进行分类的一个类"""

    def __init__(self, internal_msg_bus, logger=None):
        #viva 启动配置
        self.internal_msg_bus = internal_msg_bus
        #日志及打印
        self.logger=logger

    def _collect_all_keys(self, initial_key, dictionary, writ_word_list):
        """
            功能:
                    * 展开字典的键值对, 且避免重复
            参数:
                    * 初始 key: initial_key
                    * 源字典: dictionary
                    * 已访问 key 列表 writ_word_list
            返回值:
                    * 展开列表
        """
        # 存储已访问键的列表
        result = []
        # 创建一个集合，用于存储已经访问过的键，以避免重复访问
        visited = set(writ_word_list)
        # 创建一个队列，用于存储待访问的键
        queue = [initial_key]
        # 循环，直到队列为空
        while queue:
            # 从队列中取出一个键
            current_key = queue.pop(0)
            # 如果当前键已经被访问过，则跳过
            if current_key in visited:
                continue
            # 访问当前键，并将其添加到已访问集合
            visited.add(current_key)
            # 将当前键添加到结果列表
            result.append(current_key)
            # 如果当前键在词典中，将其所有未访问的值加入队列
            if current_key in dictionary:
                for value in dictionary[current_key]:
                    if value not in visited:
                        queue.append(value)
        return result

    def _flatten_list(self, nested_list):
        """
            功能:
                    * 多级嵌套列表展开到一个平面
        """
        flat_list = []
        for element in nested_list:
            if isinstance(element, list):
                # 如果元素是列表，则递归展平它，并将结果扩展到 flat_list 中
                flat_list.extend(self._flatten_list(element))
            else:
                # 如果元素不是列表，则直接添加到 flat_list 中
                flat_list.append(element)
        return flat_list

    def _sort_by_word_count_desc(self, strings):
        """
            功能:
                    * 根据字符串列表，中各个元素的字符数量，从大到小排序
            参数:
                    * list 字符串列表
            返回值:
                    * list 排序后的字符串列表 sorted_strings
        """
        # 定义一个函数，用于计算指令词字数
        def word_count(s):
            return len(s)

        # 使用 sorted 函数和自定义的 key 来排序列表
        sorted_strings = sorted(strings, key=word_count, reverse=True)
        return sorted_strings



    def _set_group(self, include_key_list, include_word_dict, group_num, per_group_num_list):
        """
            功能:
                    * 执行分组
            参数:
                    */
            返回值:
                    *
        """
        # 保存最终结果
        result = []
        # 根据有几组设置 group_list 变量中存在几组
        group_list = [[] for _ in range(group_num)]
        # 记录已经写入的指令词
        writ_word_list = []
        # 最大分组数
        max_group_num = max(per_group_num_list)

        for key in include_key_list:
            cur_key_include_word_list = self._collect_all_keys(key, include_word_dict, writ_word_list)
            # 这组包含关系的词已经分完了为空，不需要分了
            if not cur_key_include_word_list:
                # print("该组分配完成，继续下一组")
                continue

            # 单个指令词与其有包含关系词的超过单个分组长度(分组不可整除时与可分配数多的组比较)
            if (len(cur_key_include_word_list) > max_group_num):
                self.logger.log(f'''[SetGroups]: 错误! 单个词与其有包含关系的词个数 {len(cur_key_include_word_list)} 超过单个分组个数 {max_group_num}''', "error")
                return self.get_words_list()

            # 遍历 group_list 各组中元素个数，找到可以容纳的那一组
            for index, group in enumerate(group_list):
                # 当前组已放元素数量加本次放入数量 与 改组可放个数比较 找到符合的那一组
                if (len(self._flatten_list(group)) + len(cur_key_include_word_list)) <= per_group_num_list[index]:
                    # 该组中的元素都未分配过，现将其添加进去这个符合条件的一组
                    if not any(item in writ_word_list for item in group):
                        group_list[index].append([cur_key_include_word_list])
                        writ_word_list += cur_key_include_word_list
                        break
                    else:
                        self.logger.log("[SetGroups]: 错误! 分组中已经包含它，不合理", "error")
                        sys.exit(10)
                elif index >= group_num - 1:
                    self.logger.log("[SetGroups]: 错误! 有关 {key} 的包含关系词，没有找到合适的分组，分组失败，分组停止，请手动分组", "error")
                    break

        # print("具有包含关系的词的分组情况:")
        # for index, group_data in enumerate(group_list):
        #     print(f"分组{index}, 该组有{len(_flatten_list(group_data))}个词, 内容如下:\n {group_data}")

        for group in group_list:
            unfold_cur_group = self._flatten_list(group)
            result.append(unfold_cur_group)

        for word in self.get_words_list():
            if word not in writ_word_list: # 该指令词不是具有包含关系的词，还没参与上面的分组
                for index, cur_group in enumerate(result):
                    if len(cur_group) < per_group_num_list[index]:
                        writ_word_list.append(word)
                        cur_group.append(word)
                        break

        # print("所有的词的分组情况:")
        for index, group_data in enumerate(result):
            #print(f"分组{index}, 该组有{len(group_data)}个词, 内容如下:\n {group_data}")
            result[index] = self._sort_by_word_count_desc(group_data)
            # print(f"分组{index}, 该组有{len(group_data)}个词, 按大小排序后的内容如下:\n {result[index]}")

        result = self._flatten_list(result)

        words_list = self.get_words_list()
        # print(len(result), len(writ_word_list), len(set(words_list)))
        if len(result) != len(writ_word_list) or len(result) != len(set(words_list)):
            self.logger.log("[SetGroups]: 错误! 少了指令词，EXIT", "error")
            sys.exit(10)

        #print(result)
        return result


    #分组 run_build_kws_list
    def StartSetGroups(self, acoustic_model_info={"对应模型内容信息":{"score_group_number": 40,
                                                                    "打分窗长": 25},
                                                    "原始包路径": "readme.yaml"}):
        """
            功能:
                    * 开始设置分组
            参数:
                    * /
            返回值:
                    * dict 分组信息 output_groups_dict
        """
        self.logger.log("[SetGroups]: 提示! 开始自动分组", "info")
        acoustic_model_info["对应模型内容信息"]["打分窗长"] = self.get_decoder_window_len(self.internal_msg_bus["模型信息"])
        try:
            output_groups_dict = {}
            include_word_dict = self.get_include_word()
            #最大包含关系的数量
            max_include_num = 0
            #包含关系的数量列表
            include_key_num_dict = {}
            for key in include_word_dict:
                cur_key_include_word_list = self._collect_all_keys(key, include_word_dict, [])
                include_key_num_dict[key] = len(cur_key_include_word_list)
                if len(cur_key_include_word_list) > max_include_num:
                    max_include_num = len(cur_key_include_word_list)

            include_key_num_dict = dict(sorted(include_key_num_dict.items(), key=lambda x: x[1], reverse=True))
            include_key_list = list(include_key_num_dict.keys())
            # 以包含关系词个数降序的列表
            #print(include_key_list)

            words_list = self.get_words_list()
            words_list_len = len(words_list)
            #分组数
            group_num = (words_list_len + acoustic_model_info["对应模型内容信息"]["score_group_number"] - 1) // acoustic_model_info["对应模型内容信息"]["score_group_number"]
            #print("分组数: ", group_num)
            avg_group_size  = words_list_len // group_num

            per_group_num_list = []
            #包含关系的词太多，超过平均分组个数情况
            if max_include_num > avg_group_size:
                if max_include_num <= int(acoustic_model_info["对应模型内容信息"]["score_group_number"]):
                    group_num = (words_list_len + max_include_num - 1) // max_include_num # 分组数
                    for i in range(group_num):
                        if (i+1) * max_include_num < words_list_len:
                            per_group_num_list.append(max_include_num)
                        else:
                            per_group_num_list.append(words_list_len - (i * max_include_num))
                    self.logger.log("[SetGroups]: 警告! 请注意查看下算力是否充足", "warning")
                else:
                    group_num = 1
                    per_group_num_list.append(words_list_len)
                    print_infor = "具有包含关系的词分进一组的要 %d 个, 超过最大分组数，将不进行分组。需要手动处理\n"%max_include_num
                    print_infor += "可以尝试设置模型包中的 Readme.yml 文件 score_group_number 字段的分组数为 %s 但需要注意算力!\n"%max_include_num
                    print_infor += "Readme.yml 配置文件路径是: %s"%os.path.join(acoustic_model_info["原始包路径"], "Readme.yml")
                    self.logger.log("[SetGroups]: 警告! " + print_infor, "warning")

            else:
                total_num = 0
                per_group_num = words_list_len // group_num
                per_group_num_list.append(per_group_num)
                total_num += per_group_num

                if group_num > 1:
                    if (words_list_len % group_num) != 0:
                        per_group_num = per_group_num + 1

                    for i in range(1, group_num - 1):
                        total_num += per_group_num
                        per_group_num_list.append(per_group_num)
                    per_group_num_list.append(words_list_len - total_num)
                    total_num += words_list_len - total_num

                assert total_num == words_list_len, "分组总数不匹配"

            if group_num > 4:
                self.logger.log(f'''[SetGroups]: 警告! 每组 {acoustic_model_info["对应模型内容信息"]["score_group_number"]}, 分组数为{group_num}, 超过4!''', "warning")

            print_infor = f"总个数: {words_list_len}, 分组数: {group_num}，"
            print_infor += f"每组个数: {per_group_num_list}, 打分窗长: {acoustic_model_info['对应模型内容信息']['打分窗长']}"
            self.logger.log("[SetGroups]: 提示! " + print_infor, "info")

            group_list = self._set_group(include_key_list, include_word_dict, group_num, per_group_num_list)

            output_groups_dict["总个数"] = words_list_len
            output_groups_dict["分组数"] = group_num
            output_groups_dict["每组个数列表"] = per_group_num_list
            output_groups_dict["打分窗长"] = acoustic_model_info['对应模型内容信息']['打分窗长']
            output_groups_dict["分组列表"] = group_list
            output_groups_dict["词延时及分级信息"] = self._get_grouped_cmd_delay_dict(group_num)
            self.logger.log("[SetGroups]: 提示! 分组后的信息", "debug")
            self.logger.log(output_groups_dict, "debug")
            return output_groups_dict

        except Exception as e:
            traceback.print_exc()
            self.logger.log(output_groups_dict, "info")
            sys.exit(40)


    def _get_grouped_cmd_delay_dict(self, group_num):
        """
            功能:
                    * /
                    * 对词设置delay时间
                    * 根据分组数设置不同的delay时间
            参数：
                    *
            返回值:
                    *
        """
        output_cmd_delay_dict = {}
        # 根据分组数设置不同的delay时间
        delay_context = 4
        if group_num == 1:
            delay_context = 4
        elif group_num == 2:
            delay_context = 2
        elif group_num > 2:
            delay_context = 1

        for cmd in self.get_words_list():
            if cmd in self.get_major_words_list():      # 如果是主唤醒词
                output_cmd_delay_dict[cmd] = '1'
            elif cmd in self.get_immediate_words_list(): # 如果是免唤醒词
                # if self.viva_config.get_words_obj.get_immediate_words_list[cmd] in delay_word_list:
                #     output_cmd_delay_dict[cmd] = '2 | (4<<16)'
                # else:
                output_cmd_delay_dict[cmd] = '2'
            elif cmd in self.get_delay_word():          # 若这个词需要延迟，并且非前面已设置好的免唤醒词，也不是主唤醒词
                output_cmd_delay_dict[cmd] = f"0| ({delay_context} << 16)"
            else:
                output_cmd_delay_dict[cmd] = '0'

        # for k,v in output_cmd_delay_dict.items():
        #     print(k, v)
        #自学习退出指令，分级设置4
        #exit_learn_cmd_list = self.viva_config.parsed_self_learn_config.get("exit_learn_cmd_list")
        exit_learn_cmd_list = []
        if not exit_learn_cmd_list:
            exit_learn_cmd_list = {}
        for cmd in output_cmd_delay_dict:
            if cmd in exit_learn_cmd_list:
                output_cmd_delay_dict[cmd] = '4'
        return output_cmd_delay_dict

#############################################################

    def get_word_max_len(self):
        """
            功能:
                    * 获取所有指令词的最大长度
            参数:
                    * /
            返回值:
                    * int 指令词最大长度 word_max_len
        """
        word_max_len = 0
        for i in self.get_words_list():
            if (len(i) > 8) or (len(i) < 2):   # 检查指令词个数是否在2-8之间
                raise ValueError("\033[0;36;31m[SetGroups]: 错误! 指令字的长度不在2到8个字符之间\033[0m")
            word_max_len = word_max_len if word_max_len > len(i) else len(i)
        return word_max_len


    def get_decoder_window_len(self, acoustic_model_info):
        """
            功能:
                    * 设置 decoder_window_len
            参数:
                    * /
            返回值:
                    * int  decoder_window_len
        """
        # 少于6个字，按照说单个字耗时按375ms计算取整，注意帧长按照每帧10ms。
        # 6个字按 2.2s, 7个字2.35s , 8个字2.5s 设置
        decoder_window_len = 0
        word_max_len = self.get_word_max_len()
        if word_max_len < 6:
            decoder_window_len = int(word_max_len * 375 / (acoustic_model_info["input_stride"] * 10))
        elif word_max_len == 6:
            decoder_window_len = int(2200 / (acoustic_model_info["input_stride"] * 10))
        elif word_max_len == 7:
            decoder_window_len = int(2350 / (acoustic_model_info["input_stride"] * 10))
        elif word_max_len == 8:
            decoder_window_len = int(2500 / (acoustic_model_info["input_stride"] * 10))
        elif word_max_len == 9:
            decoder_window_len = int(2600 / (acoustic_model_info["input_stride"] * 10))
        elif word_max_len == 10:
            decoder_window_len = int(2600 / (acoustic_model_info["input_stride"] * 10))
        else:
            raise ValueError("\033[0;36;31m[SetGroups]: 错误! 指令词的长度超过10\033[0m")
        self.logger.log(f"[SetGroups]: 调试! 计算打分窗长: {decoder_window_len}", "debug")
        return decoder_window_len



    def get_major_words_list(self):
        """
            功能:
                    * 获取主唤醒词列表
            参数:
                    * /
            返回值:
                    * list:
        """
        output_list = []
        for index in self.internal_msg_bus["词信息"]:
            if self.internal_msg_bus["词信息"][index]["是否主唤醒"]:
                output_list.append(self.internal_msg_bus["词信息"][index]["词"])
        return output_list


    def get_immediate_words_list(self):
        """
            功能：
                    * 获取免唤醒列表
            参数:
                    * /
            返回值:
                    * list: 免唤醒列表
        """
        return []



    def get_command_kv_dict(self):
        """
            功能:
                    * 获取 指令词和其 kv 值对应的指令词词典
            参数:
                    * /
            返回值:
                    * /
        """
        output_dict = {}
        for index in self.internal_msg_bus["词信息"]:
            output_dict[self.internal_msg_bus["词信息"][index]["词"]] = self.internal_msg_bus["词信息"][index]["事件ID"]
        return output_dict

    def get_words_list(self):
        """
            功能:
                    * 获取词列表
            参数：
                    * /
            返回值:
                    * list 全部词列表
        """
        return list(self.get_command_kv_dict().keys())

    def _is_include_word(self, str1, str2):
        """
            功能:
                * 判断两个词是否具有包含关系或仅有一个字不同
        """
        if str1 in str2:
            return True
        similarity_ratio = difflib.SequenceMatcher(None, str1, str2).ratio()
        ratio_th = ((len(str1) + len(str2)) - 2) / (len(str1) + len(str2))
        return similarity_ratio >= ratio_th


    def _add_to_include_dict(self, include_word_dict, key, value):
        """
            功能:
                    * 将键值对添加到包含关系字典中
        """
        if key not in include_word_dict:
            include_word_dict[key] = OrderedDict([(value, None)])
        else:
            include_word_dict[key][value] = None

    def get_include_word(self):
        """
            功能:
                    * 获取具有包含关系的指令词
            参数:
                    * /
            返回值:
                    * dict include_word_dict
        """
        include_word_dict = OrderedDict()
        word_list = self.get_words_list()
        word_dict = self.get_command_kv_dict()
        for i in word_list:
            for j in word_list:
                # 满足包含关系但id一致的词不参与分组
                if i != j and word_dict[i] != word_dict[j]:
                    # 检查是否满足延迟关系或字符串相似度条件
                    if ((i in j and not j.endswith(i)) or
                        (len(i) == 3 and (j.startswith(i[-2:]) or j.startswith(i[:2]))) or
                        self._is_include_word(i, j)):
                        self._add_to_include_dict(include_word_dict, i, j)
                        self._add_to_include_dict(include_word_dict, j, i)
        # 将OrderedDict的值转换为列表
        for key in include_word_dict:
            include_word_dict[key] = list(include_word_dict[key].keys())
        #for key, value in include_word_dict.items():
        #     print(key, value)
        return include_word_dict



    def get_delay_word(self):
        """
            功能:
                    * 获取 需要延迟的指令词列表, 指令词在id不一样的时候且满足下面要求
                        1. 一个词被其它词包含，但不在结尾部分，则这个词需要延迟。例：红茶， 红茶模式。开灯，学习开灯指令
                        2. 对于3个字的指令词，末尾或开头两个字是其它指令词的开头部分，则这个词需要延迟。例：定时十一小时， 不定时
            参数:
                    * /
            返回值:
                    * list dlay_word_list
        """
        dlay_word_list = set()
        word_list = self.get_words_list()
        word_dict = self.get_command_kv_dict()

        for i in word_list:
            for j in word_list:
                if i != j and word_dict[i] != word_dict[j]:
                    if i in j and not j.endswith(i):
                        dlay_word_list.add(i)
                    elif len(i) == 3:
                        if j.startswith(i[-2:]) and j not in dlay_word_list:
                            dlay_word_list.add(i)
                        elif j.startswith(i[:2]) and j not in dlay_word_list:
                            dlay_word_list.add(i)
        return list(dlay_word_list)


if __name__=="__main__":
    pass




