/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __PRJ_CONFIG_XPLAYER

#include "cmd_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cdx_log.h>
#include "xplayer.h"
#include "fs/fatfs/ff.h"
#include "common/framework/fs_ctrl.h"
#include "xrecord.h"
#include "CaptureControl.h"
#include "driver/chip/hal_codec.h"
#include "audio/manager/audio_manager.h"

extern SoundCtrl* SoundDeviceCreate();

typedef struct DemoPlayerContext
{
    XPlayer*        mAwPlayer;
    int             mSeekable;
    int             mError;
}DemoPlayerContext;

static uint8_t cedarx_inited = 0;
static int fs_init(void)
{
    if (cedarx_inited++ == 0) {
        if (fs_ctrl_mount(FS_MNT_DEV_TYPE_SDCARD, 0) != 0) {
            printf("mount fail\n");
            return -1;
        } else {
            printf("mount success\n");
        }
    }
    return 0;
}

static void fs_exit(void)
{
    if (--cedarx_inited == 0) {
        if (fs_ctrl_unmount(FS_MNT_DEV_TYPE_SDCARD, 0) != 0) {
            printf("unmount fail\n");
        }
    }
}

static void set_source(DemoPlayerContext *demoPlayer, char* pUrl)
{
    demoPlayer->mSeekable = 1;

    //* set url to the AwPlayer.
    if(XPlayerSetDataSourceUrl(demoPlayer->mAwPlayer,
                 (const char*)pUrl, NULL, NULL) != 0)
    {
        printf("error:\n");
        printf("    AwPlayer::setDataSource() return fail.\n");
        return;
    }
    printf("setDataSource end\n");

    if ((!strncmp(pUrl, "http://", 7)) || (!strncmp(pUrl, "https://", 8))) {
        if(XPlayerPrepareAsync(demoPlayer->mAwPlayer) != 0)
        {
            printf("error:\n");
            printf("    AwPlayer::prepareAsync() return fail.\n");
            return;
        }
    }
    printf("preparing...\n");
}

static void play(DemoPlayerContext *demoPlayer)
{
    if(XPlayerStart(demoPlayer->mAwPlayer) != 0)
    {
        printf("error:\n");
        printf("    AwPlayer::start() return fail.\n");
        return;
    }
    printf("playing.\n");
}

static void pause(DemoPlayerContext *demoPlayer)
{
    if(XPlayerPause(demoPlayer->mAwPlayer) != 0)
    {
        printf("error:\n");
        printf("    AwPlayer::pause() return fail.\n");
        return;
    }
    printf("paused.\n");
}

static void stop(DemoPlayerContext *demoPlayer)
{
    if(XPlayerReset(demoPlayer->mAwPlayer) != 0)
    {
        printf("error:\n");
        printf("    AwPlayer::reset() return fail.\n");
        return;
    }
    printf("stopped.\n");
}

//* a callback for awplayer.
static int CallbackForAwPlayer(void* pUserData, int msg, int ext1, void* param)
{
    DemoPlayerContext* pDemoPlayer = (DemoPlayerContext*)pUserData;

    switch(msg)
    {
        case AWPLAYER_MEDIA_INFO:
        {
            switch(ext1)
            {
                case AW_MEDIA_INFO_NOT_SEEKABLE:
                {
                    pDemoPlayer->mSeekable = 0;
                    printf("info: media source is unseekable.\n");
                    break;
                }
                case AW_MEDIA_INFO_RENDERING_START:
                {
                    printf("info: start to show pictures.\n");
                    break;
                }
            }
            break;
        }

        case AWPLAYER_MEDIA_ERROR:
        {
            printf("error: open media source fail.\n");
            pDemoPlayer->mError = 1;
            loge(" error : how to deal with it");
            break;
        }

        case AWPLAYER_MEDIA_PREPARED:
        {
            logd("info : preared");
            printf("info: prepare ok.\n");
            break;
        }

        case AWPLAYER_MEDIA_PLAYBACK_COMPLETE:
        {
            printf("info: play complete.\n");
            break;
        }

        case AWPLAYER_MEDIA_SEEK_COMPLETE:
        {
            printf("info: seek ok.\n");
            break;
        }

        case AWPLAYER_MEDIA_CHANGE_URL:
        {
            printf("suggest to play:%s\n", (char *)param);
            break;
        }

        default:
        {
            //printf("warning: unknown callback from AwPlayer.\n");
            break;
        }
    }

    return 0;
}

static DemoPlayerContext *demoPlayer;

static enum cmd_status cmd_cedarx_create_exec(char *cmd)
{
    demoPlayer = malloc(sizeof(*demoPlayer));

    fs_init();

