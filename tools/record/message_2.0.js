let path = require("path"); //系统路径模块
let fs = require("fs"); //文件模块
let SerialPort = require("serialport");
let crc32 = require("crc-32");

class queueByArray {
  constructor() {
    this._buff = [];
    this.put = a => {
      this._buff.push(deepClone(a));
    };
    this.get = () => {
      let tmp = this._buff[0];
      this._buff.shift();
      return tmp;
    };
    this.isEmpty = () => {
      return this._buff.length === 0;
    };
  }
}

class messageHeader {
  constructor() {
    this.magic = 0;
    this.cmd = 0;
    this.seq = 0;
    this.flags = 0;
    this.length = 0;
    this.crc32 = 0;
  }
}

class messagePackge {
  constructor() {
    this.msg_header = new messageHeader();
    this.body = undefined;
  }
}

class messageRecvRegistInfo {
  constructor() {
    this.cmd = 0;
    this.callback = undefined;
    this.priv = undefined;
  }
}

let s_uartPortRecvArray = [];
let s_recvBuffer = Buffer.alloc(1023);
let s_Magic = 0x42555858;
let s_tmpMagic = 0;
let s_headerSize = 14;
let s_recvHeader = new messageHeader();
let s_recvRegistArray = [];
let s_sendSeq = 0;

function messageSendPkg(pkg, send) {
  pkg.msg_header["length"] =
    (Buffer.isBuffer(pkg.body) ? pkg.body.length : 0) +
    (pkg.msg_header.flags === 1 ? 4 : 0);
  let sendBuffer = Buffer.alloc(s_headerSize + pkg.msg_header.length);
  sendBuffer.fill();

  let magic = pkg.msg_header.magic === 0 ? s_Magic : pkg.msg_header.magic;

  sendBuffer[0] = magic & 0xff;
  sendBuffer[1] = (magic & 0xff00) >> 8;
  sendBuffer[2] = (magic & 0xff0000) >> 16;
  sendBuffer[3] = (magic & 0xff000000) >> 24;

  sendBuffer[4] = pkg.msg_header.cmd & 0xff;
  sendBuffer[5] = (pkg.msg_header.cmd & 0xff00) >> 8;

  if ((pkg.msg_header.cmd & 0xff00) !== 0x2) {
    sendBuffer[6] = s_sendSeq++;
  }

  sendBuffer[7] = pkg.msg_header.flags;

  sendBuffer[8] = pkg.msg_header.length & 0xff;
  sendBuffer[9] = (pkg.msg_header.length & 0xff00) >> 8;

  let tmpBuffer = Buffer.alloc(10);
  sendBuffer.copy(tmpBuffer, 0, 0, 10);

  let crc = crc32.buf(tmpBuffer);
  sendBuffer[10] = crc & 0xff;
  sendBuffer[11] = (crc & 0xff00) >> 8;
  sendBuffer[12] = (crc & 0xff0000) >> 16;
  sendBuffer[13] = (crc & 0xff000000) >> 24;

  if (Buffer.isBuffer(pkg.body)) {
    pkg.body.copy(sendBuffer, 14);
  }

  if (Buffer.isBuffer(pkg.body) && pkg.msg_header.flags === 1) {
    let bodyCrc = crc32.buf(pkg.body);
    sendBuffer[sendBuffer.length - 1] = (bodyCrc & 0xff000000) >> 24;
    sendBuffer[sendBuffer.length - 2] = (bodyCrc & 0xff0000) >> 16;
    sendBuffer[sendBuffer.length - 3] = (bodyCrc & 0xff00) >> 8;
    sendBuffer[sendBuffer.length - 4] = bodyCrc & 0xff;
  }

  // console.log("sendBuffer.length ", sendBuffer.length);
  // console.log(sendBuffer);
  send(sendBuffer);
}

