#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import os, re, sys
import shutil
from datetime import datetime

try:
    from modules.common.shell import Shell
except ImportError:
    sys.path.append("../../")
    from modules.common.shell import Shell

__version__ ="1.0.0"

class RebulidLvpAutoconf:
    """ 这是自动生成LVP SDK 编译配置的类"""
    def __init__(self, internal_msg_bus={}, logger=None):
        # 初始化全局变量
        self.parsed_all_models_info = {}
        #工具内部信息
        self.internal_msg_bus = internal_msg_bus
        #log句柄
        self.logger = logger
        #执行shell
        self.shell = Shell(self.logger)

        # 要配置的工程中.config路径
        self.output_config_path = os.path.join(self.internal_msg_bus["SDK 根路径"], '.config')
        self.defconfig = os.path.join(self.internal_msg_bus["SDK 根路径"],
                                            "configs/kws_model_auto_deploy_config/kws_model_auto_deploy.config")
        self.logger.log(f"[RebulidLvpAutoconf]: 提示! 使用的默认配置为: {self.defconfig}", "info")



    def delete_config_from_file(self, config_info, path):
        """
        功能：
                删除指定路径中相关配置
        参数：
                config_info: 要删除的config信息列表
                path: 要删除的文件路径
        返回:
                bool True/Flase
        """

        with open(path, "r") as fd:
            lines = fd.readlines()

        lines_to_keep = []
        for line in lines:
            if not any(config in line for config in config_info):
                lines_to_keep.append(line)

        with open(path, 'w') as file:
            file.writelines(lines_to_keep)

    def write_config_to_file(self, config_info, path, find_str):
        """
            功能:
                    * 配置写入
            参数:
                    * /
            返回值:
                    * /
        """
        with open(path, "r") as fd:
            data = fd.read()
        # 找到匹配字符串的位置
        str_index = data.find(find_str)
        start_index = data[:str_index].rfind("\n")
        end_index = data[str_index:].find("\n")
        write_data = data[:start_index+1]
        for config in config_info:
            for i in config:
                write_data += i
        write_data += data[start_index + (str_index - start_index) + end_index + 1:]
        with open(path, "w") as fd:
            fd.write(write_data)


    def set_a_config(self, config_str, config_value):
        """
            功能:
                    * 批量修改 .config 和 bautoconf.h 指定的内容
            参数:
                    * 项的关键字 config_str
                    * 修改后的值 config_value
            返回值:
                    * /
        """
        dot_config = [[f"CONFIG_{config_str}={config_value}\r\n"]]
        self.write_config_to_file(dot_config, self.output_config_path, config_str)



    def generate_defconfig(self):
        """
            功能:
                    * 生成默认配置
            参数:
                    *
            返回值:
                    * /
        """
        start_path = os.getcwd()
        os.chdir(self.internal_msg_bus["SDK 根路径"])
        process, output, error = self.shell.Run("make defconfig")
        if process.returncode != 0:
            raise ValueError(f"\033[0;36;31m[RebulidOvpAutoconf]: 错误! 生成 autoconf.h 失败! 原因: {error}\033[0m")
        os.chdir(start_path)


    def Start(self):
        """
            功能:
                    * 创建 .config 入口
            参数:
                    * /
            返回值:
                    * /
        """
        #0、拷贝默认配置
        shutil.copyfile(self.defconfig, self.output_config_path)

        #1、写入 项目名称 信息
        project_name = self.internal_msg_bus["项目名称"]
        project_name = f"CONFIG_LVP_KWS_{project_name.upper()}"
        dot_config = [[f"{project_name}=y\r\n"]]
        self.write_config_to_file(dot_config, self.output_config_path, "CONFIG_LVP_KWS_XIN_DA_LU=y")

        #2、写入 模型版本 信息
        merge_version = self.internal_msg_bus["模型自定义部署版本"]
        merge_version = f"{project_name.upper()}_{merge_version.replace('.', 'DOT').upper()}_{datetime.now().strftime('%Y_%m%d')}"
        dot_config = [[f"{merge_version}=y\r\n"]]
        self.write_config_to_file(dot_config, self.output_config_path, "CONFIG_LVP_KWS_XIN_DA_LU_V0DOT1DOT5_2024_0806=y")

        #3、写入 NPU_SRAM_SIZE 信息
        npu_sram_size = str(self.internal_msg_bus["模型关键信息"]["NPU_SRAM_SIZE"]).strip()
        if int(npu_sram_size) < 12:
            npu_sram_size = 12
        dot_config = [[f"CONFIG_NPU_SRAM_SIZE_KB={str(npu_sram_size)}\r\n"]]
        self.write_config_to_file(dot_config, self.output_config_path, "CONFIG_NPU_SRAM_SIZE_KB=60")

        # .config 生成  include/autoconf.h
        self.generate_defconfig()
        self.logger.log(f"[RebulidLvpAutoconf]: 提示! 生成在 LVP SDK 工程下生成: {self.output_config_path} 配置成功!!!", "info")




if __name__=="__main__":
    pass