    //* create a player.
    memset(demoPlayer, 0, sizeof(DemoPlayerContext));

    demoPlayer->mAwPlayer = XPlayerCreate();
    if(demoPlayer->mAwPlayer == NULL)
    {
        printf("can not create AwPlayer, quit.\n");
        return -1;
    } else {
        printf("create AwPlayer success.\n");
    }

    //* set callback to player.
    XPlayerSetNotifyCallback(demoPlayer->mAwPlayer, CallbackForAwPlayer, (void*)demoPlayer);

    //* check if the player work.
    if(XPlayerInitCheck(demoPlayer->mAwPlayer) != 0)
    {
        printf("initCheck of the player fail, quit.\n");
        XPlayerDestroy(demoPlayer->mAwPlayer);
        demoPlayer->mAwPlayer = NULL;
        return -1;
    } else
        printf("AwPlayer check success.\n");

    SoundCtrl* sound = SoundDeviceCreate();

    XPlayerSetAudioSink(demoPlayer->mAwPlayer, (void*)sound);

    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_destroy_exec(char *cmd)
{
    if(demoPlayer->mAwPlayer != NULL)
    {
        XPlayerDestroy(demoPlayer->mAwPlayer);
        demoPlayer->mAwPlayer = NULL;
    }
    printf("destroy AwPlayer.\n");

    fs_exit();

    free(demoPlayer);

    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_play_exec(char *cmd)
{
    play(demoPlayer);
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_stop_exec(char *cmd)
{
    stop(demoPlayer);
    msleep(5);
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_pause_exec(char *cmd)
{
    pause(demoPlayer);
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_seturl_exec(char *cmd)
{
    set_source(demoPlayer, cmd);
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_getpos_exec(char *cmd)
{
    int msec;
    XPlayerGetCurrentPosition(demoPlayer->mAwPlayer, &msec);
    cmd_write_respond(CMD_STATUS_OK, "played time: %d ms", msec);
    return CMD_STATUS_ACKED;
}

static enum cmd_status cmd_cedarx_seek_exec(char *cmd)
{
    int nSeekTimeMs = atoi(cmd);

    XPlayerSeekTo(demoPlayer->mAwPlayer, nSeekTimeMs);
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_setvol_exec(char *cmd)
{
    int vol = atoi(cmd);
    if (vol > 31)
        vol = 31;

    aud_mgr_handler(AUDIO_DEVICE_MANAGER_VOLUME, AUDIO_OUT_DEV_SPEAKER, vol);

    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_playdic_exec(char *cmd)
{
    return CMD_STATUS_OK;
}

static XRecord *xrecord;

static enum cmd_status cmd_cedarx_rec_exec(char *cmd)
{
    XRECODER_AUDIO_ENCODE_TYPE type;
    if (strstr(cmd, ".amr"))
        type = XRECODER_AUDIO_ENCODE_AMR_TYPE;
    else if (strstr(cmd, ".pcm"))
        type = XRECODER_AUDIO_ENCODE_PCM_TYPE;
    else {
        printf("do not support this encode type\n");
        return CMD_STATUS_INVALID_ARG;
    }

    fs_init();

    xrecord = XRecordCreate();
    if (xrecord == NULL) {
        printf("xrecord create failed\n");
        return CMD_STATUS_FAIL;
    }

    CaptureCtrl* cap = CaptureDeviceCreate();
    if (!cap) {
        printf("cap device create failed\n");
        XRecordDestroy(xrecord);
        xrecord = NULL;
        return CMD_STATUS_FAIL;
    }
    XRecordSetAudioCap(xrecord, cap);

    XRecordConfig audioConfig;
    audioConfig.nChan = 1;
    audioConfig.nSamplerate = 8000;
    audioConfig.nSamplerBits = 16;
    audioConfig.nBitrate = 12200;

    XRecordSetDataDstUrl(xrecord, cmd, NULL, NULL);
    XRecordSetAudioEncodeType(xrecord, type, &audioConfig);

    XRecordPrepare(xrecord);
    XRecordStart(xrecord);
    printf("record start\n");

    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_end_exec(char *cmd)
{
    XRecordStop(xrecord);
    XRecordDestroy(xrecord);
    printf("record destroy\n");

    fs_exit();

    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_log_exec(char *cmd)
{
    int level = atoi(cmd);
    if (level > 6)
        level = 6;

    void log_set_level(unsigned level);
    log_set_level(level);
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_version_exec(char *cmd)
{
    XPlayerShowVersion();
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_showbuf_exec(char *cmd)
{
    XPlayerShowBuffer();
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_setbuf_exec(char *cmd)
{
    int argc;
    char *argv[4];
    XPlayerBufferConfig cfg;
    int maxAudioFrameSize;
    argc = cmd_parse_argv(cmd, argv, 4);
    if (argc < 4) {
        CMD_ERR("invalid cedarx set buf cmd, argc %d\n", argc);
        goto exit;
    }

    cfg.maxStreamBufferSize = cmd_atoi(argv[0]);
    cfg.maxBitStreamBufferSize = cmd_atoi(argv[1]);
    cfg.maxPcmBufferSize = cmd_atoi(argv[2]);
    cfg.maxStreamFrameCount = 0;
    cfg.maxBitStreamFrameCount = 0;
    maxAudioFrameSize = cmd_atoi(argv[3]);

    XPlayerSetBuffer(demoPlayer->mAwPlayer, &cfg);
    XPlayerSetPcmFrameSize(demoPlayer->mAwPlayer, maxAudioFrameSize);
    XPlayerShowBuffer();
exit:
    return CMD_STATUS_OK;
}

extern void CdxBufAutoStatStart(void);
static enum cmd_status cmd_cedarx_bufinfo_exec(char *cmd)
{
    CdxBufAutoStatStart();
    return CMD_STATUS_OK;
}

static enum cmd_status cmd_cedarx_aacsbr_exec(char *cmd)
{
    int use_sbr = atoi(cmd);

    if (demoPlayer->mAwPlayer)
    {
        XPlayerSetAacSbr(demoPlayer->mAwPlayer, use_sbr);
        printf("set aac sbr success\n");
    }
    else
    {
        printf("set aac sbr fail\n");
    }
    return CMD_STATUS_OK;
}

int cedarx_loop_test(char *cmd, char *loop_url);
static enum cmd_status cmd_cedarx_loop_exec(char *cmd)
{
    int argc;
    char *argv[2];
    int ret = 0;

    /* check args */
    argc = cmd_parse_argv(cmd, argv, 2);
    if (argc != 2) {
        CMD_ERR("invalid cedarx loop cmd, argc %d\n", argc);
        goto err;
    }

    fs_init();

    ret = cedarx_loop_test(argv[0], argv[1]);
    if (ret) {
        CMD_ERR("cedarx loop test error.\n");
        goto err;
    }

    return CMD_STATUS_OK;

err:

    fs_exit();

    return CMD_STATUS_OK;
}

int cedarx_net_test(char *net_url);
static enum cmd_status cmd_cedarx_nettest_exec(char *cmd)
{
    int argc;
    char *argv[1];

    argc = cmd_parse_argv(cmd, argv, 1);

    if (argc != 1) {
        CMD_ERR("invalid cedarx nettest cmd, argc %d\n", argc);
        goto err;
    }

    cedarx_net_test(argv[0]);

err:
    return CMD_STATUS_OK;
}

/*
 * brief cedarx Test Command
 *          cedarx play
 *          cedarx stop
 *          cedarx pause
 *          cedarx seturl file://music/01.mp3
 *          cedarx getpos
 *          cedarx seek 6000
 *          cedarx setvol 8
 *          cedarx playdic file://music
 *          cedarx rec file://record/wechat.amr
 *          cedarx end
 */
static const struct cmd_data g_cedarx_cmds[] = {
    { "create",     cmd_cedarx_create_exec      },
    { "destroy",    cmd_cedarx_destroy_exec     },
    { "play",       cmd_cedarx_play_exec        },
    { "stop",       cmd_cedarx_stop_exec        },
    { "pause",      cmd_cedarx_pause_exec       },
    { "seturl",     cmd_cedarx_seturl_exec      },
    { "getpos",     cmd_cedarx_getpos_exec      },
    { "seek",       cmd_cedarx_seek_exec        },
    { "setvol",     cmd_cedarx_setvol_exec      },
    { "playdic",    cmd_cedarx_playdic_exec     },
    { "log",        cmd_cedarx_log_exec         },
    { "version",    cmd_cedarx_version_exec     },
    { "showbuf",    cmd_cedarx_showbuf_exec     },
    { "setbuf",     cmd_cedarx_setbuf_exec      },
    { "bufinfo",    cmd_cedarx_bufinfo_exec     },
    { "aacsbr",     cmd_cedarx_aacsbr_exec      },
    { "rec",        cmd_cedarx_rec_exec         },
    { "end",        cmd_cedarx_end_exec         },
    { "loop",       cmd_cedarx_loop_exec        },
    { "nettest",    cmd_cedarx_nettest_exec     },
};

enum cmd_status cmd_cedarx_exec(char *cmd)
{
    return cmd_exec(cmd, g_cedarx_cmds, cmd_nitems(g_cedarx_cmds));
}

#endif /* __PRJ_CONFIG_XPLAYER */
