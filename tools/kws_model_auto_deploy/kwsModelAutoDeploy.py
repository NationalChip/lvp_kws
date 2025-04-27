#!/usr/bin/python3
#-*- coding:utf-8 -*-


__version__ = "v1.0.0"


import argparse, sys, os
from modules.common.debugLogger import DebugLogger
from modules.common.pathProcessing import PathProcessing
from modules.common.getWordThreshold import GetWordThreshold
from modules.deploymentAcousticModel.basicDeployment import BasicDeployment
from modules.createBulidConfig.rebulidLvpAutoconf import RebulidLvpAutoconf


class KwsModelAutoDeploy:
    def __init__(self, inputPath, sdkRootPath, projectName, deployVersion,
                    deployProjectPath, logger):
        """
            功能:
                    * 初始化参数
            参数:
                    * 输入文件目录路径(BunKws 输出的模型包)
                    * debug: 打印 debug 等级使能
        """
        self.logger = logger
        self.PathProcessing = PathProcessing(self.logger)
        #内部消息总线
        self.internal_msg_bus = {
                                "当前工具路径": os.getcwd(),
                                "KWS 原始模型包路径": self.PathProcessing.get_absolute_path(inputPath),
                                "SDK 根路径": self.PathProcessing.get_absolute_path(sdkRootPath),
                                "SDK 类型": "LVP",
                                "输入文件": {},
                                "项目名称": projectName,
                                "模型自定义部署版本": deployVersion,
                                "模型部署项目路径": deployProjectPath,
                                "模型部署路径": os.path.join(deployProjectPath, deployVersion)
                                }
        self.get_input_files_path(self.internal_msg_bus["KWS 原始模型包路径"])
        self.get_word_threshold = GetWordThreshold(self.logger, self.internal_msg_bus)

    def get_input_files_path(self, inputPath):
        """
            功能:
                    * 获取输入文件路径信息(BunKws 输出的模型文件目录路径)
            参数:
                    * 输入文件目录路径
            返回值:
                    * internal_msg_bus
        """
        self.internal_msg_bus["输入文件"] = {
                                    "模型报告文件": os.path.join(inputPath, "report.xlsx"),
                                    "词有序列表文件": os.path.join(inputPath, "keyword.txt"),
                                    "模型文件": os.path.join(inputPath, "model.h")
                                    }
        fail_flag = 0
        for key in self.internal_msg_bus["输入文件"]:
            if not os.path.exists(self.internal_msg_bus["输入文件"][key]):
                self.logger.log(f'[KwsModelAutoDeploy]: 错误！缺失{key}: {self.internal_msg_bus["输入文件"][key]}', "error")
                fail_flag = 1
        if fail_flag:
            sys.exit()


    def Start(self):
        """
            功能:
                    *
            参数:
                    *
            返回值:
                    *
        """
        word_threshold_info = self.get_word_threshold.GetThresholdData()
        if not word_threshold_info:
            self.logger.log(f'[KwsModelAutoDeploy]: 词信息获取失败，请检查 {self.internal_msg_bus["输入文件"]} 文件中词和表数据是否对其以及正确', "error")
            return False

        #展示词信息
        word_max_len = max([len(word_threshold_info[x]["词"]) for x in word_threshold_info])
        self.logger.log(f'[KwsModelAutoDeploy]: 本次模型部署词信息概览', "warning")
        for index in word_threshold_info:
            spaces_num = word_max_len + 1 - len(word_threshold_info[index]["词"])
            single_word_info = f'顺序: {index}\t词: {word_threshold_info[index]["词"]}{" "*spaces_num} 阈值: {word_threshold_info[index]["阈值"]}\t'
            single_word_info += f' 事件ID: {word_threshold_info[index]["事件ID"]} 是否为主唤醒: {word_threshold_info[index]["是否主唤醒"]}'
            self.logger.log(single_word_info, "warning")
        self.internal_msg_bus["词信息"] = word_threshold_info
        self.logger.log(self.internal_msg_bus, "debug")
        #部署声学模型
        basic_deployment = BasicDeployment(self.internal_msg_bus, self.logger)
        basic_deployment.Start()
        #生成编译配置
        bulid = RebulidLvpAutoconf(self.internal_msg_bus, self.logger)
        bulid.Start()
        return True



