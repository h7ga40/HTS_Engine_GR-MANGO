#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "mbed.h"
#include "FATFileSystem.h"
#undef USBHOST_MSD
#include "SdUsbConnect.h"
#include "AUDIO_GRBoard.h"
#include "HTS_engine.h"

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

#define MOUNT_NAME             "storage"
SdUsbConnect storage(MOUNT_NAME);

const char *const label1[] = {
	"xx^xx-sil+k=o/A:xx+xx+xx/B:xx-xx_xx/C:xx_xx+xx/D:09+xx_xx/E:xx_xx!xx_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:5_5%0_xx_xx/H:xx_xx/I:xx-xx@xx+xx&xx-xx|xx+xx/J:1_5/K:4+5-20",
	"xx^sil-k+o=N/A:-4+1+5/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"sil^k-o+N=n/A:-4+1+5/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"k^o-N+n=i/A:-3+2+4/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"o^N-n+i=ch/A:-2+3+3/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"N^n-i+ch=i/A:-2+3+3/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"n^i-ch+i=w/A:-1+4+2/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"i^ch-i+w=a/A:-1+4+2/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"ch^i-w+a=pau/A:0+5+1/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"i^w-a+pau=w/A:0+5+1/B:xx-xx_xx/C:09_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:5_5#0_xx@1_1|1_5/G:4_4%0_xx_0/H:xx_xx/I:1-5@1+4&1-5|1+20/J:1_4/K:4+5-20",
	"w^a-pau+w=a/A:xx+xx+xx/B:09-xx_xx/C:xx_xx+xx/D:04+xx_xx/E:5_5!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:4_4%0_xx_xx/H:1_5/I:xx-xx@xx+xx&xx-xx|xx+xx/J:1_4/K:4+5-20",
	"a^pau-w+a=t/A:-3+1+4/B:09-xx_xx/C:04_xx+xx/D:24+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"pau^w-a+t=a/A:-3+1+4/B:09-xx_xx/C:04_xx+xx/D:24+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"w^a-t+a=sh/A:-2+2+3/B:09-xx_xx/C:04_xx+xx/D:24+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"a^t-a+sh=i/A:-2+2+3/B:09-xx_xx/C:04_xx+xx/D:24+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"t^a-sh+i=w/A:-1+3+2/B:09-xx_xx/C:04_xx+xx/D:24+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"a^sh-i+w=a/A:-1+3+2/B:09-xx_xx/C:04_xx+xx/D:24+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"sh^i-w+a=pau/A:0+4+1/B:04-xx_xx/C:24_xx+xx/D:06+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"i^w-a+pau=j/A:0+4+1/B:04-xx_xx/C:24_xx+xx/D:06+xx_xx/E:5_5!0_xx-0/F:4_4#0_xx@1_1|1_4/G:2_2%0_xx_0/H:1_5/I:1-4@2+3&2-4|6+15/J:2_5/K:4+5-20",
	"w^a-pau+j=i/A:xx+xx+xx/B:24-xx_xx/C:xx_xx+xx/D:06+xx_xx/E:4_4!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:2_2%0_xx_xx/H:1_4/I:xx-xx@xx+xx&xx-xx|xx+xx/J:2_5/K:4+5-20",
	"a^pau-j+i=i/A:-1+1+2/B:24-xx_xx/C:06_xx+xx/D:02+xx_xx/E:4_4!0_xx-0/F:2_2#0_xx@1_2|1_5/G:3_1%0_xx_1/H:1_4/I:2-5@3+2&3-3|10+11/J:1_6/K:4+5-20",
	"pau^j-i+i=a/A:-1+1+2/B:24-xx_xx/C:06_xx+xx/D:02+xx_xx/E:4_4!0_xx-0/F:2_2#0_xx@1_2|1_5/G:3_1%0_xx_1/H:1_4/I:2-5@3+2&3-3|10+11/J:1_6/K:4+5-20",
	"j^i-i+a=a/A:0+2+1/B:24-xx_xx/C:06_xx+xx/D:02+xx_xx/E:4_4!0_xx-0/F:2_2#0_xx@1_2|1_5/G:3_1%0_xx_1/H:1_4/I:2-5@3+2&3-3|10+11/J:1_6/K:4+5-20",
	"i^i-a+a=r/A:0+1+3/B:06-xx_xx/C:02_xx+xx/D:02+xx_xx/E:2_2!0_xx-1/F:3_1#0_xx@2_1|3_3/G:6_1%0_xx_0/H:1_4/I:2-5@3+2&3-3|10+11/J:1_6/K:4+5-20",
	"i^a-a+r=u/A:1+2+2/B:06-xx_xx/C:02_xx+xx/D:02+xx_xx/E:2_2!0_xx-1/F:3_1#0_xx@2_1|3_3/G:6_1%0_xx_0/H:1_4/I:2-5@3+2&3-3|10+11/J:1_6/K:4+5-20",
	"a^a-r+u=pau/A:2+3+1/B:06-xx_xx/C:02_xx+xx/D:02+xx_xx/E:2_2!0_xx-1/F:3_1#0_xx@2_1|3_3/G:6_1%0_xx_0/H:1_4/I:2-5@3+2&3-3|10+11/J:1_6/K:4+5-20",
	"a^r-u+pau=m/A:2+3+1/B:06-xx_xx/C:02_xx+xx/D:02+xx_xx/E:2_2!0_xx-1/F:3_1#0_xx@2_1|3_3/G:6_1%0_xx_0/H:1_4/I:2-5@3+2&3-3|10+11/J:1_6/K:4+5-20",
	"r^u-pau+m=a/A:xx+xx+xx/B:02-xx_xx/C:xx_xx+xx/D:02+xx_xx/E:3_1!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:6_1%0_xx_xx/H:2_5/I:xx-xx@xx+xx&xx-xx|xx+xx/J:1_6/K:4+5-20",
	"u^pau-m+a=N/A:0+1+6/B:02-xx_xx/C:02_xx+xx/D:10+7_2/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"pau^m-a+N=g/A:0+1+6/B:02-xx_xx/C:02_xx+xx/D:10+7_2/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"m^a-N+g=o/A:1+2+5/B:02-xx_xx/C:02_xx+xx/D:10+7_2/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"a^N-g+o=o/A:2+3+4/B:02-xx_xx/C:02_xx+xx/D:10+7_2/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"N^g-o+o=d/A:2+3+4/B:02-xx_xx/C:02_xx+xx/D:10+7_2/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"g^o-o+d=e/A:3+4+3/B:02-xx_xx/C:02_xx+xx/D:10+7_2/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"o^o-d+e=s/A:4+5+2/B:02-xx_xx/C:10_7+2/D:xx+xx_xx/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"o^d-e+s=U/A:4+5+2/B:02-xx_xx/C:10_7+2/D:xx+xx_xx/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"d^e-s+U=sil/A:5+6+1/B:02-xx_xx/C:10_7+2/D:xx+xx_xx/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"e^s-U+sil=xx/A:5+6+1/B:02-xx_xx/C:10_7+2/D:xx+xx_xx/E:3_1!0_xx-0/F:6_1#0_xx@1_1|1_6/G:xx_xx%xx_xx_xx/H:2_5/I:1-6@4+1&5-1|15+6/J:xx_xx/K:4+5-20",
	"s^U-sil+xx=xx/A:xx+xx+xx/B:10-7_2/C:xx_xx+xx/D:xx+xx_xx/E:6_1!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:xx_xx%xx_xx_xx/H:1_6/I:xx-xx@xx+xx&xx-xx|xx+xx/J:xx_xx/K:4+5-20",
};

