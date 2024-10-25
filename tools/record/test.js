let message = require('./message_2.0.js');
let SerialPort = require("serialport");
let fs = require("fs"); //文件模块

// let messageRegistRecv = require('./message_2.0.js');
// let messageLogoutRecv = require('./message_2.0.js');
// let messageSendPkg = require('./message_2.0.js');
// let pushData = require('./message_2.0.js');


console.log("Nationalchip Grus Record");

let intervalHandle = undefined;
let intervalCount = 0;

function messageSendPkgAndSetInterval(pkg, send, interval, event, once) {
    message.messageSendPkg(pkg, send);
    if (once) return;
    intervalHandle = setInterval(
        (a, b) => {
            console.log(intervalCount++);

            message.messageSendPkg(a, b);
        },
        interval,
        pkg,
        send
    );
}

function Pkg(magic, cmd, seq, flags, body) {
    return { msg_header: { magic, cmd, seq, flags }, body };
}

function sendStartPcmCmd(channel) {
    if (channel > 255) {
        return -1;
    }
    let pkgBuffer = Buffer.alloc(16);
    pkgBuffer[0] = 0;
    pkgBuffer[1] = 0;
    pkgBuffer[2] = 0;
    pkgBuffer[3] = 0;
    pkgBuffer[4] = ((channel > 1) && (channel % 2 != 0) ? 2 : 1);
    pkgBuffer[5] = 0;
    pkgBuffer[6] = 0;
    pkgBuffer[7] = 0;
    pkgBuffer[8] = channel;
    pkgBuffer[9] = 0;
    pkgBuffer[10] = 0;
    pkgBuffer[11] = 0;
    pkgBuffer[12] = recordHandle.flow_ctrl_enable;
    pkgBuffer[13] = 0;
    pkgBuffer[14] = 0;
    pkgBuffer[15] = 0;

    let pkg = Pkg(0, 0x307, 0, 0, pkgBuffer);
    message.messageSendPkg(pkg,
        buffer =>
            uartPort.write(buffer, err => {
                if (err) console.error(err);
            })
    );
    // messageSendPkgAndSetInterval(
    //     pkg,
    //     buffer =>
    //         uartPort.write(buffer, err => {
    //             if (err) console.error(err);
    //             console.log(pkg);
    //         }),
    //     1000,
    //     NaN
    // );


}

function sendReadPcmCmd() {

    pkgOperation = Pkg(0, 0x384, 0, 0, undefined);
    message.messageSendPkg(pkgOperation,
        buffer =>
            uartPort.write(buffer, err => {
                if (err) console.error(err);
            })
    );
}

function sendDonePcmCmd() {
    let pkg_done = Pkg(0, 0x308, 0, 0, undefined);

    message.messageSendPkg(pkg_done,
        buffer =>
            uartPort.write(buffer, err => {
                if (err) console.error(err);
            })
    );

}

var addWavHeader = function (samples, sampleRateTmp, sampleBits, channelCount) {
    var dataLength = samples.byteLength;
    var buffer = new ArrayBuffer(44 + dataLength);
    var view = new DataView(buffer);
    function writeString(view, offset, string) {
        for (var i = 0; i < string.length; i++) {
            view.setUint8(offset + i, string.charCodeAt(i));
        }
    }

    var offset = 0;
    /* 资源交换文件标识符 */
    writeString(view, offset, 'RIFF'); offset += 4;
    /* 下个地址开始到文件尾总字节数,即文件大小-8 */
    view.setUint32(offset, /*32*/ 36 + dataLength, true); offset += 4;
    /* WAV文件标志 */
    writeString(view, offset, 'WAVE'); offset += 4;
    /* 波形格式标志 */
    writeString(view, offset, 'fmt '); offset += 4;
    /* 过滤字节,一般为 0x10 = 16 */
    view.setUint32(offset, 16, true); offset += 4;
    /* 格式类别 (PCM形式采样数据) */
    view.setUint16(offset, 1, true); offset += 2;
    /* 通道数 */
    view.setUint16(offset, channelCount, true); offset += 2;
    /* 采样率,每秒样本数,表示每个通道的播放速度 */
    view.setUint32(offset, sampleRateTmp, true); offset += 4;
    /* 波形数据传输率 (每秒平均字节数) 通道数×每秒数据位数×每样本数据位/8 */
    view.setUint32(offset, sampleRateTmp * channelCount * (sampleBits / 8), true); offset += 4;
    /* 快数据调整数 采样一次占用字节数 通道数×每样本的数据位数/8 */
    view.setUint16(offset, channelCount * (sampleBits / 8), true); offset += 2;
    /* 每样本数据位数 */
    view.setUint16(offset, sampleBits, true); offset += 2;
    /* 数据标识符 */
    writeString(view, offset, 'data'); offset += 4;
    /* 采样数据总数,即数据总大小-44 */
    view.setUint32(offset, dataLength, true); offset += 4;

    function To8BitPCM(output, offset, input) {
        input = new Int8Array(input);
        for (var i = 0; i < input.length; i++ , offset++) {
            output.setInt8(offset, input[i], true);
        }
    }

    To8BitPCM(view, 44, samples);

    return view.buffer;
}

