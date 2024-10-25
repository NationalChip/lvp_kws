#pragma once
#ifndef CONFIG_LVP_ENABLE_KEYWORD_RECOGNITION
# define CONFIG_KWS_MODEL_OUTPUT_LENGTH          65
# define CONFIG_KWS_MODEL_INPUT_WIN_LENGTH       17
# define CONFIG_KWS_MODEL_FEATURES_DIM_PER_FRAME 40
# define CONFIG_KWS_MODEL_DECODER_WIN_LENGTH     23
# define CONFIG_KWS_SNPU_BUFFER_SIZE             2772
#endif

#ifdef CONFIG_PCM_SAMPLE_RATE_8K
# define PCM_SAMPLE_RATE     8000
#endif

#ifdef CONFIG_PCM_SAMPLE_RATE_16K
# define PCM_SAMPLE_RATE     16000
#endif

#ifdef CONFIG_PCM_SAMPLE_RATE_48K
# define PCM_SAMPLE_RATE     48000
#endif

#ifndef PCM_SAMPLE_RATE
# error "Unknown Sample Rate"
#endif

#ifdef CONFIG_PCM_FRAME_LENGTH_10MS
# define PCM_FRAME_LENGTH    10
#endif

#ifdef CONFIG_PCM_FRAME_LENGTH_16MS
# define PCM_FRAME_LENGTH    16
#endif

#ifndef PCM_FRAME_LENGTH
# error "Unknown Frame Length"
#endif

// size of a channel and a frame
#define PCM_FRAME_SIZE      (PCM_SAMPLE_RATE * PCM_FRAME_LENGTH / 1000)