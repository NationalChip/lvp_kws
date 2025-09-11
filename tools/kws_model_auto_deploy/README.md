# kwsModelAutoDeploy 工具
---
## 1. 简介：
* 这是一个可以把  KWS 训练框架生成的模型，自动部署到 `lvp` SDK 内的工具，使用该工具后，用户可以直接编译出可以烧录到 `GX8002` 设备的固件



## 2. 工具命令帮助

### 2.1. 命令文档（部署模式选择）
```shell
usage: kwsModelAutoDeploy [-h] [-v] {BunKwsDeploymentMode,bunkwsdeploymentmode,AlchemyDeploymentMode,alchemydeploymentmode} ...

这是一个可以把 Kws 模型训练框架生成的模型，自动部署到 lvp SDK 内的工具

optional arguments:
  -h, --help            show this help message and exit
  -v, -V, --version     Print software version info

subcommands:
  {BunKwsDeploymentMode,bunkwsdeploymentmode,AlchemyDeploymentMode,alchemydeploymentmode}
    BunKwsDeploymentMode (bunkwsdeploymentmode)
                        针对 BunKws 训练平台输出的模型，自动部署到 SDK 模式
    AlchemyDeploymentMode (alchemydeploymentmode)
                        针对 Alchemy 训练平台输出的模型，自动部署到 SDK 模式
```

### 2.2. 命令文档（BunKws 部署模式）针对纯 bunkws 训练平台输出的模型

```shell
$ ./kwsModelAutoDeploy BunKwsDeploymentMode -h
usage: kwsModelAutoDeploy BunKwsDeploymentMode [-h] [-i INPUTPATH] [-r SDKROOTPATH] [-p PROJECTNAME] [-dv DEPLOYVERSION] [-d]

optional arguments:
  -h, --help            show this help message and exit
  -i INPUTPATH, --inputPath INPUTPATH
                        输入 BunKws 模型训练框架输出的模型文件目录路径
                        注意：
                            1、该目录下应包含如下文件
                            model_79.pt
                            ├── keyword.txt     # 词有序列表 (主要给定顺序)
                            ├── model_grus.h    # 模型文件
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



### 2.3. 命令文档（Alchemy 部署模式）针对纯 Alchemy 训练平台输出的模型

```shell
$ ./kwsModelAutoDeploy AlchemyDeploymentMode -h
usage: kwsModelAutoDeploy AlchemyDeploymentMode [-h] [-i INPUTPATH] [-r SDKROOTPATH] [-p PROJECTNAME] [-dv DEPLOYVERSION] [-d]

optional arguments:
  -h, --help            show this help message and exit
  -i INPUTPATH, --inputPath INPUTPATH
                        输入 Alchemy 模型训练框架输出的模型文件目录路径
                        注意：
                            1、该目录下应包含如下文件
                            alchemy_平台输出_最新
                            ├── cmd_id.txt      # 指令信息文件 (包含指令、指令lable、阈值、是否主唤醒)
                            └── model_grus.h    # 模型文件
                                                        
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



## 3. BunKws 部署模式 使用示例

### 3.1. 情况一、训练的均是主唤醒词

* 这个情况下，基本是不用在<font color=red> keyword.txt  文件</font>中配置事件ID 和是否为主唤醒，直接生成即可

  * 事件ID 默认 100
  * 自动默认为主唤醒

  ```shell
  $ ./kwsModelAutoDeploy BunKwsDeploymentMode -i model_79.pt -r ~/work/2025-08-04-root/lvp/lvp_tws -p xiaoshu_aiot -dv v0.1.0
  Namespace(debug=False, deployVersion='v0.1.0', func=<function func_BunKwsDeploymentMode at 0x7fd128ea09d0>, inputPath='model_79.pt', projectName='xiaoshu_aiot', sdkRootPath='/home/liushk/work/2025-08-04-root/lvp/lvp_tws', subcommand='bunkwsdeploymentmode')
  INFO: [KwsModelAutoDeploy]: 提示！开始部署 bunkws 模型至 LVP SDK
  WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目已存在 并且设置的 v0.1.0 版本已经存在
  WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目下，已有版本: ['v0.1.0']
  是否覆盖已有的 v0.1.0 版本? y/n: y
  WARNING: [KwsModelAutoDeploy]: 本次模型部署词信息概览
  WARNING: {0: {'CTC模型阈值': 700,
       'label': [7, 21, 11, 37, 14, 38, 60, 22],
       'label_length': 8,
       '事件ID': 100,
       '是否主唤醒': '1',
       '词': '你好小树',
       '词拼音': 'ni hao xiao shu',
       '阈值': 935},
   1: {'CTC模型阈值': 700,
       'label': [5, 24, 10, 31, 5, 50, 9, 49],
       'label_length': 8,
       '事件ID': 100,
       '是否主唤醒': '1',
       '词': '打开灯光',
       '词拼音': 'da kai deng guang',
       '阈值': 907},
   2: {'CTC模型阈值': 700,
       'label': [9, 43, 1, 21, 5, 50, 9, 49],
       'label_length': 8,
       '事件ID': 100,
       '是否主唤醒': '1',
       '词': '关闭灯光',
       '词拼音': 'guan bi deng guang',
       '阈值': 903}}
  INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP BunKws 声学模型包!
  INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!
  INFO: [RebulidLvpAutoconf]: 提示! 使用的默认配置为: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/configs/kws_model_auto_deploy_config/bunkws_model_auto_deploy.config
  INFO: [Shell]: 提示! 命令: make defconfig
  INFO: [RebulidLvpAutoconf]: 提示! 生成在 LVP SDK 工程下生成: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/.config 配置成功!!!
  ```

