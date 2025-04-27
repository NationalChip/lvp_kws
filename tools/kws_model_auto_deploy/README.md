# kwsModelAutoDeploy 工具
---
## 简介：
* 这是一个可以把 BunKws 训练框架生成的模型，自动部署到 `lvp` SDK 内的工具，使用该工具后，用户可以直接编译出可以烧录到 `GX8002` 设备的固件



## 工具命令帮助

* 命令文档

  ```shell
  $ ./kwsModelAutoDeploy -h
  usage: kwsModelAutoDeploy [-h] [-v] [-i INPUTPATH] [-r SDKROOTPATH] [-p PROJECTNAME] [-dv DEPLOYVERSION] [-d]
  
  这是一个可以把 BunKws 训练框架生成的模型，自动部署到 lvp SDK 内的工具
  
  optional arguments:
    -h, --help            show this help message and exit
    -v, -V, --version     Print software version info
    -i INPUTPATH, --inputPath INPUTPATH
                          输入 BunKws 输出的模型文件目录路径
                          注意：
                              1、该目录下应包含如下文件
                              model_79.pt
                              ├── keyword.txt     # 词有序列表 (主要给定顺序)
                              ├── model.h         # 模型文件
                              └── report.xlsx     # 模型测试表 (主要提供阈值)
                                                          
    -r SDKROOTPATH, --sdkRootPath SDKROOTPATH
                          SDK 跟路径，该路径是一切操作的基础路径
                          注意：
                              1、该路径以 lvp_tws 为止
                                  例如:  ~/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws
                                                          
    -p PROJECTNAME, --projectName PROJECTNAME
                          设置项目名称 (后续模型会部署到 SDK 中以该项目名的目录中)
                          注意:
                              1、名称可以由字母和下划线组成，不允许其他字符
                                  例如: xiaoshu_ai
                                                          
    -dv DEPLOYVERSION, --deployVersion DEPLOYVERSION
                          自定义部署的项目模型版本 (主要是给部署到 SDK 的模型定义一个版本)
                          注意:
                              1、可以不设置，默认 v0.1.0，如果当前项目下有版本重复，工具运行过程中会跳出是否覆盖提示
                              2、如果设置的版本在当前项目下有版本重复，工具运行过程中会跳出是否覆盖提示
                                                          
    -d, --debug
                          使能调试模式，默认不打开、主要调整打印等级为 debug, 输出详细打印，用于调试
  
  ```

  

## 使用示例

### 情况一、训练的均是主唤醒词

* 这个情况下，基本是不用在文件中配置事件ID 和是否为主唤醒，直接生成即可

  * 事件ID 默认 100
  * 自动默认为主唤醒

  ```shell
  $ ./kwsModelAutoDeploy -i model_77.pt -r ~/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws -p xiaoshu_aiot -dv v.0.1.0
  WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目已存在 并且设置的 v.0.1.0 版本已经存在
  WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目下，已有版本: ['v.0.1.0']
  是否覆盖已有的 v.0.1.0 版本? y/n: yes
  INFO: [GetWordThreshold]: 提示！ 词: 你好小树  未在 /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/tools/kws_model_auto_deploy/model_77.pt/keyword.txt 中配置 '事件ID'，将默认为 100
  INFO: [GetWordThreshold]: 提示！ 词: 你好小树  未在 /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/tools/kws_model_auto_deploy/model_77.pt/keyword.txt 中配置 '是否主唤醒'，将默认为主唤醒
  INFO: [GetWordThreshold]: 提示！ 词: 打开灯光  未在 /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/tools/kws_model_auto_deploy/model_77.pt/keyword.txt 中配置 '事件ID'，将默认为 100
  INFO: [GetWordThreshold]: 提示！ 词: 打开灯光  未在 /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/tools/kws_model_auto_deploy/model_77.pt/keyword.txt 中配置 '是否主唤醒'，将默认为主唤醒
  INFO: [GetWordThreshold]: 提示！ 词: 关闭灯光  未在 /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/tools/kws_model_auto_deploy/model_77.pt/keyword.txt 中配置 '事件ID'，将默认为 100
  INFO: [GetWordThreshold]: 提示！ 词: 关闭灯光  未在 /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/tools/kws_model_auto_deploy/model_77.pt/keyword.txt 中配置 '是否主唤醒'，将默认为主唤醒
  WARNING: [KwsModelAutoDeploy]: 本次模型部署词信息概览
  WARNING: 顺序: 0	词: 你好小树  阈值: 935	 事件ID: 100 是否为主唤醒: 1
  WARNING: 顺序: 1	词: 打开灯光  阈值: 907	 事件ID: 100 是否为主唤醒: 1
  WARNING: 顺序: 2	词: 关闭灯光  阈值: 903	 事件ID: 100 是否为主唤醒: 1
  INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包!
  INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!
  INFO: [RebulidLvpAutoconf]: 提示! 使用的默认配置为: /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/configs/kws_model_auto_deploy_config/kws_model_auto_deploy.config
  INFO: [Shell]: 提示! 命令: make defconfig
  INFO: [RebulidLvpAutoconf]: 提示! 生成在 LVP SDK 工程下生成: /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/.config 配置成功!!!
  
  
  ```
  
  

