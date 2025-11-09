#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/module.h>

MODULE_DESCRIPTION("SX1255 I2S Codec Driver - Experimental");
MODULE_AUTHOR("Andreas Schmidberger, OE3ANC");
MODULE_LICENSE("GPL");

static int sx1255_codec_startup(struct snd_pcm_substream *substream,
                                struct snd_soc_dai *dai)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    
    dev_info(dai->dev, "=== SX1255 STARTUP ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
    dev_info(dai->dev, "Runtime rate_min: %u, rate_max: %u\n",
             runtime->hw.rate_min, runtime->hw.rate_max);
    dev_info(dai->dev, "Runtime channels_min: %u, channels_max: %u\n",
             runtime->hw.channels_min, runtime->hw.channels_max);
    dev_info(dai->dev, "Runtime formats: 0x%llx\n", runtime->hw.formats);
    
    return 0;
}

static void sx1255_codec_shutdown(struct snd_pcm_substream *substream,
                                  struct snd_soc_dai *dai)
{
    dev_info(dai->dev, "=== SX1255 SHUTDOWN ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
}

static int sx1255_codec_hw_params(struct snd_pcm_substream *substream,
                                  struct snd_pcm_hw_params *params,
                                  struct snd_soc_dai *dai)
{
    unsigned int rate = params_rate(params);
    unsigned int channels = params_channels(params);
    snd_pcm_format_t format = params_format(params);
    unsigned int buffer_size = params_buffer_size(params);
    unsigned int period_size = params_period_size(params);
    unsigned int periods = params_periods(params);
    int width = params_width(params);
    int physical_width = params_physical_width(params);
    ssize_t buffer_bytes = frames_to_bytes(substream->runtime, buffer_size);
    ssize_t period_bytes = frames_to_bytes(substream->runtime, period_size);
    
    dev_info(dai->dev, "=== SX1255 HW_PARAMS ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
    dev_info(dai->dev, "Sample rate: %u Hz\n", rate);
    dev_info(dai->dev, "Channels: %u\n", channels);
    dev_info(dai->dev, "Format: %d (%s)\n", format, snd_pcm_format_name(format));
    dev_info(dai->dev, "Sample width: %d bits\n", width);
    dev_info(dai->dev, "Physical width: %d bits\n", physical_width);
    dev_info(dai->dev, "Buffer size: %u frames (%zd bytes)\n",
             buffer_size, buffer_bytes);
    dev_info(dai->dev, "Period size: %u frames (%zd bytes)\n",
             period_size, period_bytes);
    dev_info(dai->dev, "Periods: %u\n", periods);
    dev_info(dai->dev, "Buffer time: ~%u ms\n", (buffer_size * 1000) / rate);
    dev_info(dai->dev, "Period time: ~%u ms\n", (period_size * 1000) / rate);
    
    // Calculate BCLK and LRCLK for reference - use integer math only
    unsigned int bclk = rate * channels * physical_width;
    dev_info(dai->dev, "Expected BCLK: %u Hz (%u.%03u MHz)\n",
             bclk, bclk / 1000000, (bclk % 1000000) / 1000);
    dev_info(dai->dev, "Expected LRCLK (Frame Sync): %u Hz\n", rate);
    
    dev_info(dai->dev, "======================\n");
    
    // Accept everything - no restrictions
    return 0;
}

static int sx1255_codec_hw_free(struct snd_pcm_substream *substream,
                                struct snd_soc_dai *dai)
{
    dev_info(dai->dev, "=== SX1255 HW_FREE ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
    return 0;
}

static int sx1255_codec_prepare(struct snd_pcm_substream *substream,
                                struct snd_soc_dai *dai)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    
    dev_info(dai->dev, "=== SX1255 PREPARE ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
    dev_info(dai->dev, "Rate: %u Hz, Channels: %u, Format: %s\n",
             runtime->rate, runtime->channels, snd_pcm_format_name(runtime->format));
    dev_info(dai->dev, "Period size: %lu, Buffer size: %lu\n",
             runtime->period_size, runtime->buffer_size);
    return 0;
}

static int sx1255_codec_trigger(struct snd_pcm_substream *substream,
                                int cmd, struct snd_soc_dai *dai)
{
    dev_info(dai->dev, "=== SX1255 TRIGGER ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
    
    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
        dev_info(dai->dev, "Command: START\n");
        break;
    case SNDRV_PCM_TRIGGER_STOP:
        dev_info(dai->dev, "Command: STOP\n");
        break;
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        dev_info(dai->dev, "Command: PAUSE\n");
        break;
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        dev_info(dai->dev, "Command: RESUME\n");
        break;
    default:
        dev_info(dai->dev, "Command: UNKNOWN (%d)\n", cmd);
        break;
    }
    
    return 0;
}

static int sx1255_codec_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    dev_info(dai->dev, "=== SX1255 SET_FMT ===\n");
    dev_info(dai->dev, "Format: 0x%x\n", fmt);
    
