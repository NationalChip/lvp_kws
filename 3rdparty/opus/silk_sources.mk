SILK_SOURCES = \
$(ROOTDIR)/silk/code_signs.c \
$(ROOTDIR)/silk/init_decoder.c \
$(ROOTDIR)/silk/decode_core.c \
$(ROOTDIR)/silk/decode_frame.c \
$(ROOTDIR)/silk/decode_parameters.c \
$(ROOTDIR)/silk/decode_indices.c \
$(ROOTDIR)/silk/decode_pulses.c \
$(ROOTDIR)/silk/decoder_set_fs.c \
$(ROOTDIR)/silk/dec_API.c \
$(ROOTDIR)/silk/gain_quant.c \
$(ROOTDIR)/silk/NLSF_decode.c \
$(ROOTDIR)/silk/shell_coder.c \
$(ROOTDIR)/silk/tables_gain.c \
$(ROOTDIR)/silk/tables_NLSF_CB_WB.c \
$(ROOTDIR)/silk/tables_NLSF_CB_NB_MB.c \
$(ROOTDIR)/silk/tables_other.c \
$(ROOTDIR)/silk/tables_pulses_per_block.c \
$(ROOTDIR)/silk/decode_pitch.c \
$(ROOTDIR)/silk/tables_LTP.c \
$(ROOTDIR)/silk/log2lin.c \
$(ROOTDIR)/silk/LPC_analysis_filter.c \
$(ROOTDIR)/silk/table_LSF_cos.c \
$(ROOTDIR)/silk/NLSF2A.c \
$(ROOTDIR)/silk/pitch_est_tables.c \
$(ROOTDIR)/silk/resampler.c \
$(ROOTDIR)/silk/NLSF_unpack.c \
$(ROOTDIR)/silk/LPC_inv_pred_gain.c \
$(ROOTDIR)/silk/tables_pitch_lag.c \
$(ROOTDIR)/silk/LPC_fit.c

SILK_SOURCES_SSE4_1 =

SILK_SOURCES_ARM_NEON_INTR = \
$(ROOTDIR/)silk/arm/arm_silk_map.c \
$(ROOTDIR/)silk/arm/LPC_inv_pred_gain_neon_intr.c

SILK_SOURCES_FIXED =

SILK_SOURCES_FIXED_SSE4_1 =

SILK_SOURCES_FIXED_ARM_NEON_INTR =

SILK_SOURCES_FLOAT = \
$(ROOTDIR)/silk/float/apply_sine_window_FLP.c \
$(ROOTDIR)/silk/float/corrMatrix_FLP.c \
$(ROOTDIR)/silk/float/find_LPC_FLP.c \
$(ROOTDIR)/silk/float/find_LTP_FLP.c \
$(ROOTDIR)/silk/float/find_pitch_lags_FLP.c \
$(ROOTDIR)/silk/float/find_pred_coefs_FLP.c \
$(ROOTDIR)/silk/float/LPC_analysis_filter_FLP.c \
$(ROOTDIR)/silk/float/LTP_analysis_filter_FLP.c \
$(ROOTDIR)/silk/float/LTP_scale_ctrl_FLP.c \
$(ROOTDIR)/silk/float/noise_shape_analysis_FLP.c \
$(ROOTDIR)/silk/float/process_gains_FLP.c \
$(ROOTDIR)/silk/float/regularize_correlations_FLP.c \
$(ROOTDIR)/silk/float/residual_energy_FLP.c \
$(ROOTDIR)/silk/float/warped_autocorrelation_FLP.c \
$(ROOTDIR)/silk/float/wrappers_FLP.c \
$(ROOTDIR)/silk/float/autocorrelation_FLP.c \
$(ROOTDIR)/silk/float/burg_modified_FLP.c \
$(ROOTDIR)/silk/float/bwexpander_FLP.c \
$(ROOTDIR)/silk/float/energy_FLP.c \
$(ROOTDIR)/silk/float/inner_product_FLP.c \
$(ROOTDIR)/silk/float/k2a_FLP.c \
$(ROOTDIR)/silk/float/LPC_inv_pred_gain_FLP.c \
$(ROOTDIR)/silk/float/pitch_analysis_core_FLP.c \
$(ROOTDIR)/silk/float/scale_copy_vector_FLP.c \
$(ROOTDIR)/silk/float/scale_vector_FLP.c \
$(ROOTDIR)/silk/float/schur_FLP.c \
$(ROOTDIR)/silk/float/sort_FLP.c
