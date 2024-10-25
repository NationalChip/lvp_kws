# lvp_kws
中英文耳机

## 环境安装
linux环境编译

sudo apt-get install libncurses5-dev

## 工具链下载和安装
### 工具链下载地址：https://ai.nationalchip.com/api/v1/assets/32/download

### 安装:
1. sudo mkdir -p /opt/csky-abiv2-elf/; cd /opt/csky-abiv2-elf/; 新建并进入路径
2. sudo tar xvf csky-elfabiv2-tools-x86_64-minilibc-*.tar.gz; 解压到当前路径
3. 然后需要编辑 ~/.profile或者 ~/.bashrc，将/opt/csky-abiv2-elf/bin加入到PATH路径中

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

## 固件烧录
1. cd tools/bootx/
2. sudo ./bootx -m grus -c download 0x0 ../../output/mcu_nor.bin -d /dev/ttyUSB0
3. 重启设备,等待烧录完成
4. 打开串口终端,重启设备,查看串口输出
