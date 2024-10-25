本工程目前是搭配lvp工程中app/uart_adpcm_demo使用
参考编译配置：configs/release/nationalchip/grus_gx8002b_dev_1v_uart_adpcm.config


1.目录结构

```
uart_adpcm/
├── adpcm  				# adpcm编解码源文件
│   ├── adpcm.c
│   └── adpcm.h
├── crc 				# crc检验相关源文件
│   ├── crc32.c
│   └── crc.h
├── main.c   			# 主程序入口文件
├── makefile 			# Makefile文件
├── message.c 			# 串口交互协议源文件
├── message.h  			# 串口交互协议头文件
├── porting				# 串口驱动源文件
│   ├── port.c
│   └── port.h
├── README.md 			# 工程说明
├── sample.elf
├── test.pcm 			# adpcm解码出来的pcm数据文件
└── 串口上传adpcm流程图v1.pdf

```


1.编译方式：
* linux平台：
	编译8002串口协议交互： make linux-8002


* windows平台： (需要安装GCC工具，下载地址：https://jmeubank.github.io/tdm-gcc/)
	编译8002串口协议交互： mingw32-make windows-8002


2.使用方法：
* linux平台：
	$ sudo ./sample.elf /dev/ttyUSBx
* windows平台：
	$ sample.exe COMx




详细的串口通信协议V2.0见官方文档：https://nationalchip.gitlab.io/ai_audio_docs/software/vsp/SDK软件架构/串口协议移植指南/串口协议移植指南/



