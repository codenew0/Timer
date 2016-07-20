#include <stdio.h>  
#include <stdlib.h>  
#include <alsa/asoundlib.h>  

void *play_sound (void *filename)
{  
	const char *str = (char*)filename;
	int ret;  
	unsigned int val;  
	int dir = 0;  
	char *buffer;  
	int size;  
	snd_pcm_uframes_t frames;  
	snd_pcm_uframes_t periodsize;  
	snd_pcm_t *playback_handle;		//PCM device handler  
	snd_pcm_hw_params_t *hw_params;	//Hardware information and PCM stream configuration

	FILE *fp = fopen(str, "rb");  
	fseek(fp, 100, SEEK_SET);  

	//1. Open PCM
	snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);  

	//2. Malloc PCM struct 
	snd_pcm_hw_params_malloc(&hw_params);  

	//3. Initialize hw_params 
	snd_pcm_hw_params_any(playback_handle, hw_params);  

	//4. Initialize access
	snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);  

	//5. Initialize sampling format
	snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);  

	//6. Set sampling rate
	val = 22050;  
	snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &val, &dir);  

	//7. Set channels
	snd_pcm_hw_params_set_channels(playback_handle, hw_params, 1);  

	/* Set period size to 32 frames. */  
	frames = 32;  
	periodsize = frames * 2;  
	snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &periodsize);  
	periodsize /= 2;  
	snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &periodsize, 0);  

	//8. Set hw_params  
	snd_pcm_hw_params(playback_handle, hw_params);  

	/* Use a buffer large enough to hold one period */  
	snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);  

	size = frames * 2; /* 2 bytes/sample, 2 channels */  
	buffer = (char *) malloc(size);  

	while (1)   
	{  
		ret = fread(buffer, 1, size, fp);  
		if(ret == 0)   
		{  
			break;  
		}   
		//9. Wirte sound data to PCM
		while((ret = snd_pcm_writei(playback_handle, buffer, frames)) < 0)  
		{  
			usleep(2000);  
			if (ret == -EPIPE)  
			{  
				/* EPIPE means underrun */  
				fprintf(stderr, "underrun occurred\n");  
				//Prepare the device
				snd_pcm_prepare(playback_handle);  
			}   
			else if (ret < 0)   
			{  
				fprintf(stderr, "error from writei: %s\n", snd_strerror(ret));  
			}    
		}  

	}         
	//10. Close PCM handler
	snd_pcm_close(playback_handle);

	return NULL;
}  