function messageRegistRecv(cmd, callback, priv) {
  if (typeof cmd !== "number") {
    return -1;
  }
  for (let i = 0; i < s_recvRegistArray.length; i++) {
    if (cmd === s_recvRegistArray[i].cmd) {
      s_recvRegistArray[i].callback = callback;
      s_recvRegistArray[i].priv = priv;

      return 0;
    }
  }

  let info = new messageRecvRegistInfo();

  info.cmd = cmd;

  info.callback = callback;
  info.priv = priv;

  s_recvRegistArray.push(deepClone(info));
}

function messageLogoutRecv(cmd, callback, priv) {
  if (typeof cmd !== "number") {
    return -1;
  }
  for (let i = 0; i < s_recvRegistArray.length; i++) {
    if (cmd === s_recvRegistArray[i].cmd) {
      s_recvRegistArray.splice(i, 1);
      return 0;
    }
  }
  return;
}

// function _getRegistedBufferLength(cmd) {
//     for (let i of s_recvRegistArray) {
//         if (i.cmd === cmd) {
//             return Buffer.isBuffer(i.body) ? i.body.length : 0;
//         }
//     }

//     return 0;
// }
// function _getRegistedInfo(cmd) {
//     for (let i of s_recvRegistArray) {
//         if (i.cmd === cmd) {
//             return i;
//         }
//     }

//     return undefined;
// }

// setTimeout(function () {

//     console.log(s_uartPortRecvArray.length);
// }, 1);

// let t1 = setInterval(function () {

//     console.log(s_uartPortRecvArray.length);
//     if (s_uartPortRecvArray.length) {
//         clearInterval(t1)
//         wmz = 233;
//     }
// }, 100);

function myCallback(pack, priv) {
  console.log(pack);
  if (pack.msg_header.length > 0) {
    // console.log(pack.body.toString());
  }

  console.log(priv);
  // console.log(s_recvBuffer.toString());
}

function _callRegistCallback(pkg) {
  // console.log(s_recvRegistArray);
  for (let i = 0; i < s_recvRegistArray.length; i++) {
    if (pkg.msg_header.cmd === s_recvRegistArray[i].cmd) {
      s_recvRegistArray[i].callback(pkg, s_recvRegistArray[i].priv);
      return 0;
    }
  }

  return -1;
}

let recvState = "magic";

function cheackMagic(ch) {
  let m = ch & 0xff;

  s_tmpMagic = ((s_tmpMagic >> 8) & 0xffffff) | (m << 24);

  if (s_tmpMagic === s_Magic) {
    s_recvHeader.magic = s_Magic;
    recvState = "header";
    return true;
  }
}

function cheackHeader(buff) {
  // console.log("cheackHeader");
  s_recvHeader.cmd = (s_recvHeader.cmd & 0) | (buff[1] << 8) | buff[0];
  s_recvHeader.seq = (s_recvHeader.seq & 0) | buff[2];
  s_recvHeader.flags = (s_recvHeader.flags & 0) | buff[3];
  s_recvHeader.length = (s_recvHeader.length & 0) | (buff[5] << 8) | buff[4];
  s_recvHeader.crc32 =
    (s_recvHeader.crc32 & 0) |
    (buff[9] << 24) |
    (buff[8] << 16) |
    (buff[7] << 8) |
    buff[6];

  let tmpBuffer = Buffer.alloc(10);
  let magicBuffer = Buffer.from([
    s_recvHeader.magic & 0xff,
    (s_recvHeader.magic & 0xff00) >> 8,
    (s_recvHeader.magic & 0xff0000) >> 16,
    (s_recvHeader.magic & 0xff000000) >> 24
  ]);
  magicBuffer.copy(tmpBuffer, 0, 0, 4);
  buff.copy(tmpBuffer, 4, 0, 6);

  buff.fill();

  if (s_recvHeader.crc32 !== crc32.buf(tmpBuffer)) {
    // console.log("header crc err");
    recvState = "magic";
  } else {
    // console.log("header crc ok");
    recvState = "body";
  }
}

