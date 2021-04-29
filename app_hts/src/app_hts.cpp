#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "mbed.h"
#include "FATFileSystem.h"
#undef USBHOST_MSD
#include "SdUsbConnect.h"
#include "AUDIO_GRBoard.h"

/* Main headers */
#include "mecab.h"
#include "njd.h"
#include "jpcommon.h"
#include "HTS_engine.h"

/* Sub headers */
#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"
#include "njd2jpcommon.h"

#include <vector>

#define MAXBUFLEN 1024

#define WRITE_BUFF_NUM         (8)
#define READ_BUFF_NUM          (8)
#define MAIL_QUEUE_SIZE        (WRITE_BUFF_NUM + READ_BUFF_NUM + 1)
#define INFO_TYPE_WRITE_END    (0)
#define INFO_TYPE_READ_END     (1)

#define AUDIO_BUFF_SIZE        (4096)
AUDIO_GRBoard Audio(0x80, WRITE_BUFF_NUM, READ_BUFF_NUM);

typedef struct {
	uint32_t info_type;
	void *   p_data;
	int32_t  result;
} mail_t;
Mail<mail_t, MAIL_QUEUE_SIZE> mail_box;

//32 bytes aligned! No cache memory
static short audio_read_buff[READ_BUFF_NUM][AUDIO_BUFF_SIZE / sizeof(short)] __attribute((section("NC_BSS"),aligned(32)));

static void callback_audio(void * p_data, int32_t result, void * p_app_data) {
	mail_t *mail = mail_box.alloc();

	if (mail == NULL) {
		printf("error mail alloc\r\n");
	} else {
		mail->info_type = (uint32_t)p_app_data;
		mail->p_data    = p_data;
		mail->result    = result;
		mail_box.put(mail);
	}
}

rbsp_data_conf_t audio_write_conf = { &callback_audio, (void *)INFO_TYPE_WRITE_END };
rbsp_data_conf_t audio_read_conf = { &callback_audio, (void *)INFO_TYPE_READ_END };

extern "C"
void HTS_Audio_initialize(HTS_Audio * audio)
{
	if (audio == NULL)
	  return;

	audio->sampling_frequency = 0;
	audio->max_buff_size = 0;
	audio->buff = NULL;
	audio->buff_size = 0;
	audio->audio_interface = NULL;
}

extern "C"
void HTS_Audio_set_parameter(HTS_Audio * audio, size_t sampling_frequency, size_t max_buff_size)
{
	if (audio == NULL)
		return;

	if (audio->sampling_frequency == sampling_frequency && audio->max_buff_size == max_buff_size)
		return;

	if (sampling_frequency == 0 || max_buff_size == 0)
	  return;

	Audio.outputVolume(0.7, 0.7);
	//Audio.micVolume(0.7);
	Audio.frequency(sampling_frequency);

	audio->audio_interface = &Audio;
	audio->sampling_frequency = sampling_frequency;
	audio->max_buff_size = max_buff_size;

    osEvent evt = mail_box.get();
    if (evt.status == osEventMail) {
        mail_t *mail = (mail_t *)evt.value.p;

		audio->buff = (short *)mail->p_data;
		audio->buff_size = 0;
    }
    else {
		audio->buff = NULL;
		audio->buff_size = 0;
    }
}

extern "C"
void HTS_Audio_write(HTS_Audio * audio, short data)
{
	if (audio == NULL || audio->audio_interface == NULL)
		return;

	audio->buff[audio->buff_size++] = data;
	audio->buff[audio->buff_size++] = data;

	if (audio->buff_size >= audio->max_buff_size) {
        osEvent evt = mail_box.get();
        if (evt.status == osEventMail) {
            mail_t *mail = (mail_t *)evt.value.p;
    		short *buff = audio->buff;
    		size_t size = audio->buff_size * sizeof(short);

    		audio->buff = (short *)mail->p_data;
    		audio->buff_size = 0;

            Audio.write(buff, size, &audio_write_conf);

            mail_box.free(mail);
        }
	}
}

extern "C"
void HTS_Audio_flush(HTS_Audio * audio)
{
	if (audio == NULL || audio->audio_interface == NULL)
		return;

	if (Audio.write(audio->buff, audio->buff_size * sizeof(short), &audio_write_conf) > 0) {
		return;
	}
	audio->buff_size = 0;
}

extern "C"
void HTS_Audio_clear(HTS_Audio * audio)
{
}

typedef struct _Open_JTalk {
	Mecab mecab;
	NJD njd;
	JPCommon jpcommon;
} Open_JTalk;

static void Open_JTalk_initialize(Open_JTalk *open_jtalk)
{
	Mecab_initialize(&open_jtalk->mecab);
	NJD_initialize(&open_jtalk->njd);
	JPCommon_initialize(&open_jtalk->jpcommon);
}

static void Open_JTalk_clear(Open_JTalk *open_jtalk)
{
	Mecab_clear(&open_jtalk->mecab);
	NJD_clear(&open_jtalk->njd);
	JPCommon_clear(&open_jtalk->jpcommon);
}

static int Open_JTalk_load(Open_JTalk *open_jtalk, const char *dn_mecab)
{
	if (Mecab_load(&open_jtalk->mecab, dn_mecab) != TRUE) {
		Open_JTalk_clear(open_jtalk);
		return 0;
	}
	return 1;
}