### 3.2. 情况二、训练的有主唤醒和指令词

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
    $ ./kwsModelAutoDeploy BunKwsDeploymentMode -i model_79.pt -r ~/work/2025-08-04-root/lvp/lvp_tws -p xiaoshu_aiot -dv v0.1.0
    Namespace(debug=False, deployVersion='v0.1.0', func=<function func_BunKwsDeploymentMode at 0x7f23b11be9d0>, inputPath='model_79.pt', projectName='xiaoshu_aiot', sdkRootPath='/home/liushk/work/2025-08-04-root/lvp/lvp_tws', subcommand='bunkwsdeploymentmode')
    INFO: [KwsModelAutoDeploy]: 提示！开始部署 bunkws 模型至 LVP SDK
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目已存在 并且设置的 v0.1.0 版本已经存在
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目下，已有版本: ['v0.1.0']
    是否覆盖已有的 v0.1.0 版本? y/n: y
    WARNING: [KwsModelAutoDeploy]: 本次模型部署词信息概览
    WARNING: {0: {'CTC模型阈值': 700,
         'label': [7, 21, 11, 37, 14, 38, 60, 22],
         'label_length': 8,
         '事件ID': 100,
         '是否主唤醒': '1',
         '词': '你好小树',
         '词拼音': 'ni hao xiao shu',
         '阈值': 935},
     1: {'CTC模型阈值': 700,
         'label': [5, 24, 10, 31, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 101,
         '是否主唤醒': '0',
         '词': '打开灯光',
         '词拼音': 'da kai deng guang',
         '阈值': 907},
     2: {'CTC模型阈值': 700,
         'label': [9, 43, 1, 21, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 102,
         '是否主唤醒': '0',
         '词': '关闭灯光',
         '词拼音': 'guan bi deng guang',
         '阈值': 903}}
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP BunKws 声学模型包!
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!
    INFO: [RebulidLvpAutoconf]: 提示! 使用的默认配置为: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/configs/kws_model_auto_deploy_config/bunkws_model_auto_deploy.config
    INFO: [Shell]: 提示! 命令: make defconfig
    INFO: [RebulidLvpAutoconf]: 提示! 生成在 LVP SDK 工程下生成: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/.config 配置成功!!!
    ```



## 4. Alchemy 部署模式 使用示例

### 4.1. 情况一、没有自定义事件ID的需求

* 这个情况下，基本是不用在<font color=red> cmd_id.txt  文件</font>中配置事件ID，直接生成即可

  * 主唤醒事件ID 默认 100

  * 非主唤醒事件ID 在 100 基础上自动递增（详情可以看工具打印输出的<font color=red>本次模型部署词信息概览</font>） 

    ```shell
    $ ./kwsModelAutoDeploy AlchemyDeploymentMode -r ~/work/2025-08-04-root/lvp/lvp_tws -p xiaoshu_aiot -i alchemy_平台输出_最新 -dv v0.0.1   
    Namespace(debug=False, deployVersion='v0.0.1', func=<function func_AlchemyDeploymentMode at 0x7f6ac3210e50>, inputPath='alchemy_平台输出_最新', projectName='xiaoshu_aiot', sdkRootPath='/home/liushk/work/2025-08-04-root/lvp/lvp_tws', subcommand='alchemydeploymentmode')
    INFO: [KwsModelAutoDeploy]: 提示！开始部署 alchemy 模型至 LVP SDK
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目已存在 并且设置的 v0.0.1 版本已经存在
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目下，已有版本: ['v0.0.1', 'v0.1.0']
    是否覆盖已有的 v0.0.1 版本? y/n: Y
    INFO: [GetWordsConfig]: 提示! 开始词分组
    INFO: [SetGroups]: 提示! 开始自动分组
    INFO: [SetGroups]: 提示! 总个数: 6, 分组数: 1，每组个数: [6], 打分窗长: 37
    WARNING: [KwsModelAutoDeploy]: 本次模型部署词信息概览
    WARNING: {0: {'CTC模型阈值': 700,
         'label': [7, 21, 11, 37, 14, 38, 5, 50],
         'label_length': 8,
         '事件ID': 100,
         '是否主唤醒': 1,
         '词': '你好小灯',
         '词拼音': 'ni hao xiao deng',
         '阈值': ' 998'},
     1: {'CTC模型阈值': 700,
         'label': [7, 21, 11, 37, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 100,
         '是否主唤醒': 1,
         '词': '你好灯光',
         '词拼音': 'ni hao deng guang',
         '阈值': ' 998'},
     2: {'CTC模型阈值': 700,
         'label': [9, 43, 1, 21, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 101,
         '是否主唤醒': 0,
         '词': '关闭灯光',
         '词拼音': 'guan bi deng guang',
         '阈值': ' 998'},
     3: {'CTC模型阈值': 700,
         'label': [14, 38, 60, 22, 7, 21, 11, 37],
         'label_length': 8,
         '事件ID': 102,
         '是否主唤醒': 0,
         '词': '小树你好',
         '词拼音': 'xiao shu ni hao',
         '阈值': ' 998'},
     4: {'CTC模型阈值': 700,
         'label': [5, 24, 10, 31, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 103,
         '是否主唤醒': 0,
         '词': '打开灯光',
         '词拼音': 'da kai deng guang',
         '阈值': ' 998'},
     5: {'CTC模型阈值': 700,
         'label': [13, 40, 7, 21, 10, 31, 5, 50],
         'label_length': 8,
         '事件ID': 104,
         '是否主唤醒': 0,
         '词': '请你开灯',
         '词拼音': 'qing ni kai deng',
         '阈值': ' 998'}}
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP  Alchemy 声学模型包!
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!
    INFO: [RebulidLvpAutoconf]: 提示! 使用的默认配置为: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/configs/kws_model_auto_deploy_config/alchemy_model_auto_deploy.config
    INFO: [Shell]: 提示! 命令: make defconfig
    INFO: [RebulidLvpAutoconf]: 提示! 生成在 LVP SDK 工程下生成: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/.config 配置成功!!!
    ```
### 4.2. 情况二、有自定义事件ID的需求

* 这个情况下，需要用户手动打开 Alchemy 训练输出的模型包中 <font color=red> cmd_id.txt  文件</font>中配置事件ID，如果不配置，部署工具自动分配事件ID

* 操作步骤一、

  * 配置事件ID

    * <font color=red>注意：通常训练框架输出的时候已经给主唤醒事件ID 配置了 100，正常不需要动，关键自定义的是短指令</font>
    * <font color=red>注意：只需要指令词的最后一个数字，其他字段请不要随意修改</font>

    ```shell
    vim alchemy_平台输出_最新/cmd_id.txt #进入指令信息文件 (内部包含指令、指令lable、阈值、是否主唤醒)
    
    # 修改前
    md5:b4f8a3
    input_stride:4
    你好小灯, [7, 21, 11, 37, 14, 38, 5, 50], 998, 100
    小树你好, [14, 38, 60, 22, 7, 21, 11, 37], 998, 100
    关闭灯光, [9, 43, 1, 21, 5, 50, 9, 49], 998, 000
    打开灯光, [5, 24, 10, 31, 5, 50, 9, 49], 998, 000
    请你开灯, [13, 40, 7, 21, 10, 31, 5, 50], 998, 000
    
    #修改后
    md5:b4f8a3
    input_stride:4
    你好小灯, [7, 21, 11, 37, 14, 38, 5, 50], 998, 100
    小树你好, [14, 38, 60, 22, 7, 21, 11, 37], 998, 100
    关闭灯光, [9, 43, 1, 21, 5, 50, 9, 49], 998, 101     #配置短指令 "关闭灯光" 的事件ID 为 101
    打开灯光, [5, 24, 10, 31, 5, 50, 9, 49], 998, 102    #配置短指令 "打开灯光" 的事件ID 为 102
    请你开灯, [13, 40, 7, 21, 10, 31, 5, 50], 998, 103   #配置短指令 "请你开灯" 的事件ID 为 103
    ```

* 操作步骤二、

  * 修改完成后，保存退出

  * 执行工具部署
  
    ```shell
    $ ./kwsModelAutoDeploy AlchemyDeploymentMode -r ~/work/2025-08-04-root/lvp/lvp_tws -p xiaoshu_aiot -i alchemy_平台输出_最新 -dv v0.0.1
    Namespace(debug=False, deployVersion='v0.0.1', func=<function func_AlchemyDeploymentMode at 0x7f769c45be50>, inputPath='alchemy_平台输出_最新', projectName='xiaoshu_aiot', sdkRootPath='/home/liushk/work/2025-08-04-root/lvp/lvp_tws', subcommand='alchemydeploymentmode')
    INFO: [KwsModelAutoDeploy]: 提示！开始部署 alchemy 模型至 LVP SDK
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目已存在 并且设置的 v0.0.1 版本已经存在
    WARNING: [KwsModelAutoDeploy]: 注意！xiaoshu_aiot 项目下，已有版本: ['v0.0.1', 'v0.1.0']
    是否覆盖已有的 v0.0.1 版本? y/n: y
    INFO: [GetWordsConfig]: 提示! 开始词分组
    INFO: [SetGroups]: 提示! 开始自动分组
    INFO: [SetGroups]: 提示! 总个数: 5, 分组数: 1，每组个数: [5], 打分窗长: 37
    WARNING: [KwsModelAutoDeploy]: 本次模型部署词信息概览
    WARNING: {0: {'CTC模型阈值': 700,
         'label': [7, 21, 11, 37, 14, 38, 5, 50],
         'label_length': 8,
         '事件ID': 100,
         '是否主唤醒': 1,
         '词': '你好小灯',
         '词拼音': 'ni hao xiao deng',
         '阈值': ' 998'},
     1: {'CTC模型阈值': 700,
         'label': [7, 21, 11, 37, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 100,
         '是否主唤醒': 1,
         '词': '你好灯光',
         '词拼音': 'ni hao deng guang',
         '阈值': ' 998'},
     2: {'CTC模型阈值': 700,
         'label': [9, 43, 1, 21, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 101,
         '是否主唤醒': 0,
         '词': '关闭灯光',
         '词拼音': 'guan bi deng guang',
         '阈值': ' 998'},
     3: {'CTC模型阈值': 700,
         'label': [5, 24, 10, 31, 5, 50, 9, 49],
         'label_length': 8,
         '事件ID': 102,
         '是否主唤醒': 0,
         '词': '打开灯光',
         '词拼音': 'da kai deng guang',
         '阈值': ' 998'},
     4: {'CTC模型阈值': 700,
         'label': [13, 40, 7, 21, 10, 31, 5, 50],
         'label_length': 8,
         '事件ID': 103,
         '是否主唤醒': 0,
         '词': '请你开灯',
         '词拼音': 'qing ni kai deng',
         '阈值': ' 998'}}
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP  Alchemy 声学模型包!
    INFO: [CreateLvpAcousticModelPackage]: 提示! 生成 LVP 声学模型包完成!
    INFO: [RebulidLvpAutoconf]: 提示! 使用的默认配置为: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/configs/kws_model_auto_deploy_config/alchemy_model_auto_deploy.config
    INFO: [Shell]: 提示! 命令: make defconfig
    INFO: [RebulidLvpAutoconf]: 提示! 生成在 LVP SDK 工程下生成: /home/liushk/work/2025-08-04-root/lvp/lvp_tws/.config 配置成功!!!
    ```





## 5. 附加说明：编译固件，烧录芯片

* 经过该工具部署后，用户可以直接回到 SDK 工程，直接编译出固件，并且可以直接把固件烧录芯片（板子）中。

### 5.1.  编译

```shell
cd ~/work/2025-04-21-Kws2SdkDeploy/lvp/lvp_tws #回到 SDK 根目录
make clean
make #编译固件
```

### 5.2. 烧录
```shell
~/work/2025-04-25-KwsDeploy/lvp/lvp_tws/tools/bootx  #回到固件烧录脚本目录

# 烧录脚本    串口号  串口波特率
./flash_nor.sh 0 10000000
```


## 6. 版本记录
---
### v1.0.0

* 1.工具首次提交

---
### v1.0.1

* 1.解决环境依赖报错BUG

---

---
### v2.0.0

* 1.重构命令行参数
* 2.添加支持 alchemy 训练框架输出的模型自动部署至 SDK

---

### v2.0.1

* 1.解决 bunkws 部署英文报错问题

---

