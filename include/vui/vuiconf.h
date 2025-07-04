#ifndef __VUI_CONF_H__
#define __VUI_CONF_H__
// VUI Settings
//

//
// VAD Settings:
//
#define CONFIG_LVP_FFT_VAD_ENABLE
#define CONFIG_LVP_FFT_INVALID_VAD_NUM 25
#define CONFIG_LVP_VAD_FALLBACK_CONTEXT 3
#define CONFIG_LVP_ENABLE_AUTO_ADJUST_VAD_PARAMETER

//
// 
//
// CONFIG_LVP_ADVANCE_HUMAN_VAD_ENABLE is not set

//
// 
//

//
// ENV Noise Judgment Settings:
//
// CONFIG_ENABLE_NOISE_JUDGEMENT is not set

//
// 
//
#define CONFIG_LVP_STANDBY_ENABLE
#define CONFIG_LVP_DISABLE_XIP_WHILE_CODE_RUN_AT_SRAM
// CONFIG_LVP_ENABLE_ENERGY_VAD is not set
#define CONFIG_LVP_STATE_FVAD_COUNT_DOWN 18

//
// 
//
#define CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
#define CONFIG_KWS_SNPU_BUFFER_SIZE 2996
#define CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME 40
#define CONFIG_KWS_MODEL_INPUT_STRIDE_LENGTH 4
#define CONFIG_KWS_MODEL_INPUT_WIN_LENGTH 15
#define CONFIG_KWS_MODEL_OUTPUT_LENGTH 65
#define CONFIG_KWS_MODEL_SUPPORT_SOFTMAX
#define CONFIG_PREFIX_CHAR_LENGTH 8
#define CONFIG_NORMAL_CTC_SCORE
// CONFIG_ENABLE_CTC_KWS_AND_BUN_KWS_CASCADE is not set
#define CONFIG_LVP_ENABLE_CTC_DECODER
// CONFIG_LVP_ENABLE_CTC_GX_DECODER is not set
// CONFIG_LVP_ENABLE_CTC_AND_BEAMSEARCH_DECODER is not set
// CONFIG_LVP_ENABLE_BEAMSEARCH_DECODER is not set
// CONFIG_LVP_ENABLE_USER_DECODER is not set
// CONFIG_LVP_ENABLE_MAX_DECODER is not set
// CONFIG_LVP_KWS_ALEXA is not set
#define CONFIG_LVP_KWS_GRUS_TWS_ZH
// CONFIG_LVP_KWS_XIAOSHU_AIOT is not set
#define CONFIG_LVP_KWS_GRUS_TWS_ZH_V0DOT2DOT0_2021_0527
// CONFIG_LVP_KWS_GRUS_TWS_ZH_V0DOT2DOT1_2021_0720 is not set
// CONFIG_LVP_KWS_GRUS_TWS_ZH_V0DOT2DOT1_V0DOT1DOT9_2021_0722 is not set
// CONFIG_LVP_KWS_GRUS_TWS_ZH_V0DOT2DOT2_V0DOT1DOT9_2021_0728 is not set
#define CONFIG_KWS_MODEL_DECODER_STRIDE_LENGTH 4
#define CONFIG_KWS_MODEL_DECODER_WIN_LENGTH 38

//
// Model Param Setting:
//

//
// Kws List Settings
//
#define CONFIG_KEYWORD_ENABLE_XIAO_AI_TONG_XUE
#define CONFIG_KEYWORD_XIAO_AI_TONG_XUE_THRESHOLD 942
#define CONFIG_KEYWORD_XIAO_AI_TONG_XUE_VALUE 100

//
// 
//
// CONFIG_KEYWORD_ENABLE_XIAO_DU_XIAO_DU is not set
// CONFIG_KEYWORD_ENABLE_TIAN_MAO_JING_LING is not set
#define CONFIG_KEYWORD_ENABLE_JIE_TING_DIAN_HUA
#define CONFIG_KEYWORD_JIE_TING_DIAN_HUA_THRESHOLD 872
#define CONFIG_KEYWORD_JIE_TING_DIAN_HUA_VALUE 104

//
// 
//
#define CONFIG_KEYWORD_ENABLE_GUA_DIAO_DIAN_HUA
#define CONFIG_KEYWORD_GUA_DIAO_DIAN_HUA_THRESHOLD 881
#define CONFIG_KEYWORD_GUA_DIAO_DIAN_HUA_VALUE 115