static int Open_JTalk_synthesis(Open_JTalk *open_jtalk, const char *txt,
	std::vector<char *> *code)
{
	char buff[MAXBUFLEN];
	int len;
	char **lines;

	text2mecab(buff, txt);
	Mecab_analysis(&open_jtalk->mecab, buff);
	mecab2njd(&open_jtalk->njd, Mecab_get_feature(&open_jtalk->mecab),
		Mecab_get_size(&open_jtalk->mecab));
	njd_set_pronunciation(&open_jtalk->njd);
	njd_set_digit(&open_jtalk->njd);
	njd_set_accent_phrase(&open_jtalk->njd);
	njd_set_accent_type(&open_jtalk->njd);
	njd_set_unvoiced_vowel(&open_jtalk->njd);
	njd_set_long_vowel(&open_jtalk->njd);
	njd2jpcommon(&open_jtalk->jpcommon, &open_jtalk->njd);
	JPCommon_make_label(&open_jtalk->jpcommon);

	if (code != NULL) {
		len = JPCommon_get_label_size(&open_jtalk->jpcommon);
		lines = JPCommon_get_label_feature(&open_jtalk->jpcommon);
		for (int i = 0; i < len; i++) {
			code->push_back(strdup(lines[i]));
		}
	}

	JPCommon_refresh(&open_jtalk->jpcommon);
	NJD_refresh(&open_jtalk->njd);
	Mecab_refresh(&open_jtalk->mecab);

	return 1;
}

#define MOUNT_NAME             "storage"
SdUsbConnect storage(MOUNT_NAME);

/* hts_engine API */
HTS_Engine engine;
/* Open JTalk */
Open_JTalk open_jtalk;

char input_buff[MAXBUFLEN];

extern "C" void wmem_init();

int main()
{
	const char *dn_dict = "/" MOUNT_NAME "/dic";
	const char *fn_voice = "/" MOUNT_NAME "/mei/mei_normal.htsvoice";

	Audio.power();

	// connect wait
	storage.wait_connect();

	for (int i = 0; i < READ_BUFF_NUM; i++) {
		mail_t *mail = mail_box.alloc();

		if (mail == NULL) {
			return 1;
		} else {
			mail->info_type = (uint32_t)INFO_TYPE_WRITE_END;
			mail->p_data    = audio_read_buff[i];
			mail->result    = AUDIO_BUFF_SIZE;
			mail_box.put(mail);
		}
    }

	for (;;) {
		memset(input_buff, 0, sizeof(input_buff));
		puts(">");
		char *p = input_buff, *end = &input_buff[256];
		while (p < end) {
			int c = getc(stdin);
			if (c == '\n'){
				break;
			}
			if (c <= 0)
				continue;
			putc(c, stdout);
			*p = c;
			p++;
		}
		*p = '\0';
		puts("\nanalize...");

		std::vector<char *> code;

		wmem_init();

		/* initialize Open JTalk */
		Open_JTalk_initialize(&open_jtalk);

		/* load dictionary and HTS voice */
		if (Open_JTalk_load(&open_jtalk, dn_dict) != TRUE) {
			fprintf(stderr, "Error: Dictionary or HTS voice cannot be loaded.\n");
			Open_JTalk_clear(&open_jtalk);
			continue;
		}

		/* synthesize */
		if (Open_JTalk_synthesis(&open_jtalk, input_buff, &code) != TRUE) {
			fprintf(stderr, "Error: Open_JTalk_synthesis.\n");
			Open_JTalk_clear(&open_jtalk);
			continue;
		}

		/* free memory */
		Open_JTalk_clear(&open_jtalk);

		int count = code.size();
		char **labels = (char **)malloc(count * sizeof(char *)), **pos = labels;
		for (auto i = code.begin(); i != code.end(); i++, pos++) {
			*pos = *i;
		}
		code.clear();

		puts("synthesize...");

		wmem_init();

		/* initialize hts_engine API */
		HTS_Engine_initialize(&engine);

		if (HTS_Engine_load(&engine, &fn_voice, 1) != TRUE) {
			fprintf(stderr, "Error: HTS_Engine_load.\n");
			HTS_Engine_clear(&engine);
			continue;
		}

		if (strcmp(HTS_Engine_get_fullcontext_label_format(&engine), "HTS_TTS_JPN") != 0) {
			fprintf(stderr, "Error: HTS_Engine_get_fullcontext_label_format.\n");
			HTS_Engine_clear(&engine);
			continue;
		}

		//HTS_Engine_set_speed(&engine, 100);

		//HTS_Engine_set_sampling_frequency(&engine, 48000);

		HTS_Engine_set_audio_buff_size(&engine, AUDIO_BUFF_SIZE / sizeof(short));

		/* synthesize */
		if (HTS_Engine_synthesize_from_strings(&engine, labels, count) != TRUE) {
			fprintf(stderr, "Error: waveform cannot be synthesized.\n");
			HTS_Engine_clear(&engine);
			continue;
		}

		/* free memory */
		HTS_Engine_clear(&engine);

		pos = labels;
		for (int i = 0; i < count; i++, pos++) {
			free(*pos);
		}
		free(labels);

		puts("\n");
	}

	return 0;
}