    // Verify we're being set as clock master
    if ((fmt & SND_SOC_DAIFMT_CLOCK_PROVIDER_MASK) != SND_SOC_DAIFMT_CBP_CFP) {
        dev_warn(dai->dev, "SX1255 must be clock master (CBP_CFP)\n");
    }
    
    return 0;
}

static const struct snd_soc_dai_ops sx1255_dai_ops = {
    .startup = sx1255_codec_startup,
    .shutdown = sx1255_codec_shutdown,
    .hw_params = sx1255_codec_hw_params,
    .hw_free = sx1255_codec_hw_free,
    .prepare = sx1255_codec_prepare,
    .trigger = sx1255_codec_trigger,
    .set_fmt = sx1255_codec_set_fmt,  // Add this
};


static struct snd_soc_dai_driver sx1255_codec_dai = {
    .name = "sx1255-i2s",
    .id = 0,
    .ops = &sx1255_dai_ops,
    
    .playback = {
        .stream_name = "Playback",
        .channels_min = 1,
        .channels_max = 8,
        .rates = SNDRV_PCM_RATE_CONTINUOUS,
        .rate_min = 8000,
        .rate_max = 1000000,
        .formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE,
    },
    
    .capture = {
        .stream_name = "Capture",
        .channels_min = 1,
        .channels_max = 8,
        .rates = SNDRV_PCM_RATE_CONTINUOUS,
        .rate_min = 8000,
        .rate_max = 1000000,
        .formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE,
    },
    
    .symmetric_rate = 1,  // Add this for master mode
};

static const struct snd_soc_component_driver sx1255_component_driver = {
    .name = "sx1255-codec",
};

static int sx1255_codec_probe(struct platform_device *pdev)
{
    int ret;
    
    dev_info(&pdev->dev, "========================================\n");
    dev_info(&pdev->dev, "SX1255 EXPERIMENTAL codec driver probing\n");
    dev_info(&pdev->dev, "Mode: UNCONSTRAINED (accepts all parameters)\n");
    dev_info(&pdev->dev, "Rate range: 8 kHz - 1 MHz (continuous)\n");
    dev_info(&pdev->dev, "Channels: 1-8\n");
    dev_info(&pdev->dev, "Formats: S8, S16_LE, S24_LE, S24_3LE, S32_LE\n");
    dev_info(&pdev->dev, "========================================\n");
    
    ret = devm_snd_soc_register_component(&pdev->dev,
                                          &sx1255_component_driver,
                                          &sx1255_codec_dai, 1);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to register component: %d\n", ret);
        return ret;
    }
    
    dev_info(&pdev->dev, "SX1255 experimental codec registered successfully\n");
    return 0;
}

static int sx1255_codec_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "SX1255 experimental codec driver removing\n");
    return 0;
}

static const struct of_device_id sx1255_codec_of_match[] = {
    { .compatible = "semtech,sx1255-codec" },
    { }
};

MODULE_DEVICE_TABLE(of, sx1255_codec_of_match);

static struct platform_driver sx1255_codec_platform_driver = {
    .probe = sx1255_codec_probe,
    .remove = sx1255_codec_remove,
    .driver = {
        .name = "sx1255-codec",
        .of_match_table = sx1255_codec_of_match,
    },
};

module_platform_driver(sx1255_codec_platform_driver);