//
// 
//
#define CONFIG_KEYWORD_ENABLE_GUA_DUAN_DIAN_HUA
#define CONFIG_KEYWORD_GUA_DUAN_DIAN_HUA_THRESHOLD 881
#define CONFIG_KEYWORD_GUA_DUAN_DIAN_HUA_VALUE 105

//
// 
//
#define CONFIG_KEYWORD_ENABLE_BO_FANG_YIN_YUE
#define CONFIG_KEYWORD_BO_FANG_YIN_YUE_THRESHOLD 887
#define CONFIG_KEYWORD_BO_FANG_YIN_YUE_VALUE 106

//
// 
//
// CONFIG_KEYWORD_ENABLE_JI_XU_BO_FANG is not set
// CONFIG_KEYWORD_ENABLE_ZAN_TING_YIN_YUE is not set
// CONFIG_KEYWORD_ENABLE_TING_ZHI_YIN_YUE is not set
#define CONFIG_KEYWORD_ENABLE_ZAN_TING_BO_FANG
#define CONFIG_KEYWORD_ZAN_TING_BO_FANG_THRESHOLD 884
#define CONFIG_KEYWORD_ZAN_TING_BO_FANG_VALUE 110

//
// 
//
// CONFIG_KEYWORD_ENABLE_TING_ZHI_BO_FANG is not set
#define CONFIG_KEYWORD_ENABLE_ZENG_DA_YIN_LIANG
#define CONFIG_KEYWORD_ZENG_DA_YIN_LIANG_THRESHOLD 869
#define CONFIG_KEYWORD_ZENG_DA_YIN_LIANG_VALUE 111

//
// 
//
#define CONFIG_KEYWORD_ENABLE_JIAN_XIAO_YIN_LIANG
#define CONFIG_KEYWORD_JIAN_XIAO_YIN_LIANG_THRESHOLD 890
#define CONFIG_KEYWORD_JIAN_XIAO_YIN_LIANG_VALUE 112

//
// 
//
// CONFIG_KEYWORD_ENABLE_JIA_DA_YIN_LIANG is not set
// CONFIG_KEYWORD_ENABLE_JIANG_DI_YIN_LIANG is not set
#define CONFIG_KEYWORD_ENABLE_SHANG_YI_SHOU
#define CONFIG_KEYWORD_SHANG_YI_SHOU_THRESHOLD 889
#define CONFIG_KEYWORD_SHANG_YI_SHOU_VALUE 115

//
// 
//
#define CONFIG_KEYWORD_ENABLE_XIA_YI_SHOU
#define CONFIG_KEYWORD_XIA_YI_SHOU_THRESHOLD 904
#define CONFIG_KEYWORD_XIA_YI_SHOU_VALUE 116

//
// 
//
// CONFIG_KEYWORD_ENABLE_SHANG_YI_QU is not set
// CONFIG_KEYWORD_ENABLE_XIA_YI_QU is not set
// CONFIG_KEYWORD_ENABLE_DA_KAI_DENG_GUANG is not set
// CONFIG_KEYWORD_ENABLE_GUAN_BI_DENG_GUNAG is not set
// CONFIG_USE_CTC_VERSION_V0DOT0DOT1 is not set
#define CONFIG_USE_CTC_VERSION_V0DOT0DOT3
#define CONFIG_LVP_MODEL_USE_RNN
// CONFIG_LVP_ENABLE_CTC_SCORE_COMPENSATOR is not set
// CONFIG_LVP_ENABLE_OPTIMIZE_SHORT_INSTRUCTIONS is not set
// CONFIG_LVP_ENABLE_CTC_BIONIC is not set
// CONFIG_ENABLE_CTC_JUDGE_TO_REDUCE_FALSE_ACTIVATION is not set

//
// 
//
// CONFIG_VPA_ENABLE_ONLY_PRINTF_SCORE_WITHOUT_ACTIVATE is not set
// CONFIG_VPA_ENABLE_DELAY_TO_DECODE is not set

//
// 
//
// CONFIG_LVP_ENABLE_VOICE_PRINT_RECOGNITION is not set

//
// 
//

//
// 
//

//
// LVP Application Settings
#endif // __VUI_CONF_H__
