

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

#include "defs.h"
#include "pcm.h"
#include "rc.h"

/* FIXME - all this code is VERY basic, improve it! */


struct pcm pcm;

static char *dsp_device;
static int stereo = 0;
static int samplerate = 11025;
static int sound = 1;

snd_pcm_t *playback_handle;
#define DSP_DEVICE "hw:0,0"

rcvar_t pcm_exports[] =
{
	RCV_BOOL("sound", &sound),
	RCV_INT("stereo", &stereo),
	RCV_INT("samplerate", &samplerate),
	RCV_STRING("alsa_device", &dsp_device),
	RCV_END
};


void pcm_init()
{
	int n, m, err;
	snd_pcm_hw_params_t *hw_params;

	if (!sound)
	{
		pcm.hz = 11025;
		pcm.len = 4096;
		pcm.buf = malloc(pcm.len);
		pcm.pos = 0;
		playback_handle = NULL;
		return;
	}

	if (!dsp_device) dsp_device = strdup(DSP_DEVICE);

	if ((err = snd_pcm_open (&playback_handle, dsp_device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 dsp_device,
			 snd_strerror (err));
		exit (1);
	}
	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_U8)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, samplerate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, stereo ? 2 : 1)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	pcm.stereo = stereo;
	pcm.hz = samplerate;
	pcm.len = 4096;
	pcm.buf = malloc(pcm.len);
}

void pcm_close()
{
	if (pcm.buf) free(pcm.buf);
	memset(&pcm, 0, sizeof pcm);
	snd_pcm_close (playback_handle);
	playback_handle = NULL;
}

int pcm_submit()
{
	if (playback_handle == NULL)
	{
		pcm.pos = 0;
		return 0;
	}
	if (pcm.buf) snd_pcm_writei(playback_handle, pcm.buf, pcm.pos);
	pcm.pos = 0;
	return 1;
}