def main():
    update_time = "Update time: 2025-04-24\nauthor: liushk"
    version_info = "\nversion: " + str(__version__) + "\n" + update_time
    tool_description = "这是一个可以把 BunKws 训练框架生成的模型，自动部署到 lvp SDK 内的工具"
    parser = argparse.ArgumentParser(description=tool_description, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-v', '-V', '--version', action='version', version=version_info,
                        help='Print software version info')



    parser.add_argument('-i', '--inputPath', dest="inputPath", default=None,
                                help="""输入 BunKws 输出的模型文件目录路径
\033[0;36;33m注意：
    1、该目录下应包含如下文件
    model_79.pt
    ├── keyword.txt     # 词有序列表 (主要给定顺序)
    ├── model.h         # 模型文件
    └── report.xlsx     # 模型测试表 (主要提供阈值)
                                \033[0m""")
    parser.add_argument('-r', '--sdkRootPath', type=str, default=None,
                                help="""SDK 跟路径，该路径是一切操作的基础路径
\033[0;36;33m注意：
    1、该路径以 lvp_tws 为止
        例如:  ~/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws
                                \033[0m""")

    parser.add_argument('-p', '--projectName', type=str, default=None,
                                help="""设置项目名称 (后续模型会部署到 SDK 中以该项目名的目录中)
\033[0;36;33m注意:
    1、名称可以由字母和下划线组成，不允许其他字符
        例如: xiaoshu_ai
                                \033[0m""")

    parser.add_argument('-dv', '--deployVersion', type=str, default="v0.1.0",
                                help="""自定义部署的项目模型版本 (主要是给部署到 SDK 的模型定义一个版本)
\033[0;36;33m注意:
    1、可以不设置，默认 v0.1.0，如果当前项目下有版本重复，工具运行过程中会跳出是否覆盖提示
    2、如果设置的版本在当前项目下有版本重复，工具运行过程中会跳出是否覆盖提示
                                \033[0m""")

    parser.add_argument('-d', '--debug', action="store_true", default=False,
                                help="""
使能调试模式，默认不打开、主要调整打印等级为 debug, 输出详细打印，用于调试""")

    # 解析命令行参数
    args = parser.parse_args()

    #参数检查

    if args.debug:
        logger = DebugLogger("debug")
    else:
        logger = DebugLogger("info")
    path_processing = PathProcessing(logger)

    fail_flag = 0
    if not args.inputPath:
        fail_flag = 1
        logger.log(f'[KwsModelAutoDeploy]: 错误！缺失 -i/--inputPath 输入 BunKws 输出的模型文件目录路径', "error")

    if not args.projectName:
        fail_flag = 1
        logger.log(f'[KwsModelAutoDeploy]: 错误！缺失 -p/--projectName 为设置项目名称', "error")

    sdkRootPath = path_processing.get_absolute_path(args.sdkRootPath)
    if not sdkRootPath:
        fail_flag = 1
        logger.log(f'[KwsModelAutoDeploy]: 错误！缺失/错误输入 -r/--sdkRootPath SDK 根路径', "error")
    #版本号检查
    if not fail_flag:
        model_deploy_project_path = os.path.join(sdkRootPath, "lvp/vui/kws/models/", args.projectName)
        os.makedirs(model_deploy_project_path, exist_ok=True)
        model_deploy_project_path = path_processing.get_absolute_path(model_deploy_project_path)
        if model_deploy_project_path:
            version_already_exists = [x for x in os.listdir(model_deploy_project_path) if os.path.isdir(os.path.join(model_deploy_project_path, x))]
            version_already_exists.sort()
            if args.deployVersion in version_already_exists:
                logger.log(f'[KwsModelAutoDeploy]: 注意！{args.projectName} 项目已存在 并且设置的 {args.deployVersion} 版本已经存在', "warning")
                logger.log(f'[KwsModelAutoDeploy]: 注意！{args.projectName} 项目下，已有版本: {version_already_exists}', "warning")
                version_coverage = input(f"\033[0;36;33m是否覆盖已有的 {args.deployVersion} 版本? y/n\033[0m: ")
                if version_coverage.strip().lower() not in ["y", "yes"]:
                    logger.log(f'[KwsModelAutoDeploy]: 注意！你选择非覆盖，那请你重新启动工具，重新定义版本，并且非 {version_already_exists} 版本', "warning")
                    #fail_flag = 1
                    sys.exit()
    model_deploy_version_path = os.path.join(model_deploy_project_path, args.deployVersion)

    if fail_flag:
        print("="*80)
        parser.print_help()
        sys.exit()


    at = KwsModelAutoDeploy(args.inputPath, args.sdkRootPath, args.projectName,
                            args.deployVersion, model_deploy_project_path, logger)
    at.Start()

if __name__ == "__main__":
    main()
