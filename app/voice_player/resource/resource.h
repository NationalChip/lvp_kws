#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#ifdef CONFIG_YILIAN_FAN_SOLUTION

//#include "dakaidengguang.c"
//#include "dakaifengshan.c"
//#include "dakaishengyin.c"
#include "dangweitiaoda.c"
#include "dangweitiaoxiao.c"
//#include "fengsutiaoda.c"
//#include "fengsutiaoxiao.c"
#include "fengsuzuida.c"
#include "fengsuzuixiao.c"
//#include "guanbidengguang.c"
//#include "guanbifengshan.c"
//#include "guanbishengyin.c"
#include "guanbiyaotou.c"
#include "guandingshi.c"
#include "guanfengshan.c"
#include "guanyaotou.c"
#include "jianxiaofengsu.c"
#include "jianxiaoyinliang.c"
#include "kaidingshi.c"
#include "kaifengshan.c"
#include "kaishiyaotou.c"
#include "kaiyaotou.c"
//#include "lafengmoshi.c"
//#include "shiyongshuoming.c"
#include "shuimianfeng.c"
//#include "shushimoshi.c"

#include "zengdafengsu.c"
#include "zengdayinliang.c"
#include "zhengchangfeng.c"
#include "ziranfeng.c"

#include "wozai.c"
//#include "wozaine.c"
//#include "wozaine16k.c"
//#include "wozaine8k.c"
//#include "xiaoduxiaodu.c"
#include "zaijian.c"
#endif

#ifdef CONFIG_PLAY_WAV_DATA
# include "wozai.h"
//# include "dkfengg.h"
//# include "dkref.h"
#endif

#ifdef CONFIG_PLAY_OPUS_DATA
# include "wozai.c"
#endif

#endif
