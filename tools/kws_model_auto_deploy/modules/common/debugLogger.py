#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from colorama import init, Fore, Style
import pprint

# 初始化 colorama 库，用于在控制台输出中添加颜色
init(autoreset=True)

class DebugLogger:
    """ 这是一个调试、运行记录器、控制打印"""
    def __init__(self, set_log_level="info"):
        """
        初始化一些参数

        Args:
            set_log_level: 设置日志输出的等级
        Returns:
                /
        """
        set_log_level = set_log_level.lower()
        self.log_level_dict = {"debug":logging.DEBUG, "info":logging.INFO,
                                "warning":logging.WARNING, "error":logging.ERROR,
                                "critical":logging.CRITICAL}
        if set_log_level not in self.log_level_dict:
            raise ValueError(f"\033[0;36;31m[DebugLogger]: 错误! 设置 {set_log_level} 日志等级不存在!\033[0m")


        self.logger_console = logging.getLogger('console_logger')  # 创建一个名为 console_logger 的打印输出记录器
        self.logger_console.setLevel(logging.DEBUG)  # 设置日志记录器的默认级别为 DEBUG
        # 创建控制台处理器
        console_handler = logging.StreamHandler()
        console_handler.setLevel(self.log_level_dict[set_log_level])  # 设置控制台处理器的日志级别
        #formatter = logging.Formatter('%(asctime)s: %(levelname)s: %(message)s')  # 创建日志格式化器
        formatter = logging.Formatter('%(levelname)s: %(message)s')  # 创建日志格式化器
        console_handler.setFormatter(formatter)  # 设置控制台处理器的日志格式

        self.logger_console.addHandler(console_handler)  # 将控制台处理器添加到日志记录器中

    def log(self, message, level):
        """
        日志信息输出打印

        Args:
            message:    需要打印输出的信息
            level:      信息等级
        Returns:
                /
        """
        level = level.lower()
        if level not in self.log_level_dict:
            level = "info"
        level = self.log_level_dict[level]
        try:
            # 记录日志信息的方法，接受两个参数：message 为要记录的消息内容，level 为日志级别
            color_map = {
                logging.DEBUG: Fore.CYAN,
                logging.INFO: Fore.GREEN,
                logging.WARNING: Fore.YELLOW,
                logging.ERROR: Fore.RED,
                logging.CRITICAL: Fore.MAGENTA
            }
            if level in color_map:
                if isinstance(message, dict) or isinstance(message, list):
                    self.logger_console.log(level, color_map[level] + pprint.pformat(message) + Style.RESET_ALL)
                else:
                    self.logger_console.log(level, color_map[level] + str(message) + Style.RESET_ALL)
        except Exception as e:
            print(f"\033[0;36;31m[DebugLogger]: 错误! 输出时出错: {e}\033[0m")


if __name__ == "__main__":
    # 使用示例
    logger = DebugLogger("DEBUG")
    data = {
        'name': 'Alice',
        'age': 30,
        'city': 'New York',
        'info list': [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    }
    logger.log([0, 2,3 ,4, 5, 6,7 ,8 ,9], "DEBUG")
    logger.log(data, "debug")
    logger.log("调试", "debug")
    logger.log("信息", "info")
    logger.log('警告!', "warning")
    logger.log("错误", "error")
    logger.log("严重错误", "critical")