const char *const label2[] = {
	"xx^xx-sil+w=a/A:xx+xx+xx/B:xx-xx_xx/C:xx_xx+xx/D:04+xx_xx/E:xx_xx!xx_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:4_4%0_xx_xx/H:xx_xx/I:xx-xx@xx+xx&xx-xx|xx+xx/J:1_4/K:3+7-25",
	"xx^sil-w+a=t/A:-3+1+4/B:xx-xx_xx/C:04_xx+xx/D:24+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"sil^w-a+t=a/A:-3+1+4/B:xx-xx_xx/C:04_xx+xx/D:24+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"w^a-t+a=sh/A:-2+2+3/B:xx-xx_xx/C:04_xx+xx/D:24+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"a^t-a+sh=i/A:-2+2+3/B:xx-xx_xx/C:04_xx+xx/D:24+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"t^a-sh+i=w/A:-1+3+2/B:xx-xx_xx/C:04_xx+xx/D:24+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"a^sh-i+w=a/A:-1+3+2/B:xx-xx_xx/C:04_xx+xx/D:24+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"sh^i-w+a=pau/A:0+4+1/B:04-xx_xx/C:24_xx+xx/D:02+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"i^w-a+pau=j/A:0+4+1/B:04-xx_xx/C:24_xx+xx/D:02+xx_xx/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_1|1_4/G:5_5%0_xx_0/H:xx_xx/I:1-4@1+3&1-7|1+25/J:4_14/K:3+7-25",
	"w^a-pau+j=o/A:xx+xx+xx/B:24-xx_xx/C:xx_xx+xx/D:02+xx_xx/E:4_4!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:5_5%0_xx_xx/H:1_4/I:xx-xx@xx+xx&xx-xx|xx+xx/J:4_14/K:3+7-25",
	"a^pau-j+o=o/A:-4+1+5/B:24-xx_xx/C:02_xx+xx/D:23+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"pau^j-o+o=h/A:-4+1+5/B:24-xx_xx/C:02_xx+xx/D:23+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"j^o-o+h=o/A:-3+2+4/B:24-xx_xx/C:02_xx+xx/D:23+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"o^o-h+o=o/A:-2+3+3/B:24-xx_xx/C:02_xx+xx/D:23+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"o^h-o+o=n/A:-2+3+3/B:24-xx_xx/C:02_xx+xx/D:23+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"h^o-o+n=o/A:-1+4+2/B:24-xx_xx/C:02_xx+xx/D:23+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"o^o-n+o=u/A:0+5+1/B:02-xx_xx/C:23_xx+xx/D:02+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"o^n-o+u=m/A:0+5+1/B:02-xx_xx/C:23_xx+xx/D:02+xx_xx/E:4_4!0_xx-0/F:5_5#0_xx@1_4|1_14/G:3_1%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"n^o-u+m=i/A:0+1+3/B:23-xx_xx/C:02_xx+xx/D:13+xx_xx/E:5_5!0_xx-1/F:3_1#0_xx@2_3|6_9/G:4_4%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"o^u-m+i=d/A:1+2+2/B:23-xx_xx/C:02_xx+xx/D:13+xx_xx/E:5_5!0_xx-1/F:3_1#0_xx@2_3|6_9/G:4_4%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"u^m-i+d=e/A:1+2+2/B:23-xx_xx/C:02_xx+xx/D:13+xx_xx/E:5_5!0_xx-1/F:3_1#0_xx@2_3|6_9/G:4_4%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"m^i-d+e=h/A:2+3+1/B:02-xx_xx/C:13_xx+xx/D:03+xx_xx/E:5_5!0_xx-1/F:3_1#0_xx@2_3|6_9/G:4_4%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"i^d-e+h=a/A:2+3+1/B:02-xx_xx/C:13_xx+xx/D:03+xx_xx/E:5_5!0_xx-1/F:3_1#0_xx@2_3|6_9/G:4_4%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"d^e-h+a=cl/A:-3+1+4/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:3_1!0_xx-1/F:4_4#0_xx@3_2|9_6/G:2_2%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"e^h-a+cl=s/A:-3+1+4/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:3_1!0_xx-1/F:4_4#0_xx@3_2|9_6/G:2_2%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"h^a-cl+s=e/A:-2+2+3/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:3_1!0_xx-1/F:4_4#0_xx@3_2|9_6/G:2_2%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"a^cl-s+e=e/A:-1+3+2/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:3_1!0_xx-1/F:4_4#0_xx@3_2|9_6/G:2_2%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"cl^s-e+e=sh/A:-1+3+2/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:3_1!0_xx-1/F:4_4#0_xx@3_2|9_6/G:2_2%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"s^e-e+sh=I/A:0+4+1/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:3_1!0_xx-1/F:4_4#0_xx@3_2|9_6/G:2_2%0_xx_1/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"e^e-sh+I=t/A:-1+1+2/B:03-xx_xx/C:20_4+1/D:10+7_2/E:4_4!0_xx-1/F:2_2#0_xx@4_1|13_2/G:4_4%0_xx_0/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"e^sh-I+t=a/A:-1+1+2/B:03-xx_xx/C:20_4+1/D:10+7_2/E:4_4!0_xx-1/F:2_2#0_xx@4_1|13_2/G:4_4%0_xx_0/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"sh^I-t+a=pau/A:0+2+1/B:20-4_1/C:10_7+2/D:03+xx_xx/E:4_4!0_xx-1/F:2_2#0_xx@4_1|13_2/G:4_4%0_xx_0/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"I^t-a+pau=s/A:0+2+1/B:20-4_1/C:10_7+2/D:03+xx_xx/E:4_4!0_xx-1/F:2_2#0_xx@4_1|13_2/G:4_4%0_xx_0/H:1_4/I:4-14@2+2&2-6|5+21/J:2_7/K:3+7-25",
	"t^a-pau+s=e/A:xx+xx+xx/B:10-7_2/C:xx_xx+xx/D:03+xx_xx/E:2_2!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:4_4%0_xx_xx/H:4_14/I:xx-xx@xx+xx&xx-xx|xx+xx/J:2_7/K:3+7-25",
	"a^pau-s+e=i/A:-3+1+4/B:10-7_2/C:03_xx+xx/D:20+1_1/E:2_2!0_xx-0/F:4_4#0_xx@1_2|1_7/G:3_3%0_xx_1/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"pau^s-e+i=m/A:-3+1+4/B:10-7_2/C:03_xx+xx/D:20+1_1/E:2_2!0_xx-0/F:4_4#0_xx@1_2|1_7/G:3_3%0_xx_1/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"s^e-i+m=e/A:-2+2+3/B:10-7_2/C:03_xx+xx/D:20+1_1/E:2_2!0_xx-0/F:4_4#0_xx@1_2|1_7/G:3_3%0_xx_1/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"e^i-m+e=i/A:-1+3+2/B:10-7_2/C:03_xx+xx/D:20+1_1/E:2_2!0_xx-0/F:4_4#0_xx@1_2|1_7/G:3_3%0_xx_1/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"i^m-e+i=t/A:-1+3+2/B:10-7_2/C:03_xx+xx/D:20+1_1/E:2_2!0_xx-0/F:4_4#0_xx@1_2|1_7/G:3_3%0_xx_1/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"m^e-i+t=a/A:0+4+1/B:10-7_2/C:03_xx+xx/D:20+1_1/E:2_2!0_xx-0/F:4_4#0_xx@1_2|1_7/G:3_3%0_xx_1/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"e^i-t+a=i/A:-2+1+3/B:03-xx_xx/C:20_1+1/D:10+7_2/E:4_4!0_xx-1/F:3_3#0_xx@2_1|5_3/G:xx_xx%xx_xx_xx/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"i^t-a+i=d/A:-2+1+3/B:03-xx_xx/C:20_1+1/D:10+7_2/E:4_4!0_xx-1/F:3_3#0_xx@2_1|5_3/G:xx_xx%xx_xx_xx/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"t^a-i+d=a/A:-1+2+2/B:03-xx_xx/C:20_1+1/D:10+7_2/E:4_4!0_xx-1/F:3_3#0_xx@2_1|5_3/G:xx_xx%xx_xx_xx/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"a^i-d+a=sil/A:0+3+1/B:20-1_1/C:10_7+2/D:xx+xx_xx/E:4_4!0_xx-1/F:3_3#0_xx@2_1|5_3/G:xx_xx%xx_xx_xx/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"i^d-a+sil=xx/A:0+3+1/B:20-1_1/C:10_7+2/D:xx+xx_xx/E:4_4!0_xx-1/F:3_3#0_xx@2_1|5_3/G:xx_xx%xx_xx_xx/H:4_14/I:2-7@3+1&6-2|19+7/J:xx_xx/K:3+7-25",
	"d^a-sil+xx=xx/A:xx+xx+xx/B:10-7_2/C:xx_xx+xx/D:xx+xx_xx/E:3_3!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:xx_xx%xx_xx_xx/H:2_7/I:xx-xx@xx+xx&xx-xx|xx+xx/J:xx_xx/K:3+7-25",
};

