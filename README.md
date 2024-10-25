# lvp_kws
低功耗中文、英文唤醒SDK，适用于可穿戴产品

## SDK工具链安装
* 请阅读：[SDK工具链安装](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8/%E6%90%AD%E5%BB%BA%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83/#1-sdk)

## 中文编译：
1. cp ./configs/grus_gx8002b_dev_erji_chinese.config .config
2. make menuconfig
3. 然后保存退出
4. make clean;make

中文指令：
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

## 英文编译：
1. cp ./configs/grus_gx8002b_dev_erji_english.config .config
2. make menuconfig
3. 然后保存退出
4. make clean;make

英文指令：
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

## 固件烧录：
* 请阅读：[串口升级](https://nationalchip.gitlab.io/ai_audio_docs/software/lvp/SDK%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97/SDK%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8/%E4%B8%B2%E5%8F%A3%E5%8D%87%E7%BA%A7/)

