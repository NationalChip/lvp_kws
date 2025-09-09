#!/usr/bin/python3
#-*- coding:utf-8 -*-


__version__ = "v2.0.0"


import argparse, sys, os, pprint
from modules.common.debugLogger import DebugLogger
from modules.common.pathProcessing import PathProcessing
from modules.common.getWordInfo import GetWordsConfig
from modules.deploymentAcousticModel.basicDeployment import BasicDeployment
from modules.createBulidConfig.rebulidBunKwsLvpAutoconf import RebulidBunKwsLvpAutoconf
from modules.createBulidConfig.rebulidAlchemyLvpAutoconf import RebulidAlchemyLvpAutoconf


class KwsModelAutoDeploy:
    def __init__(self, inputPath, sdkRootPath, projectName, deployVersion,
                    deployProjectPath, logger, deploymentMode="bunkws"):
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
                                "部署模式":deploymentMode,
                                "当前工具路径": os.getcwd(),
                                "KWS 原始模型包路径": self.PathProcessing.get_absolute_path(inputPath),
                                "SDK 根路径": self.PathProcessing.get_absolute_path(sdkRootPath),
                                "SDK 类型": "LVP",
                                "输入文件": {},
                                "项目名称": projectName,
                                "模型自定义部署版本": deployVersion,
                                "模型部署项目路径": deployProjectPath,
                                "模型部署路径": os.path.join(deployProjectPath, deployVersion),
                                "模型信息": {"input_stride": None}
                                }
        self.deploymentMode_to_info_map = {"alchemy": self.get_alchemy_input_info,
                                        "bunkws": self.get_bunkws_input_info,}
        self.deploymentMode_to_info_map.get(deploymentMode.lower())(self.internal_msg_bus["KWS 原始模型包路径"])


    def get_bunkws_input_info(self, inputPath):
        """
            功能:
                    * 获取输入文件路径信息(BunKws 输出的模型文件目录路径)
            参数:
                    * 输入文件目录路径
            返回值:
                    * internal_msg_bus
        """
        self.internal_msg_bus["输入文件"] = {
                                    "模型报告文件": self.PathProcessing.get_file_list(inputPath, "report.xlsx")[0],
                                    "词有序列表文件": self.PathProcessing.get_file_list(inputPath, "keyword.txt")[0],
                                    "模型文件": self.PathProcessing.get_file_list(inputPath, "model_grus.h")[0],
                                    }
        fail_flag = 0
        for key in self.internal_msg_bus["输入文件"]:
            if not os.path.exists(self.internal_msg_bus["输入文件"][key]):
                self.logger.log(f'[KwsModelAutoDeploy]: 错误！缺失{key}: {self.internal_msg_bus["输入文件"][key]}', "error")
                fail_flag = 1
        if fail_flag:
            sys.exit()

        self.get_words_config = GetWordsConfig(self.logger, self.internal_msg_bus)
        if not self.get_words_config.Info():
            raise ValueError(f'\033[0;36;31m[KwsModelAutoDeploy]: 词信息获取失败，请检查 {self.internal_msg_bus["输入文件"]} 文件中词和表数据是否对其以及正确\033[0m')
        self.logger.log(f'[KwsModelAutoDeploy]: 调试! 获取的数据: {pprint.pformat(self.internal_msg_bus)}', "debug")


    def get_alchemy_input_info(self, inputPath):
        """
            功能:
                    * 获取输入文件路径信息(Alchemy 输出的模型文件目录路径)
            参数:
                    * 输入文件目录路径
            返回值:
                    * internal_msg_bus
        """
        self.internal_msg_bus["输入文件"] = {
                                    #"模型报告文件": self.PathProcessing.get_file_list(inputPath, "阈值表.xlsx$")[0],
                                    "词有序列表文件": self.PathProcessing.get_file_list(inputPath, "cmd_id.txt")[0],
                                    "模型文件": self.PathProcessing.get_file_list(inputPath, "model_grus.h$")[0],
                                    }
        fail_flag = 0
        for key in self.internal_msg_bus["输入文件"]:
            if not os.path.exists(self.internal_msg_bus["输入文件"][key]):
                self.logger.log(f'[KwsModelAutoDeploy]: 错误！缺失{key}: {self.internal_msg_bus["输入文件"][key]}', "error")
                fail_flag = 1
        if fail_flag:
            sys.exit()

        self.get_words_config = GetWordsConfig(self.logger, self.internal_msg_bus)
        if not self.get_words_config.Info():
            raise ValueError(f'\033[0;36;31m[KwsModelAutoDeploy]: 词信息获取失败，请检查 {self.internal_msg_bus["输入文件"]} 文件中词和表数据是否对其以及正确\033[0m')
        self.logger.log(f'[KwsModelAutoDeploy]: 调试! 获取的数据: {pprint.pformat(self.internal_msg_bus)}', "debug")


    def Start(self):
        """
            功能:
                    * 主要部署逻辑入口
            参数:
                    * /
            返回值:
                    * /
        """
        #展示词信息
        self.logger.log(f'[KwsModelAutoDeploy]: 本次模型部署词信息概览', "warning")
        self.logger.log(self.internal_msg_bus["词信息"] , "warning")
        #部署声学模型
        basic_deployment = BasicDeployment(self.internal_msg_bus, self.logger)
        basic_deployment.Start()
        #生成编译配置
        bulid_handle_map = {"bunkws":RebulidBunKwsLvpAutoconf,
                            "alchemy":RebulidAlchemyLvpAutoconf}
        bulid = bulid_handle_map.get(self.internal_msg_bus["部署模式"])(self.internal_msg_bus, self.logger)
        #bulid = RebulidLvpAutoconf(self.internal_msg_bus, self.logger)
        bulid.Start()
        return True