const char *const label3[] = {
	"xx^xx-sil+k=a/A:xx+xx+xx/B:xx-xx_xx/C:xx_xx+xx/D:03+xx_xx/E:xx_xx!xx_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:4_4%0_xx_xx/H:xx_xx/I:xx-xx@xx+xx&xx-xx|xx+xx/J:2_6/K:2+4-15",
	"xx^sil-k+a=k/A:-3+1+4/B:xx-xx_xx/C:03_xx+xx/D:20+4_1/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_2|1_6/G:2_2%0_xx_1/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"sil^k-a+k=u/A:-3+1+4/B:xx-xx_xx/C:03_xx+xx/D:20+4_1/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_2|1_6/G:2_2%0_xx_1/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"k^a-k+u=n/A:-2+2+3/B:xx-xx_xx/C:03_xx+xx/D:20+4_1/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_2|1_6/G:2_2%0_xx_1/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"a^k-u+n=i/A:-2+2+3/B:xx-xx_xx/C:03_xx+xx/D:20+4_1/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_2|1_6/G:2_2%0_xx_1/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"k^u-n+i=N/A:-1+3+2/B:xx-xx_xx/C:03_xx+xx/D:20+4_1/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_2|1_6/G:2_2%0_xx_1/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"u^n-i+N=sh/A:-1+3+2/B:xx-xx_xx/C:03_xx+xx/D:20+4_1/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_2|1_6/G:2_2%0_xx_1/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"n^i-N+sh=I/A:0+4+1/B:xx-xx_xx/C:03_xx+xx/D:20+4_1/E:xx_xx!xx_xx-xx/F:4_4#0_xx@1_2|1_6/G:2_2%0_xx_1/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"i^N-sh+I=t/A:-1+1+2/B:03-xx_xx/C:20_4+1/D:10+7_2/E:4_4!0_xx-1/F:2_2#0_xx@2_1|5_2/G:6_3%0_xx_0/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"N^sh-I+t=a/A:-1+1+2/B:03-xx_xx/C:20_4+1/D:10+7_2/E:4_4!0_xx-1/F:2_2#0_xx@2_1|5_2/G:6_3%0_xx_0/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"sh^I-t+a=pau/A:0+2+1/B:20-4_1/C:10_7+2/D:02+xx_xx/E:4_4!0_xx-1/F:2_2#0_xx@2_1|5_2/G:6_3%0_xx_0/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"I^t-a+pau=m/A:0+2+1/B:20-4_1/C:10_7+2/D:02+xx_xx/E:4_4!0_xx-1/F:2_2#0_xx@2_1|5_2/G:6_3%0_xx_0/H:xx_xx/I:2-6@1+2&1-4|1+15/J:2_9/K:2+4-15",
	"t^a-pau+m=a/A:xx+xx+xx/B:10-7_2/C:xx_xx+xx/D:02+xx_xx/E:2_2!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:6_3%0_xx_xx/H:2_6/I:xx-xx@xx+xx&xx-xx|xx+xx/J:2_9/K:2+4-15",
	"a^pau-m+a=ch/A:-2+1+6/B:10-7_2/C:02_xx+xx/D:10+7_1/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"pau^m-a+ch=i/A:-2+1+6/B:10-7_2/C:02_xx+xx/D:10+7_1/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"m^a-ch+i=g/A:-1+2+5/B:10-7_2/C:02_xx+xx/D:10+7_1/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"a^ch-i+g=a/A:-1+2+5/B:10-7_2/C:02_xx+xx/D:10+7_1/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"ch^i-g+a=i/A:0+3+4/B:10-7_2/C:02_xx+xx/D:10+7_1/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"i^g-a+i=n/A:0+3+4/B:10-7_2/C:02_xx+xx/D:10+7_1/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"g^a-i+n=a/A:1+4+3/B:10-7_2/C:02_xx+xx/D:10+7_1/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"a^i-n+a=k/A:2+5+2/B:02-xx_xx/C:10_7+1/D:04+xx_xx/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"i^n-a+k=U/A:2+5+2/B:02-xx_xx/C:10_7+1/D:04+xx_xx/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"n^a-k+U=k/A:3+6+1/B:02-xx_xx/C:10_7+1/D:04+xx_xx/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"a^k-U+k=a/A:3+6+1/B:02-xx_xx/C:10_7+1/D:04+xx_xx/E:2_2!0_xx-0/F:6_3#0_xx@1_2|1_9/G:3_1%0_xx_1/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"k^U-k+a=r/A:0+1+3/B:10-7_1/C:04_xx+xx/D:10+7_2/E:6_3!0_xx-1/F:3_1#0_xx@2_1|7_3/G:xx_xx%xx_xx_xx/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"U^k-a+r=e/A:0+1+3/B:10-7_1/C:04_xx+xx/D:10+7_2/E:6_3!0_xx-1/F:3_1#0_xx@2_1|7_3/G:xx_xx%xx_xx_xx/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"k^a-r+e=d/A:1+2+2/B:10-7_1/C:04_xx+xx/D:10+7_2/E:6_3!0_xx-1/F:3_1#0_xx@2_1|7_3/G:xx_xx%xx_xx_xx/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"a^r-e+d=a/A:1+2+2/B:10-7_1/C:04_xx+xx/D:10+7_2/E:6_3!0_xx-1/F:3_1#0_xx@2_1|7_3/G:xx_xx%xx_xx_xx/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"r^e-d+a=sil/A:2+3+1/B:04-xx_xx/C:10_7+2/D:xx+xx_xx/E:6_3!0_xx-1/F:3_1#0_xx@2_1|7_3/G:xx_xx%xx_xx_xx/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"e^d-a+sil=xx/A:2+3+1/B:04-xx_xx/C:10_7+2/D:xx+xx_xx/E:6_3!0_xx-1/F:3_1#0_xx@2_1|7_3/G:xx_xx%xx_xx_xx/H:2_6/I:2-9@2+1&3-2|7+9/J:xx_xx/K:2+4-15",
	"d^a-sil+xx=xx/A:xx+xx+xx/B:10-7_2/C:xx_xx+xx/D:xx+xx_xx/E:3_1!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:xx_xx%xx_xx_xx/H:2_9/I:xx-xx@xx+xx&xx-xx|xx+xx/J:xx_xx/K:2+4-15",
};

