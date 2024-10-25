/* Grus
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * kws_list.h: a Circular Queue using array
 *
 */

#ifndef __KWS_LIST_H__
#define __KWS_LIST_H__

#include <lvp_param.h>

LVP_KWS_PARAM g_kws_param_list[] = {
    {"alexa",              {4, 22, 12, 21, 30, 4}, 6, CONFIG_KEYWORD_ALEXA_THRESHOLD, CONFIG_KEYWORD_ALEXA_VALUE, 0},
    //这两维数据没用到，所以预设值1000
    {"sheila",             {0, 0, 0, 0}, 4, 1000, 150, 0},
    {"Hi_Xiaowen",         {0, 0, 0, 0}, 4, 1000, 150, 0},

    {"answer the phone",   {3, 24, 30, 13, 11, 19}, 6, CONFIG_KEYWORD_ANSWER_THE_PHONE_THRESHOLD, CONFIG_KEYWORD_ANSWER_THE_PHONE_VALUE, 0},
    {"hey siri",           {17,  7, 30, 18, 29, 19}, 6, CONFIG_KEYWORD_HEY_SIRI_THRESHOLD, CONFIG_KEYWORD_HEY_SIRI_VALUE, 1},
    {"ok google",          {26, 21, 14, 16, 35, 16, 4, 22}, 8, CONFIG_KEYWORD_OK_GOOGLE_THRESHOLD, CONFIG_KEYWORD_OK_GOOGLE_VALUE, 1},
    {"play music",         {28, 22, 14, 23, 38, 35, 39, 18, 21}, 9, CONFIG_KEYWORD_PLAY_MUSIC_THRESHOLD, CONFIG_KEYWORD_PLAY_MUSIC_VALUE, 0},
    {"play next song",     {28, 22, 14, 24, 12, 21, 30, 32, 30, 5, 25}, 11, CONFIG_KEYWORD_PLAY_NEXT_SONG_THRESHOLD, CONFIG_KEYWORD_PLAY_NEXT_SONG_VALUE, 0},
    {"play previous song", {28, 22, 14, 28, 29, 19, 36, 19, 4, 30}, 10, CONFIG_KEYWORD_PLAY_PREVIOUS_SONG_THRESHOLD, CONFIG_KEYWORD_PLAY_PREVIOUS_SONG_VALUE, 0},
    {"reject the call",    {29, 18, 20, 12, 21, 32, 11, 4, 21, 5, 22}, 11, CONFIG_KEYWORD_REJECT_THE_CALL_THRESHOLD, CONFIG_KEYWORD_REJECT_THE_CALL_VALUE, 0},
    {"stop playing",       {30, 32, 2, 28, 28, 22, 14, 18, 25}, 9, CONFIG_KEYWORD_STOP_PLAYING_THRESHOLD, CONFIG_KEYWORD_STOP_PLAYING_VALUE, 0},
    {"volume down",        {36, 2, 22, 38, 35, 23, 10, 6, 24}, 9, CONFIG_KEYWORD_VOLUME_DOWN_THRESHOLD, CONFIG_KEYWORD_VOLUME_DOWN_VALUE, 0},
    {"volume up",          {36, 2, 22, 38, 35, 23, 4, 28}, 8, CONFIG_KEYWORD_VOLUME_UP_THRESHOLD, CONFIG_KEYWORD_VOLUME_UP_VALUE, 0},
};

#endif /* __KWS_LIST_H__ */
