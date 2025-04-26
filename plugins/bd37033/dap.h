#ifndef _DAP_H__
#define _DAP_H__


enum _speaker_ch
{
	SPEAKER_CH_LF = 0,
	SPEAKER_CH_RF,
	SPEAKER_CH_LR,
	SPEAKER_CH_RR,
	SPEAKER_CH_MAX
};

typedef struct _dap_t
{
	int (*init)(void);
	int (*deinit)(void);
	int (*set_source)(uint8_t source);
	int (*set_loudness)(uint8_t enable);
	int (*set_volume)(uint8_t vol);
	int (*get_volume)(void);
	int (*set_bass)(int bass);
	int (*set_treble)(int treble);
	int (*set_speaker)(int channel, int volume);
	int (*set_mute)(int enable);
}dap_t; 

dap_t *register_dap(void);

#endif


