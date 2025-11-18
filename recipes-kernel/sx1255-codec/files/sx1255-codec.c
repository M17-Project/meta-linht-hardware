#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/module.h>

MODULE_DESCRIPTION("SX1255 I2S Codec Driver - Experimental");
MODULE_AUTHOR("Andreas Schmidberger, OE3ANC");
MODULE_LICENSE("GPL");


// SX1255 supported sample rates (from datasheet)
static unsigned int sx1255_rates[] = {
    8000, 16000, 32000, 48000, 96000, 192000,
    25000, 50000, 75000, 150000, 300000, 500000, 600000
};

static struct snd_pcm_hw_constraint_list sx1255_rate_constraints = {
    .count = ARRAY_SIZE(sx1255_rates),
    .list = sx1255_rates,
    .mask = 0,
};


static int sx1255_codec_startup(struct snd_pcm_substream *substream,
                                struct snd_soc_dai *dai)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    int ret;
    
    dev_info(dai->dev, "=== SX1255 STARTUP ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
    
    // Force stereo (I/Q channels)
    ret = snd_pcm_hw_constraint_minmax(runtime, SNDRV_PCM_HW_PARAM_CHANNELS, 2, 2);
    if (ret < 0) {
        dev_err(dai->dev, "Failed to set channel constraint: %d\n", ret);
        return ret;
    }
    
    // Set custom rate list including 500 kHz
    ret = snd_pcm_hw_constraint_list(runtime, 0, 
                                     SNDRV_PCM_HW_PARAM_RATE,
                                     &sx1255_rate_constraints);
    if (ret < 0) {
        dev_err(dai->dev, "Failed to set rate constraints: %d\n", ret);
        return ret;
    }
    
    // Force S32_LE format only (matches 32-bit TDM slots)
    runtime->hw.formats = SNDRV_PCM_FMTBIT_S32_LE;
    
    // Ensure integer number of periods
    ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
    if (ret < 0) {
        dev_err(dai->dev, "Failed to set period constraint: %d\n", ret);
        return ret;
    }
    
    dev_info(dai->dev, "Constraints applied: S32_LE, 2 channels (stereo I/Q), rates up to 600 kHz\n");
    
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
    unsigned int bclk;
    
    dev_info(dai->dev, "=== SX1255 HW_PARAMS ===\n");
    dev_info(dai->dev, "Stream: %s\n",
             substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE");
    dev_info(dai->dev, "Sample rate: %u Hz\n", rate);
    dev_info(dai->dev, "Channels: %u (I/Q)\n", channels);
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
    
    // Calculate BCLK and LRCLK
    bclk = rate * channels * physical_width;
    dev_info(dai->dev, "Expected BCLK: %u Hz (%u.%03u MHz)\n",
             bclk, bclk / 1000000, (bclk % 1000000) / 1000);
    dev_info(dai->dev, "Expected LRCLK (Frame Sync): %u Hz\n", rate);
    
    // Validate parameters
    if (channels != 2) {
        dev_err(dai->dev, "Only stereo (2 channels) supported, got %u\n", channels);
        return -EINVAL;
    }
    
    if (format != SNDRV_PCM_FORMAT_S32_LE) {
        dev_err(dai->dev, "Only S32_LE format supported, got %s\n",
                snd_pcm_format_name(format));
        return -EINVAL;
    }
    
    dev_info(dai->dev, "======================\n");
    
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
    
    // Verify we're being set as clock master (SX1255 generates BCLK/LRCLK)
    if ((fmt & SND_SOC_DAIFMT_CLOCK_PROVIDER_MASK) != SND_SOC_DAIFMT_CBP_CFP) {
        dev_warn(dai->dev, "SX1255 should be clock master (CBP_CFP), got 0x%x\n",
                 fmt & SND_SOC_DAIFMT_CLOCK_PROVIDER_MASK);
    }
    
    // Verify I2S format
    if ((fmt & SND_SOC_DAIFMT_FORMAT_MASK) != SND_SOC_DAIFMT_I2S) {
        dev_warn(dai->dev, "Expected I2S format, got 0x%x\n",
                 fmt & SND_SOC_DAIFMT_FORMAT_MASK);
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
    .set_fmt = sx1255_codec_set_fmt,
};


static struct snd_soc_dai_driver sx1255_codec_dai = {
    .name = "sx1255-i2s",
    .id = 0,
    .ops = &sx1255_dai_ops,
    
    .playback = {
        .stream_name = "Playback",
        .channels_min = 2,
        .channels_max = 2,
        .rates = SNDRV_PCM_RATE_KNOT,  // Custom rates via constraints
        .formats = SNDRV_PCM_FMTBIT_S32_LE,
    },
    
    .capture = {
        .stream_name = "Capture",
        .channels_min = 2,
        .channels_max = 2,
        .rates = SNDRV_PCM_RATE_KNOT,  // Custom rates via constraints
        .formats = SNDRV_PCM_FMTBIT_S32_LE,
    },
    
    .symmetric_rate = 1,
    .symmetric_channels = 1,
    .symmetric_sample_bits = 1,
};


static const struct snd_soc_component_driver sx1255_component_driver = {
    .name = "sx1255-codec",
};


static int sx1255_codec_probe(struct platform_device *pdev)
{
    int ret;
    
    dev_info(&pdev->dev, "========================================\n");
    dev_info(&pdev->dev, "SX1255 I/Q Codec Driver\n");
    dev_info(&pdev->dev, "Configuration:\n");
    dev_info(&pdev->dev, "  Format: S32_LE (32-bit signed little endian)\n");
    dev_info(&pdev->dev, "  Channels: 2 (stereo I/Q)\n");
    dev_info(&pdev->dev, "  Sample rates: 8-600 kHz (including 500 kHz)\n");
    dev_info(&pdev->dev, "  Mode: Full duplex bidirectional\n");
    dev_info(&pdev->dev, "  Clock: SX1255 is master (provides BCLK/LRCLK)\n");
    dev_info(&pdev->dev, "========================================\n");
    
    ret = devm_snd_soc_register_component(&pdev->dev,
                                          &sx1255_component_driver,
                                          &sx1255_codec_dai, 1);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to register component: %d\n", ret);
        return ret;
    }
    
    dev_info(&pdev->dev, "SX1255 codec registered successfully\n");
    return 0;
}


static int sx1255_codec_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "SX1255 codec driver removing\n");
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