const char *const label4[] = {
	"xx^xx-sil+w=a/A:xx+xx+xx/B:xx-xx_xx/C:xx_xx+xx/D:06+xx_xx/E:xx_xx!xx_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:4_1%0_xx_xx/H:xx_xx/I:xx-xx@xx+xx&xx-xx|xx+xx/J:5_17/K:3+12-44",
	"xx^sil-w+a=z/A:0+1+4/B:xx-xx_xx/C:06_xx+xx/D:10+7_3/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"sil^w-a+z=u/A:0+1+4/B:xx-xx_xx/C:06_xx+xx/D:10+7_3/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"w^a-z+u=k/A:1+2+3/B:xx-xx_xx/C:06_xx+xx/D:10+7_3/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"a^z-u+k=a/A:1+2+3/B:xx-xx_xx/C:06_xx+xx/D:10+7_3/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"z^u-k+a=n/A:2+3+2/B:xx-xx_xx/C:06_xx+xx/D:10+7_3/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"u^k-a+n=a/A:2+3+2/B:xx-xx_xx/C:06_xx+xx/D:10+7_3/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"k^a-n+a=k/A:3+4+1/B:06-xx_xx/C:10_7+3/D:03+xx_xx/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"a^n-a+k=i/A:3+4+1/B:06-xx_xx/C:10_7+3/D:03+xx_xx/E:xx_xx!xx_xx-xx/F:4_1#0_xx@1_5|1_17/G:4_1%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"n^a-k+i=n/A:0+1+4/B:10-7_3/C:03_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:4_1#0_xx@2_4|5_13/G:4_4%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"a^k-i+n=o/A:0+1+4/B:10-7_3/C:03_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:4_1#0_xx@2_4|5_13/G:4_4%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"k^i-n+o=o/A:1+2+3/B:10-7_3/C:03_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:4_1#0_xx@2_4|5_13/G:4_4%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"i^n-o+o=n/A:1+2+3/B:10-7_3/C:03_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:4_1#0_xx@2_4|5_13/G:4_4%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"n^o-o+n=i/A:2+3+2/B:10-7_3/C:03_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:4_1#0_xx@2_4|5_13/G:4_4%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"o^o-n+i=r/A:3+4+1/B:03-xx_xx/C:13_xx+xx/D:03+xx_xx/E:4_1!0_xx-1/F:4_1#0_xx@2_4|5_13/G:4_4%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"o^n-i+r=e/A:3+4+1/B:03-xx_xx/C:13_xx+xx/D:03+xx_xx/E:4_1!0_xx-1/F:4_1#0_xx@2_4|5_13/G:4_4%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"n^i-r+e=e/A:-3+1+4/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:4_1!0_xx-1/F:4_4#0_xx@3_3|9_9/G:2_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"i^r-e+e=z/A:-3+1+4/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:4_1!0_xx-1/F:4_4#0_xx@3_3|9_9/G:2_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"r^e-e+z=o/A:-2+2+3/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:4_1!0_xx-1/F:4_4#0_xx@3_3|9_9/G:2_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"e^e-z+o=k/A:-1+3+2/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:4_1!0_xx-1/F:4_4#0_xx@3_3|9_9/G:2_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"e^z-o+k=u/A:-1+3+2/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:4_1!0_xx-1/F:4_4#0_xx@3_3|9_9/G:2_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"z^o-k+u=sh/A:0+4+1/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:4_1!0_xx-1/F:4_4#0_xx@3_3|9_9/G:2_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"o^k-u+sh=I/A:0+4+1/B:13-xx_xx/C:03_xx+xx/D:20+4_1/E:4_1!0_xx-1/F:4_4#0_xx@3_3|9_9/G:2_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"k^u-sh+I=t/A:-1+1+2/B:03-xx_xx/C:20_4+1/D:12+xx_xx/E:4_4!0_xx-1/F:2_2#0_xx@4_2|13_5/G:3_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"u^sh-I+t=e/A:-1+1+2/B:03-xx_xx/C:20_4+1/D:12+xx_xx/E:4_4!0_xx-1/F:2_2#0_xx@4_2|13_5/G:3_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"sh^I-t+e=i/A:0+2+1/B:20-4_1/C:12_xx+xx/D:17+3_1/E:4_4!0_xx-1/F:2_2#0_xx@4_2|13_5/G:3_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"I^t-e+i=t/A:0+2+1/B:20-4_1/C:12_xx+xx/D:17+3_1/E:4_4!0_xx-1/F:2_2#0_xx@4_2|13_5/G:3_2%0_xx_1/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"t^e-i+t=a/A:-1+1+3/B:12-xx_xx/C:17_3+1/D:10+7_2/E:2_2!0_xx-1/F:3_2#0_xx@5_1|15_3/G:5_5%0_xx_0/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"e^i-t+a=g/A:0+2+2/B:17-3_1/C:10_7+2/D:12+xx_xx/E:2_2!0_xx-1/F:3_2#0_xx@5_1|15_3/G:5_5%0_xx_0/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"i^t-a+g=a/A:0+2+2/B:17-3_1/C:10_7+2/D:12+xx_xx/E:2_2!0_xx-1/F:3_2#0_xx@5_1|15_3/G:5_5%0_xx_0/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"t^a-g+a=pau/A:1+3+1/B:10-7_2/C:12_xx+xx/D:03+xx_xx/E:2_2!0_xx-1/F:3_2#0_xx@5_1|15_3/G:5_5%0_xx_0/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"a^g-a+pau=s/A:1+3+1/B:10-7_2/C:12_xx+xx/D:03+xx_xx/E:2_2!0_xx-1/F:3_2#0_xx@5_1|15_3/G:5_5%0_xx_0/H:xx_xx/I:5-17@1+3&1-12|1+44/J:2_7/K:3+12-44",
	"g^a-pau+s=e/A:xx+xx+xx/B:12-xx_xx/C:xx_xx+xx/D:03+xx_xx/E:3_2!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:5_5%0_xx_xx/H:5_17/I:xx-xx@xx+xx&xx-xx|xx+xx/J:2_7/K:3+12-44",
	"a^pau-s+e=e/A:-4+1+5/B:12-xx_xx/C:03_xx+xx/D:13+xx_xx/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"pau^s-e+e=y/A:-4+1+5/B:12-xx_xx/C:03_xx+xx/D:13+xx_xx/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"s^e-e+y=a/A:-3+2+4/B:12-xx_xx/C:03_xx+xx/D:13+xx_xx/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"e^e-y+a=k/A:-2+3+3/B:12-xx_xx/C:03_xx+xx/D:13+xx_xx/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"e^y-a+k=u/A:-2+3+3/B:12-xx_xx/C:03_xx+xx/D:13+xx_xx/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"y^a-k+u=o/A:-1+4+2/B:12-xx_xx/C:03_xx+xx/D:13+xx_xx/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"a^k-u+o=s/A:-1+4+2/B:12-xx_xx/C:03_xx+xx/D:13+xx_xx/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"k^u-o+s=U/A:0+5+1/B:03-xx_xx/C:13_xx+xx/D:20+3_1/E:3_2!0_xx-0/F:5_5#0_xx@1_2|1_7/G:2_2%0_xx_1/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"u^o-s+U=t/A:-1+1+2/B:13-xx_xx/C:20_3+1/D:07+xx_xx/E:5_5!0_xx-1/F:2_2#0_xx@2_1|6_2/G:4_1%0_xx_0/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"o^s-U+t=e/A:-1+1+2/B:13-xx_xx/C:20_3+1/D:07+xx_xx/E:5_5!0_xx-1/F:2_2#0_xx@2_1|6_2/G:4_1%0_xx_0/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"s^U-t+e=pau/A:0+2+1/B:13-xx_xx/C:20_3+1/D:07+xx_xx/E:5_5!0_xx-1/F:2_2#0_xx@2_1|6_2/G:4_1%0_xx_0/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"U^t-e+pau=s/A:0+2+1/B:13-xx_xx/C:20_3+1/D:07+xx_xx/E:5_5!0_xx-1/F:2_2#0_xx@2_1|6_2/G:4_1%0_xx_0/H:5_17/I:2-7@2+2&6-7|18+27/J:5_20/K:3+12-44",
	"t^e-pau+s=a/A:xx+xx+xx/B:20-3_1/C:xx_xx+xx/D:07+xx_xx/E:2_2!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:4_1%0_xx_xx/H:2_7/I:xx-xx@xx+xx&xx-xx|xx+xx/J:5_20/K:3+12-44",
	"e^pau-s+a=r/A:0+1+4/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"pau^s-a+r=a/A:0+1+4/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"s^a-r+a=n/A:1+2+3/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"a^r-a+n=a/A:1+2+3/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"r^a-n+a=r/A:2+3+2/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"a^n-a+r=u/A:2+3+2/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"n^a-r+u=j/A:3+4+1/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"a^r-u+j=o/A:3+4+1/B:20-3_1/C:07_xx+xx/D:02+xx_xx/E:2_2!0_xx-0/F:4_1#0_xx@1_5|1_20/G:8_4%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"r^u-j+o=o/A:-3+1+8/B:07-xx_xx/C:02_xx+xx/D:02+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"u^j-o+o=b/A:-3+1+8/B:07-xx_xx/C:02_xx+xx/D:02+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"j^o-o+b=u/A:-2+2+7/B:07-xx_xx/C:02_xx+xx/D:02+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^o-b+u=k/A:-1+3+6/B:07-xx_xx/C:02_xx+xx/D:02+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^b-u+k=o/A:-1+3+6/B:07-xx_xx/C:02_xx+xx/D:02+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"b^u-k+o=o/A:0+4+5/B:02-xx_xx/C:02_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"u^k-o+o=z/A:0+4+5/B:02-xx_xx/C:02_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"k^o-o+z=o/A:1+5+4/B:02-xx_xx/C:02_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^o-z+o=o/A:2+6+3/B:02-xx_xx/C:02_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^z-o+o=e/A:2+6+3/B:02-xx_xx/C:02_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"z^o-o+e=sh/A:3+7+2/B:02-xx_xx/C:02_xx+xx/D:13+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^o-e+sh=i/A:4+8+1/B:02-xx_xx/C:13_xx+xx/D:03+xx_xx/E:4_1!0_xx-1/F:8_4#0_xx@2_4|5_16/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^e-sh+i=f/A:0+1+3/B:13-xx_xx/C:03_xx+xx/D:20+4_2/E:8_4!0_xx-1/F:3_1#0_xx@3_3|13_8/G:2_2%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"e^sh-i+f=U/A:0+1+3/B:13-xx_xx/C:03_xx+xx/D:20+4_2/E:8_4!0_xx-1/F:3_1#0_xx@3_3|13_8/G:2_2%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"sh^i-f+U=t/A:1+2+2/B:13-xx_xx/C:03_xx+xx/D:20+4_2/E:8_4!0_xx-1/F:3_1#0_xx@3_3|13_8/G:2_2%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"i^f-U+t=o/A:1+2+2/B:13-xx_xx/C:03_xx+xx/D:20+4_2/E:8_4!0_xx-1/F:3_1#0_xx@3_3|13_8/G:2_2%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"f^U-t+o=s/A:2+3+1/B:13-xx_xx/C:03_xx+xx/D:20+4_2/E:8_4!0_xx-1/F:3_1#0_xx@3_3|13_8/G:2_2%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"U^t-o+s=u/A:2+3+1/B:13-xx_xx/C:03_xx+xx/D:20+4_2/E:8_4!0_xx-1/F:3_1#0_xx@3_3|13_8/G:2_2%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"t^o-s+u=r/A:-1+1+2/B:03-xx_xx/C:20_4+2/D:22+xx_xx/E:3_1!0_xx-1/F:2_2#0_xx@4_2|16_5/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^s-u+r=u/A:-1+1+2/B:03-xx_xx/C:20_4+2/D:22+xx_xx/E:3_1!0_xx-1/F:2_2#0_xx@4_2|16_5/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"s^u-r+u=t/A:0+2+1/B:03-xx_xx/C:20_4+2/D:22+xx_xx/E:3_1!0_xx-1/F:2_2#0_xx@4_2|16_5/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"u^r-u+t=o/A:0+2+1/B:03-xx_xx/C:20_4+2/D:22+xx_xx/E:3_1!0_xx-1/F:2_2#0_xx@4_2|16_5/G:3_1%0_xx_1/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"r^u-t+o=k/A:0+1+3/B:20-4_2/C:22_xx+xx/D:10+7_2/E:2_2!0_xx-1/F:3_1#0_xx@5_1|18_3/G:xx_xx%xx_xx_xx/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"u^t-o+k=i/A:0+1+3/B:20-4_2/C:22_xx+xx/D:10+7_2/E:2_2!0_xx-1/F:3_1#0_xx@5_1|18_3/G:xx_xx%xx_xx_xx/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"t^o-k+i=d/A:1+2+2/B:20-4_2/C:22_xx+xx/D:10+7_2/E:2_2!0_xx-1/F:3_1#0_xx@5_1|18_3/G:xx_xx%xx_xx_xx/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"o^k-i+d=a/A:1+2+2/B:20-4_2/C:22_xx+xx/D:10+7_2/E:2_2!0_xx-1/F:3_1#0_xx@5_1|18_3/G:xx_xx%xx_xx_xx/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"k^i-d+a=sil/A:2+3+1/B:22-xx_xx/C:10_7+2/D:xx+xx_xx/E:2_2!0_xx-1/F:3_1#0_xx@5_1|18_3/G:xx_xx%xx_xx_xx/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"i^d-a+sil=xx/A:2+3+1/B:22-xx_xx/C:10_7+2/D:xx+xx_xx/E:2_2!0_xx-1/F:3_1#0_xx@5_1|18_3/G:xx_xx%xx_xx_xx/H:2_7/I:5-20@3+1&8-5|25+20/J:xx_xx/K:3+12-44",
	"d^a-sil+xx=xx/A:xx+xx+xx/B:10-7_2/C:xx_xx+xx/D:xx+xx_xx/E:3_1!0_xx-xx/F:xx_xx#xx_xx@xx_xx|xx_xx/G:xx_xx%xx_xx_xx/H:5_20/I:xx-xx@xx+xx&xx-xx|xx+xx/J:xx_xx/K:3+12-44",
};

