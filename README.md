# lvp_kws
* GX8002 是一款专为低功耗领域设计的**超低功耗 AI 神经网络芯片**，适用于低功耗可穿戴设备和语音遥控器等应用。该芯片具有体积小、功耗低、成本低等显著优势。它集成了杭州国芯微自主研发的第二代神经网络处理器 gxNPU V200，支持 **TensorFlow** 和 **Pytorch** 框架，以及自研的硬件 VAD（语音活动检测），显著降低了功耗。在实际测试场景中，VAD 待机功耗可低至 70uW，运行功耗约为 0.6mW，芯片的平均功耗约为 300uW。
* 此仓库适用于开发者部署自己训练模型进行开发。

## 开发板介绍
* 请阅读：[GX8002_DEV开发板介绍](https://nationalchip.gitlab.io/ai_audio_docs/hardware/%E5%BC%80%E5%8F%91%E6%9D%BF%E7%A1%AC%E4%BB%B6%E5%8F%82%E8%80%83%E8%AE%BE%E8%AE%A1/GX8002/GX8002_DEV%E5%BC%80%E5%8F%91%E6%9D%BF/)，在此页面您可以下载开发板硬件规格资料和硬件设计资料。

## 快速入门
* 请阅读：[搭建开发环境](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8/%E6%90%AD%E5%BB%BA%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83/#1-sdk/) 完成编译环境的安装。
* 默认示例编译，请参考下面的[默认示例](#默认示例)
* 请阅读 [串口升级](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8/%E4%B8%B2%E5%8F%A3%E5%8D%87%E7%BA%A7/) 以了解如何将 output/mcu_nor.bin 文件烧录到我们的开发板 (Grus_Dev_V1.4)。
* [串口录音](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8/%E4%B8%B2%E5%8F%A3%E5%BD%95%E9%9F%B3/)
* [应用程序快速开发](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8/%E5%BA%94%E7%94%A8%E7%A8%8B%E5%BA%8F%E5%BF%AB%E9%80%9F%E5%BC%80%E5%8F%91/)
* [内存分布](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8/%E5%86%85%E5%AD%98%E5%88%86%E5%B8%83/)
* [GX8002芯片SDK软件流程图](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E8%BD%AF%E4%BB%B6%E6%9E%B6%E6%9E%84/SDK%E8%BD%AF%E4%BB%B6%E6%B5%81%E7%A8%8B%E5%9B%BE/)
* [SDK开发_FAQ](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BC%80%E5%8F%91_FAQ/%E5%8A%A8%E6%80%81%E8%B0%83%E9%A2%91APP%E4%BB%8B%E7%BB%8D/%E5%8A%A8%E6%80%81%E8%B0%83%E9%A2%91APP%E4%BB%8B%E7%BB%8D%28lvp_app_kws_state_demo%29/)

## NPU使用指南
* [NPU概述](https://nationalchip.gitlab.io/ai_audio_docs/software/npu/NPU%E6%A6%82%E8%BF%B0/)
* [NPU编译器安装](https://nationalchip.gitlab.io/ai_audio_docs/software/npu/NPU%E7%BC%96%E8%AF%91%E5%99%A8%E5%AE%89%E8%A3%85/)
* 支持的算子列表：
    * [支持的 TensorFlow op算子列表](https://nationalchip.gitlab.io/ai_audio_docs/software/npu/NPU%E6%A6%82%E8%BF%B0/#4-gx8002-npu-tensorflow-op)
    * [GX8002 的 NPU 支持的 PyTorch op算子列表](https://nationalchip.gitlab.io/ai_audio_docs/software/npu/NPU%E6%A6%82%E8%BF%B0/#5-gx8002-npu-pytorch-op)
* [NPU编译器使用](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/NPU%E7%BC%96%E8%AF%91%E5%99%A8%E4%BD%BF%E7%94%A8/)
* 模型编译使用示例:
    * [TensorFlow示例1](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/%E6%A8%A1%E5%9E%8B%E7%BC%96%E8%AF%91%E4%BD%BF%E7%94%A8%E7%A4%BA%E4%BE%8B/TensorFlow%E7%A4%BA%E4%BE%8B1/)
    * [TensorFlow示例2](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/%E6%A8%A1%E5%9E%8B%E7%BC%96%E8%AF%91%E4%BD%BF%E7%94%A8%E7%A4%BA%E4%BE%8B/TensorFlow%E7%A4%BA%E4%BE%8B2/)
    * [PyTorch示例](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/%E6%A8%A1%E5%9E%8B%E7%BC%96%E8%AF%91%E4%BD%BF%E7%94%A8%E7%A4%BA%E4%BE%8B/PyTorch%E7%A4%BA%E4%BE%8B/)
* [NPU 模型的 API 部署流程](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/NPU%E6%A8%A1%E5%9E%8B%E7%9A%84API%E9%83%A8%E7%BD%B2%E6%B5%81%E7%A8%8B/)
* [NPU 模型格式说明](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/NPU%E6%A8%A1%E5%9E%8B%E6%A0%BC%E5%BC%8F%E8%AF%B4%E6%98%8E/)
* [NPU 性能介绍](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/NPU%E6%80%A7%E8%83%BD%E4%BB%8B%E7%BB%8D/)
* [NPU 量化精度调试](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/NPU%E9%87%8F%E5%8C%96%E7%B2%BE%E5%BA%A6%E8%B0%83%E8%AF%95/)
* [NPU_FAQ](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/%E9%9F%B3%E9%A2%91%E7%AE%97%E6%B3%95%E5%8F%8ANPU%E5%BC%80%E5%8F%91/NPU%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/NPU_FAQ/)


## 默认示例
### 中文编译：
1. cp ./configs/grus_gx8002b_dev_erji_chinese.config .config
2. make menuconfig
3. 然后保存退出
4. make clean;make
* 中文指令词列表：
```
    1. 小爱同学
    2. 接听电话
    3. 挂掉电话
    4. 挂断电话
    5. 上一首
    6. 下一首
    7. 暂停播放
    8. 停止播放
    9. 播放音乐
    10. 增大音量
    11. 减小音量
```

### 英文编译：
1. cp ./configs/grus_gx8002b_dev_erji_english.config .config
2. make menuconfig
3. 然后保存退出
4. make clean;make
* 英文指令词列表：
    ```
    1. hey siri
    2. ok google
    3. answer the phone
    4. play music
    5. play next song
    6. play previous song
    7. reject the call
    8. stop playing
    9. volume down
    10. volume up
    ```