### 情况二、训练的有主唤醒和指令词

* 这个情况下、需要进入 KWS 原始模型包中，在 `keyword.txt` 正确配置事件ID 和 是否为主唤醒

  * 工具会读取 keyword.txt 中的词、词顺序、事件ID、是否为主唤醒等等内容，跟随部署到 SDK
  * 缺失、不配置的情况自动使用默认值
    * 事件ID 默认 100
    * 自动默认为主唤醒

* 操作步骤一、

  * 配置 `事件ID` 和`是否为主唤醒`内容

    ```shell
    vim model_77.pt/keyword.txt #进入 BunKws 输出模型包的有序词列表文件
    
    #修改前
    
    <filler> -1
    你好小树 0
    打开灯光 1
    关闭灯光 2
    
    
    # 增加 事件ID 是否主唤醒字段，修改后
    
    <filler> -1
    你好小树 0 事件id: 100  是否主唤醒:1   #1 表示主唤醒，0 表示非主唤醒
    打开灯光 1 事件id: 101  是否主唤醒: 0
    关闭灯光 2 事件id: 102  是否为主唤醒 0
    
    ```

* 操作步骤二、

  * 修改完成后，保存退出

  * 执行工具部署

    ```shell
    $ ./kwsModelAutoDeploy -i model_77.pt -r ~/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws -p xiaoshu_aiot -dv v.0.1.0
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目已存在 并且设置的 v.0.1.0 版本已经存在
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目下，已有版本: ['v.0.1.0']
    是否覆盖已有的 v.0.1.0 版本? y/n: yes
    WARNING: [KwsModelAutoDeploy]: 本次模型部署词信息概览
    WARNING: 顺序: 0	词: 你好小树  阈值: 935	 事件ID: 100 是否为主唤醒: 1
    WARNING: 顺序: 1	词: 打开灯光  阈值: 907	 事件ID: 101 是否为主唤醒: 0
    WARNING: 顺序: 2	词: 关闭灯光  阈值: 903	 事件ID: 102 是否为主唤醒: 0
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包!
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!
    INFO: [RebulidLvpAutoconf]: 提示! 使用的默认配置为: /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/configs/kws_model_auto_deploy_config/kws_model_auto_deploy.config
    INFO: [Shell]: 提示! 命令: make defconfig
    INFO: [RebulidLvpAutoconf]: 提示! 生成在 LVP SDK 工程下生成: /home/liushk/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws/.config 配置成功!!!
    
    ```


### 附加说明：编译固件，烧录芯片

* 经过该工具部署后，用户可以直接回到 SDK 工程，直接编译出固件，并且可以直接把固件烧录芯片（板子）中。

* 编译

  ```shell
  cd ~/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws #回到 SDK 根目录
  make clean
  make #编译固件
  ```

* 烧录

  ```shell
  ~/work/2025-04-25-KwsDeploy/lvp/lvp_tws/tools/bootx  #回到固件烧录脚本目录

  # 烧录脚本    串口号  串口波特率
  ./flash_nor.sh 0 10000000
  ```
  

## 一. 版本记录
---
### v1.0.0

* 1.工具首次提交

---