def func_BunKwsDeploymentMode(args, DeploymentMode="bunkws"):
    """
        功能:
                * 执行 bunkws 模型自动部署 SDK 入口
        参数:
                * args: 命令行参数
        返回值:
                * /
    """
    #参数检查
    if args.debug:
        logger = DebugLogger("debug")
    else:
        logger = DebugLogger("info")
    logger.log(f'[KwsModelAutoDeploy]: 提示！开始部署 {DeploymentMode} 模型至 LVP SDK', "info")

    fail_flag = 0
    if not args.inputPath:
        fail_flag = 1
        logger.log(f'[KwsModelAutoDeploy]: 错误！缺失 -i/--inputPath 输入 BunKws 输出的模型文件目录路径', "error")

    if not args.projectName:
        fail_flag = 1
        logger.log(f'[KwsModelAutoDeploy]: 错误！缺失 -p/--projectName 为设置项目名称', "error")

    path_processing = PathProcessing(logger)
    sdkRootPath = path_processing.get_absolute_path(args.sdkRootPath)
    if not sdkRootPath:
        fail_flag = 1
        logger.log(f'[KwsModelAutoDeploy]: 错误！缺失/错误输入 -r/--sdkRootPath SDK 根路径', "error")
    #版本号检查
    args.projectName = args.projectName.lower()
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
    if fail_flag:
        sys.exit()

    at = KwsModelAutoDeploy(args.inputPath, args.sdkRootPath, args.projectName,
                    args.deployVersion, model_deploy_project_path, logger, DeploymentMode)
    at.Start()

def func_AlchemyDeploymentMode(args):
    """
        功能:
                * 执行 alchemy 模型自动部署 SDK 入口
        参数:
                * args: 命令行参数
        返回值:
                * /
    """
    return func_BunKwsDeploymentMode(args, DeploymentMode="alchemy")