function pcmToWav() {
    if (recordHandle.wavPath !== "") {
        if (fs.existsSync(recordHandle.outPath) === true) {
            var pcmData = fs.readFileSync(recordHandle.outPath);
            var wavData = addWavHeader(pcmData, 16000, 16, 1);
            var int8wavData = new Int8Array(wavData)
            fs.writeFileSync(recordHandle.wavPath, int8wavData);
        }
    }
}

let count = 0;
let recv_done = false;
function messageRecvCallback(pkg, priv) {
    if (intervalHandle !== undefined) {
        clearInterval(intervalHandle);
        intervalCount = 0;
        intervalHandle = undefined;
    }
    if (pkg.msg_header.cmd === 0x381) { // start
        if (pkg.body[8] == recordHandle.channel) {
            // console.log(pkg.body[8]);
            sendReadPcmCmd();
        } else {
            // console.log(pkg.body[8]);
        }

    }

    if (pkg.msg_header.cmd === 0x382) { // data
        if (recv_done) {
            return;
        }

        // console.log(pkg.body.length);

        recordHandle.savedLen += pkg.body.length / 2 / 16000 / (recordHandle.channel === 0x3 ? 2 : 1); // 双通道时统计长度除以2
        console.log("%", (recordHandle.savedLen / recordHandle.recordLen * 100 < 100 ? recordHandle.savedLen / recordHandle.recordLen * 100 : 100).toFixed(1),
            "|",
            (recordHandle.recordLen - recordHandle.savedLen > 0 ? recordHandle.recordLen - recordHandle.savedLen : 0).toFixed(3), "s",
            "|",
            recordHandle.outPath
        );

        // console.log(count++);

        if (recordHandle.savedLen >= recordHandle.recordLen
        ) {
            recv_done = true;

            sendDonePcmCmd();
            intervalHandle = setInterval(
                () => {
                    pcmToWav();
                    console.log("Done, seaved", recordHandle.savedLen.toFixed(3), "s");
                    uartPort.close(() => { });
                    process.exit()
                },
            );

        } else if (recordHandle.flow_ctrl_enable === 1) {
            sendReadPcmCmd();
        }


        if (recordHandle.multi_channel === 0) {
            if (((recordHandle.channel > 1) && (recordHandle.channel % 2 != 0) ? 2 : 1) == 1)
                fs.appendFileSync(recordHandle.outPath, pkg.body);
            else {
                if (pkg.msg_header.seq % 2)
                    fs.appendFileSync(recordHandle.outPath + '_a', pkg.body);
                else
                    fs.appendFileSync(recordHandle.outPath + '_b', pkg.body);
            }
        } else {
            switch (pkg.msg_header.seq % recordHandle.multi_channel) {
                case 0:
                    fs.appendFileSync(recordHandle.outPath + '_0', pkg.body);
                    break;
                case 1:
                    fs.appendFileSync(recordHandle.outPath + '_1', pkg.body);
                    break;
                case 2:
                    fs.appendFileSync(recordHandle.outPath + '_2', pkg.body);
                    break;
                case 3:
                    fs.appendFileSync(recordHandle.outPath + '_3', pkg.body);
                    break;
            }
        }

    }
    if (false) {
        uartPort.close(() => { });
    }
}

let sig_over = ['SIGINT', 'SIGTERM', 'SIGHUP']

for (let i in sig_over) {
    process.on(sig_over[i], function () {
        sendDonePcmCmd();
        pcmToWav();
        console.log(" " + sig_over[i] + ' Exit now!');
        uartPort.close(() => {
            process.exit();
        });
    });
}