function checkBody(buff) {
  // console.log("checkBody");
  let pkg = new messagePackge();
  pkg.msg_header = s_recvHeader;
  pkg.body = buff;

  if (
    pkg.msg_header.flags !== 1 ||
    ((pkg.body[pkg.body.length - 1] << 24) |
      (pkg.body[pkg.body.length - 2] << 16) |
      (pkg.body[pkg.body.length - 3] << 8) |
      pkg.body[pkg.body.length - 4]) ===
    crc32.buf(pkg.body.slice(0, pkg.body.length - 4))
  ) {
    if (pkg.msg_header.flags === 1) {
      pkg.body.length -= 4;
      pkg.body = pkg.body.slice(0, pkg.body.length - 4);
    }
    _callRegistCallback(pkg);
  } else {
    // console.log("body crc err");
  }

  recvState = "magic";
}

let recvTmpBuff = Buffer.alloc(0);

function pushData(buff) {
  if (!Buffer.isBuffer(buff)) return;

  let tmp = Buffer.alloc(recvTmpBuff.length + buff.length);
  recvTmpBuff.copy(tmp);
  buff.copy(tmp, recvTmpBuff.length);
  recvTmpBuff = tmp;

  while (recvTmpBuff.length > 0) {
    if (recvState === "magic") {
      while (recvTmpBuff.length > 0) {
        let ch = recvTmpBuff[0];
        recvTmpBuff = recvTmpBuff.slice(1);
        if (cheackMagic(ch)) break;
      }
    }

    if (recvState === "header") {
      if (recvTmpBuff.length >= s_headerSize - 4) {
        let tmp = Buffer.alloc(s_headerSize - 4);
        recvTmpBuff.copy(tmp, 0, 0, s_headerSize - 4);
        if (recvTmpBuff.length > s_headerSize - 4)
          recvTmpBuff = recvTmpBuff.slice(s_headerSize - 4);
        else recvTmpBuff = Buffer.alloc(0);

        cheackHeader(tmp);
      } else {
        break;
      }
    }

    if (recvState === "body") {
      // console.log("state body");
      if (s_recvHeader.length === 0) {
        recvState = "magic";

        let pkg = new messagePackge();
        pkg.msg_header = s_recvHeader;
        pkg.body = undefined;
        // console.log(pkg);
        return _callRegistCallback(pkg);
      } else {
        // console.log("recvTmpBuff.length,s_recvHeader.length", recvTmpBuff.length, s_recvHeader.length)
        if (recvTmpBuff.length >= s_recvHeader.length) {
          let tmp = Buffer.alloc(s_recvHeader.length);
          recvTmpBuff.copy(tmp, 0, 0, s_recvHeader.length);
          recvTmpBuff = recvTmpBuff.slice(s_recvHeader.length);
          checkBody(tmp);
        } else {
          break;
        }
      }
    }
  }
}

function getType(obj) {
  //tostring会返回对应不同的标签的构造函数
  var toString = Object.prototype.toString;
  var map = {
    "[object Boolean]": "boolean",
    "[object Number]": "number",
    "[object String]": "string",
    "[object Function]": "function",
    "[object Array]": "array",
    "[object Date]": "date",
    "[object RegExp]": "regExp",
    "[object Undefined]": "undefined",
    "[object Null]": "null",
    "[object Object]": "object"
  };
  // if (obj instanceof Element) {
  //     return 'element';
  // }
  return map[toString.call(obj)];
}

function deepClone(data) {
  let type = getType(data);
  let obj;
  if (type === "array") {
    obj = [];
  } else if (type === "object") {
    obj = {};
  } else {
    //不再具有下一层次
    return data;
  }
  if (type === "array") {
    for (var i = 0, len = data.length; i < len; i++) {
      obj.push(deepClone(data[i]));
    }
  } else if (type === "object") {
    for (var key in data) {
      obj[key] = deepClone(data[key]);
    }
  }
  return obj;
}

module.exports = {
  messageRegistRecv,
  messageSendPkg,
  messagePackge,
  pushData
};