###################################################################################
def main():
    update_time = "Update time: 2025-08-28\nauthor: liushk"
    version_info = "\nversion: " + str(__version__) + "\n" + update_time
    tool_description = "这是一个可以把 Kws 模型训练框架生成的模型，自动部署到 lvp SDK 内的工具"
    parser = argparse.ArgumentParser(description=tool_description, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-v', '-V', '--version', action='version', version=version_info,
                        help='Print software version info')
    # 创建子命令解析器
    subparsers = parser.add_subparsers(title='subcommands', dest='subcommand')

    # 定义一个函数来添加共享的参数
    def add_common_bunkws_arguments(subparser):
        subparser.add_argument('-r', '--sdkRootPath', type=str, default=None,
                                help="""SDK 跟路径，该路径是一切操作的基础路径
\033[0;36;33m注意：
    1、该路径以 lvp_tws 为止
        例如:  ~/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws
                                \033[0m""")
        subparser.add_argument('-p', '--projectName', type=str, default=None,
                                help="""设置项目名称 (后续模型会部署到 SDK 中以该项目名的目录中)
\033[0;36;33m注意:
    1、名称可以由字母和下划线组成，不允许其他字符
        例如: xiaoshu_ai
                                \033[0m""")
        subparser.add_argument('-dv', '--deployVersion', type=str, default="v0.1.0",
                                help="""自定义部署的项目模型版本 (主要是给部署到 SDK 的模型定义一个版本)
\033[0;36;33m注意:
    1、可以不设置，默认 v0.1.0，如果当前项目下有版本重复，工具运行过程中会跳出是否覆盖提示
    2、如果设置的版本在当前项目下有版本重复，工具运行过程中会跳出是否覆盖提示
                                \033[0m""")
        subparser.add_argument('-d', '--debug', action="store_true", default=False,
                                help="""
使能调试模式，默认不打开、主要调整打印等级为 debug, 输出详细打印，用于调试""")

    cmd_BunKwsDeploymentMode = subparsers.add_parser('BunKwsDeploymentMode',
                                           help="针对 BunKws 训练平台输出的模型，自动部署到 SDK 模式",
                                           aliases=["bunkwsdeploymentmode"],
                                            formatter_class=argparse.RawTextHelpFormatter)

    cmd_BunKwsDeploymentMode.add_argument('-i', '--inputPath', dest="inputPath", default=None,
                                help="""输入 BunKws 模型训练框架输出的模型文件目录路径
\033[0;36;33m注意：
    1、该目录下应包含如下文件
    model_79.pt
    ├── keyword.txt     # 词有序列表 (主要给定顺序)
    ├── model_grus.h    # 模型文件
    └── report.xlsx     # 模型测试表 (主要提供阈值)
                                \033[0m""")
    add_common_bunkws_arguments(cmd_BunKwsDeploymentMode)
    cmd_BunKwsDeploymentMode.set_defaults(func=func_BunKwsDeploymentMode)

    cmd_AlchemyDeploymentMode = subparsers.add_parser('AlchemyDeploymentMode',
                                           help="针对 Alchemy 训练平台输出的模型，自动部署到 SDK 模式",
                                           aliases=["alchemydeploymentmode"],
                                            formatter_class=argparse.RawTextHelpFormatter)
    cmd_AlchemyDeploymentMode.add_argument('-i', '--inputPath', dest="inputPath", default=None,
                                help="""输入 Alchemy 模型训练框架输出的模型文件目录路径
\033[0;36;33m注意：
    1、该目录下应包含如下文件
    alchemy_平台输出_最新
    ├── cmd_id.txt      # 指令信息文件 (包含指令、指令lable、阈值、是否主唤醒)
    └── model_grus.h    # 模型文件
                                \033[0m""")
    add_common_bunkws_arguments(cmd_AlchemyDeploymentMode)
    cmd_AlchemyDeploymentMode.set_defaults(func=func_AlchemyDeploymentMode)




    # 设置子命令名不区分大小写任意组合
    if len(sys.argv) > 1:
        sys.argv[1] = sys.argv[1].lower()  # 将第一个参数（子命令名称）转换为小写

    # 解析命令行参数
    args = parser.parse_args()
    if not args.subcommand:
        parser.print_help()
    else:
        print(args)
        args.func(args)

if __name__ == "__main__":
    main()