function consoleHelpInfo() {
    console.log(`
help info:
Version : v1.0.0 (20200529)
NationalChip GX8002 Record Tools
Copyright (C) 2001-2020 NationalChip Co., Ltd
ALL RIGHTS RESERVED!

Usage: grus-record-tool [-p <uart port path>] [-b uart baudrate] [-c <record channel>] [-o <outfile path>] [-d <record time len>] [-w <wav path>] [-h]

    -p : e.g. /dev/ttyUSB0 (default:/dev/ttyUSB0)
    -b : 460800/500000/576000/1000000  (just support 500000 for now)
    -c : mic0/mic1/fft/logfbank/G-sensor
    -o : e.g. ./record.pcm (just output pcm data for now, sample rate 16000, 16 bit)
    -w : e.g. ./record.wav (pcm to wav, just support mic0 or mic1)
    -d : e.g. 10 (default: 10s)
    -h : show help info

    ex. grus-record-tool -p /dev/ttyUSB0 -o record.pcm

`
    );
    process.exit()

}

let recordHandle = {
    channel: 1,
    uartPort: "/dev/ttyUSB0",
    uartBaudRate: 500000,
    outPath: "./record.pcm",
    wavPath: "",
    recordLen: 10,
    savedLen: 0,
    flow_ctrl_enable: 0,
    multi_channel: 0
}

let argvs = process.argv;

// console.log(argvs);

let argv_index = 0;

if (argvs.length < 3)
    consoleHelpInfo();

argv_index = argvs.indexOf("-h");
if (argv_index !== -1)
    consoleHelpInfo();

argv_index = argvs.indexOf("-p");
if (argv_index !== -1)
    recordHandle.uartPort = argvs[argv_index + 1];

argv_index = argvs.indexOf("-w");
if (argv_index !== -1)
    recordHandle.wavPath = argvs[argv_index + 1];

argv_index = argvs.indexOf("-b");
if (argv_index !== -1)
    recordHandle.uartBaudRate = parseInt(argvs[argv_index + 1]);

argv_index = argvs.indexOf("-c");
if (argv_index !== -1) {
    switch (argvs[argv_index + 1]) {
        case "mic0": {
            recordHandle.channel = 0x1;
            break;
        }
        case "mic1": {
            recordHandle.channel = 0x2;
            break;
        }
        case "fft": {
            recordHandle.channel = 0x4;
            break;
        }
        case "logfbank": {
            recordHandle.channel = 0x8;
            break;
        }
        case "G-sensor": {
            recordHandle.channel = 0x10;
            break;
        }
        case "mic0+mic1": {
            recordHandle.channel = 0x3;
            break;
        }
        case "1": case "2": case "3": case "4": case "5": case "6": {
            recordHandle.multi_channel = Number(argvs[argv_index + 1]);
            break;
        }
        default: {
            console.log("Channel set error!");
            consoleHelpInfo();
            break;
        }
    }
}

argv_index = argvs.indexOf("-o");
if (argv_index !== -1)
    recordHandle.outPath = argvs[argv_index + 1];

argv_index = argvs.indexOf("-d");
if (argv_index !== -1)
    recordHandle.recordLen = argvs[argv_index + 1];

argv_index = argvs.indexOf("-f");
if (argv_index !== -1)
    recordHandle.flow_ctrl_enable = 1;


fs.exists(recordHandle.uartPort, function (exists) {
    if (exists) {
        console.log(recordHandle.uartPort);
    } else {
        console.log("Can not find uart port:", recordHandle.uartport)
        consoleHelpInfo();
    };
});

let Path_Suffix = ["", "_a", "_b", "_0", "_1", "_2", "_3", "_4", "_5"];

for (let i in Path_Suffix) {
    let path = recordHandle.outPath + Path_Suffix[i];
    fs.exists(path, function (exists) {
        if (exists) {
            fs.unlink(path, function (error) {
                if (error) {
                    console.log(error);
                    return false;
                }
            });
        };
    });
}


// if (recordHandle.uartBaudRate < 460800) {
//     console.log("Uart baudRate too low");
//     consoleHelpInfo();
// }



message.messageRegistRecv(0x381, messageRecvCallback, undefined); // start
message.messageRegistRecv(0x382, messageRecvCallback, undefined); // data


let uartPort = new SerialPort(recordHandle.uartPort, {
    baudRate: recordHandle.uartBaudRate,
    autoOpen: true,
    dataBits: 8, //数据位
    parity: "none", //奇偶校验
    stopBits: 1, //停止位
    flowControl: false
}).on("data", function (buff) {
    // console.log(buff);
    message.pushData(buff);
});

sendStartPcmCmd(recordHandle.channel);
