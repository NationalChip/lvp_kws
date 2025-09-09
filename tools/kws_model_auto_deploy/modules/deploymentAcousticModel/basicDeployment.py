#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#from __future__ import unicode_literals
import sys, copy


try:
    #1、获取模型数据
    from modules.deploymentAcousticModel.modules.getModelInfo import GetModelInfor
    #3、创建模型相关文件
    from modules.deploymentAcousticModel.modules.createLvpAcousticModelPackage import CreateLvpAcousticModelPackage
except ImportError:
    sys.path.append("../../")
    #1、获取模型数据
    from modules.deploymentAcousticModel.modules.getModelInfo import GetModelInfor
    #3、创建模型相关文件
    from modules.deploymentAcousticModel.modules.createLvpAcousticModelPackage import CreateLvpAcousticModelPackage

__version__ ="2.1.0"

class BasicDeployment:

    def __init__(self, internal_msg_bus={}, logger=None):

        # 初始化全局变量
        self.parsed_all_models_info = {}
        #工具内部信息
        self.internal_msg_bus = internal_msg_bus
        #log句柄
        self.logger = logger
        #创建对象
        self.create_lvp_package = CreateLvpAcousticModelPackage(self.internal_msg_bus, self.logger)



    def _get_acoustic_model_info(self, support_SoftMax, normal_ctc):
        """
            功能:
                    * 读取配置文件
            参数:
                    * bool 是否支持 SoftMax upport_SoftMax True/False
                    * bool 是否是 normal_ctc  True/False
            返回值:
                    * /
        """

        parsed_model_data = {
                    "project name": "",
                    "acoustics model version": "",
                    "sourcePath": "",
                    "stride": 4,
                    "windowLength": 17,
                    "supportSoftMax": False,
                    "userDecoder": False,
                    "word info":[],
                    "model_data":{}
        }

        #添加模型步进


        #项目名称
        parsed_model_data["project name"] = self.internal_msg_bus['项目名称']
        #模型版本
        parsed_model_data["acoustics model version"] = self.internal_msg_bus["模型自定义部署版本"]
        #模型文件路径
        parsed_model_data["sourcePath"] = self.internal_msg_bus["输入文件"]["模型文件"]
        #归一化参数文件路径
        parsed_model_data["normalized_path"] = ""

        try:
            obj = None
            self.get_model_info = GetModelInfor()
            obj = self.get_model_info.get_model_info(parsed_model_data["sourcePath"],
                                                            parsed_model_data["normalized_path"])

            self.logger.log("模型基础信息", "debug")
            self.logger.log(obj["info"], "debug")
            parsed_model_data["model_data"] = obj
            if obj["info"].get("window_length"):
                parsed_model_data["windowLength"] = obj["info"]["window_length"]
        except Exception as e:
            self.logger.log(e, "error")
            self.logger.log("[BasicDeployment]模型文件读取失败, 请确认文件格式和路径是否正确!", "error")
            sys.exit(9)

        parsed_model_data["supportSoftMax"] = support_SoftMax
        parsed_model_data["normal_ctc"] = normal_ctc

        if self.internal_msg_bus["模型信息"]["input_stride"]:
            parsed_model_data["stride"] = self.internal_msg_bus["模型信息"]["input_stride"]

        ppint_data = copy.deepcopy(parsed_model_data)
        ppint_data["model_data"]["res"] = "模型文件数据,过大暂不展开 ..."
        self.logger.log(ppint_data, "debug")
        del ppint_data
        return parsed_model_data




    def StartCreateModelPackage(self, threshold_dict={}):
        """
            功能:
                    * 判断 SDK 类型，生成支持对应类型的声学模型包
            参数:
                    * dict 阈值字典 threshold_dict
                    * 当为空的时候，kws_list.h 写入的是初始默认值
            返回值:
                    *
        """
        create_lvp_package_map = {"bunkws": self.create_lvp_package.CreateBunKwsAcousticModelPackage,
                "alchemy": self.create_lvp_package.CreateAlchemyAcousticModelPackage}
        if self.internal_msg_bus['SDK 类型']=="LVP":
            create_lvp_package_map.get(self.internal_msg_bus["部署模式"].lower())(
                        self.parsed_all_models_info[self.internal_msg_bus["模型自定义部署版本"]])
            #self.create_lvp_package.CreateLVPAcousticModelPackage(
            #        self.parsed_all_models_info[self.internal_msg_bus["模型自定义部署版本"]])
        else:
            self.logger.log("[BasicDeployment]: 错误! { self.internal_msg_bus['SDK 类型']} SDK 暂不支持部署", "error")


    def Start(self, upport_SoftMax=True, normal_ctc=True):
        """
            功能:
                * 自动部署模型入口
        """
        info = self._get_acoustic_model_info(upport_SoftMax, normal_ctc)
        self.parsed_all_models_info[self.internal_msg_bus["模型自定义部署版本"]] = info
        self.internal_msg_bus["模型关键信息"] = info["model_data"]["info"]
        self.StartCreateModelPackage()