struct synse_list_t {
	const char *const *labels;
	int count;
	const char *voice;
};

synse_list_t synse_list[] = {
	{label1, sizeof(label1) / sizeof(label1[0]), "/" MOUNT_NAME "/mei/mei_normal.htsvoice"},
	{label2, sizeof(label2) / sizeof(label2[0]), "/" MOUNT_NAME "/mei/mei_sad.htsvoice"},
	{label3, sizeof(label3) / sizeof(label3[0]), "/" MOUNT_NAME "/nitech_jp_atr503_m001.htsvoice"},
	{label4, sizeof(label4) / sizeof(label4[0]), "/" MOUNT_NAME "/mei/mei_happy.htsvoice"},
};

/* hts_engine API */
HTS_Engine engine;

extern "C" void wmem_init();

int main()
{
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

	for (int i = 0; i < sizeof(synse_list) / sizeof(synse_list[0]); i++){
		const char *const *labels = synse_list[i].labels;
		int count = synse_list[i].count;
		const char *fn_voice = synse_list[i].voice;

		wmem_init();

		/* initialize hts_engine API */
		HTS_Engine_initialize(&engine);

		if (HTS_Engine_load(&engine, &fn_voice, 1) != TRUE) {
			HTS_Engine_clear(&engine);
			return -1;
		}

		if (strcmp(HTS_Engine_get_fullcontext_label_format(&engine), "HTS_TTS_JPN") != 0) {
			HTS_Engine_clear(&engine);
			return -1;
		}

		//HTS_Engine_set_speed(&engine, 100);

		//HTS_Engine_set_sampling_frequency(&engine, 48000);

		HTS_Engine_set_audio_buff_size(&engine, AUDIO_BUFF_SIZE / sizeof(short));

		/* synthesize */
		if (HTS_Engine_synthesize_from_strings(&engine, labels, count) != TRUE) {
			fprintf(stderr, "Error: waveform cannot be synthesized.\n");
			HTS_Engine_clear(&engine);
			return -1;
		}

		/* free memory */
		HTS_Engine_clear(&engine);
	}

	return 0;
}
