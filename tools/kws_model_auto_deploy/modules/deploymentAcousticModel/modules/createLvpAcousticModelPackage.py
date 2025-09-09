#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#from __future__ import unicode_literals
import os, sys


try:
    from modules.deploymentAcousticModel.modules.createBunKwsToLvpFile import CreateBunKwsAcousticModelPackageFile
    from modules.deploymentAcousticModel.modules.createAlchemyToLvpFile import CreateAlchemyAcousticModelPackageFile
except:
    sys.path.append("../../../")
    from modules.deploymentAcousticModel.modules.createBunKwsToLvpFile import CreateBunKwsAcousticModelPackageFile
    from modules.deploymentAcousticModel.modules.createAlchemyToLvpFile import CreateAlchemyAcousticModelPackageFile

__version__="3.0.0"


class CreateLvpAcousticModelPackage:
    """这个是管理创建 LVP 声学模型包的类"""
    def __init__(self, internal_msg_bus, logger):
        #工具内部消息句柄
        self.internal_msg_bus = internal_msg_bus
        #log句柄
        self.logger = logger


    def CreateBunKwsAcousticModelPackage(self, acoustic_model_parsed_info):
        """
            功能:
                    * 创建支持 LVP SDK 的声学模型包 (仅仅支持 normal_mode 模式)
            参数:
                    * dict: 解析后声学模型包文件: acoustic_model_parsed_info
            返回值:
                    * /
        """
        self.logger.log("[CreateLvpAcousticModelPackage]: 提示! 生成 LVP BunKws 声学模型包!", "info")
        #创建普通模型下的声学模型包文件
        self.create_lvp_package_handle = CreateBunKwsAcousticModelPackageFile()
        #1、创建模型包目录
        lvp_path = self.internal_msg_bus["模型部署项目路径"]
        if not lvp_path:
            return False
        os.makedirs(lvp_path, exist_ok=True)

        #2、创建 LVP kws.name
        kws_name_filename = os.path.join(lvp_path,"kws.name")
        if not os.path.exists(kws_name_filename):
            #没有才创建
            self.logger.log(f"[CreateLvpAcousticModelPackage]: 调试! {kws_name_filename} 没有，将会创建", "debug")
            self.create_lvp_package_handle.create_kws_name(kws_name_filename,
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        #3、创建 LVP kws_version.list
        kws_version_filename = os.path.join(lvp_path,"kws_version.list")
        if not os.path.exists(kws_version_filename):
            #没有才创建
            self.logger.log(f"[CreateLvpAcousticModelPackage]: 调试! {kws_version_filename} 没有，将会创建", "debug")
            self.create_lvp_package_handle.create_kws_version_list(kws_version_filename,
                                                            acoustic_model_parsed_info, self.internal_msg_bus)


        #4、创建以路径版本命名的核心目录，存储声学模型文件
        lvp_model_version_path = os.path.join(lvp_path, str(self.internal_msg_bus["模型自定义部署版本"]))
        os.makedirs(lvp_model_version_path, exist_ok=True)

        #5、创建 LVP kws_list.h
        self.create_lvp_package_handle.create_kws_list_h(os.path.join(lvp_model_version_path, "kws_list.h"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)
        #6、创建声学模型文件
        self.create_lvp_package_handle.create_model_h(os.path.join(lvp_model_version_path, "model.h"),
                                                    acoustic_model_parsed_info, self.internal_msg_bus)

        #7、创建 CTC model .c 和 .h 文件
        self.create_lvp_package_handle.create_ctc_model_c(os.path.join(lvp_model_version_path,"ctc_model.c"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)
        self.create_lvp_package_handle.create_ctc_model_h(os.path.join(lvp_model_version_path, "ctc_model.h"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        #8、创建 kconfig 文件
        self.create_lvp_package_handle.create_kconfig(os.path.join(lvp_model_version_path,"Kconfig"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)
        #9、创建 kws_version.name 文件
        self.create_lvp_package_handle.create_kws_version_name(os.path.join(lvp_model_version_path,"kws_version.name"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        ##10、创建 makefile 文件
        self.create_lvp_package_handle.create_makefile(os.path.join(lvp_model_version_path,"Makefile"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)
        self.logger.log("[CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!", "info")
        return True


    def CreateAlchemyAcousticModelPackage(self, acoustic_model_parsed_info):
        """
            功能:
                    * 创建支持 LVP SDK 的声学模型包 (仅仅支持 normal_mode 模式)
            参数:
                    * dict: 解析后声学模型包文件: acoustic_model_parsed_info
            返回值:
                    * /
        """

        self.logger.log("[CreateLvpAcousticModelPackage]: 提示! 生成 LVP  Alchemy 声学模型包!", "info")
        self.create_lvp_package_handle = CreateAlchemyAcousticModelPackageFile()
        #1、创建模型包目录
        lvp_path = self.internal_msg_bus["模型部署项目路径"]
        if not lvp_path:
            return False
        os.makedirs(lvp_path, exist_ok=True)

        #2、创建 LVP kws.name
        kws_name_filename = os.path.join(lvp_path,"kws.name")
        if not os.path.exists(kws_name_filename):
            #没有才创建
            self.logger.log(f"[CreateLvpAcousticModelPackage]: 调试! {kws_name_filename} 没有，将会创建", "debug")
            self.create_lvp_package_handle.create_kws_name(kws_name_filename,
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        #3、创建 LVP kws_version.list
        kws_version_filename = os.path.join(lvp_path,"kws_version.list")
        if not os.path.exists(kws_version_filename):
            #没有才创建
            self.logger.log(f"[CreateLvpAcousticModelPackage]: 调试! {kws_version_filename} 没有，将会创建", "debug")
            self.create_lvp_package_handle.create_kws_version_list(kws_version_filename,
                                                            acoustic_model_parsed_info, self.internal_msg_bus)

        #4、创建 LVP kws_list.h
        kws_list_h_filename = os.path.join(lvp_path, "kws_list.h")
        self.create_lvp_package_handle.create_kws_list_h(kws_list_h_filename,
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        #5、创建以路径版本命名的核心目录，存储声学模型文件
        lvp_model_version_path = os.path.join(lvp_path, str(self.internal_msg_bus["模型自定义部署版本"]))
        os.makedirs(lvp_model_version_path, exist_ok=True)

        #6、创建声学模型文件
        self.create_lvp_package_handle.create_model_h(os.path.join(lvp_model_version_path, "model.h"),
                                                    acoustic_model_parsed_info, self.internal_msg_bus)

        #7、创建 CTC model .c 和 .h 文件
        self.create_lvp_package_handle.create_ctc_model_c(os.path.join(lvp_model_version_path,"ctc_model.c"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)
        self.create_lvp_package_handle.create_ctc_model_h(os.path.join(lvp_model_version_path, "ctc_model.h"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        #8、创建 kconfig 文件
        self.create_lvp_package_handle.create_kconfig(os.path.join(lvp_model_version_path,"Kconfig"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus,
                                                        decoder_stride_len=self.internal_msg_bus["分组信息"]["分组数"],
                                                        decoder_window_len=self.internal_msg_bus["分组信息"]["打分窗长"])
        #9、创建 kws_version.name 文件
        self.create_lvp_package_handle.create_kws_version_name(os.path.join(lvp_model_version_path,"kws_version.name"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        ##10、创建 makefile 文件
        self.create_lvp_package_handle.create_makefile(os.path.join(lvp_model_version_path,"Makefile"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        ##11、创建 kws.list 文件
        self.create_lvp_package_handle.create_kws_list(os.path.join(lvp_model_version_path,"kws.list"),
                                                        acoustic_model_parsed_info, self.internal_msg_bus)

        self.logger.log("[CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!", "info")
        return True



