#!/usr/bin/env python3
# -*- coding: utf-8 -*-




import os, sys

try:
    from modules.common.debugLogger import DebugLogger
except:
    sys.path.append("../../")
    from modules.common.debugLogger import DebugLogger

__version__="V2.2.0"

import subprocess, sys

class Shell:
    """ 执行 shell 命令类 """
    def __init__(self, logger=None):
        self.logger = logger
        if not self.logger:
            self.logger = DebugLogger()


    def Run(self, cmd:str, env = None):
        '''
        执行 shell 命令.

        Args:
            cmd (str): shell 执行命令.
            env (dict): 传递给shell的环境变量,非必填参数.

        Returns:
            returncode: shell 执行后的返回码,一般0代表成功,非0即失败.
            stderr: 标准错误输出文本
            stdout: 标准输出文本

        Raises:
            无
        '''
        self.logger.log(f"[Shell]: 提示! 命令: {cmd}", "info")

        if sys.version_info.major == 4 and sys.version_info.minor >= 0:
            # Python 4.0 或更高版本的代码
            result = None
            if env:
                result = subprocess.run(cmd, \
                    shell=True, text=True, env=env)
            else:
                result = subprocess.run(cmd, \
                    shell=True, text=True)
            #print(result.stdout)  # 打印标准输出
            #print(result.stderr)  # 打印标准错误输出
            return result, result.stdout, result.stderr
        else:
            # Python 3.7 以下版本的代码
            process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output, error = process.communicate()
            output = output.decode('utf-8', "ignore")  # 根据实际编码进行解码
            error = error.decode('utf-8', "ignore")  # 根据实际编码进行解码
            #return output, error, process.returncode
            #print(output)
            return process, output, error


if __name__=="__main__":
    shell = Shell()
    shell.Run("ls")


