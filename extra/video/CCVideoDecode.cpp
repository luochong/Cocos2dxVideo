/**
 * 视频解码类
 */
#include "CCVideoDecode.h"
#include "CCVideoTextureCache.h"
NS_CC_BEGIN

CCVideoPic::CCVideoPic()
{
    m_pPicture = NULL;
}

CCVideoPic::~CCVideoPic()
{  
    if(m_pPicture)
    {
        avpicture_free(m_pPicture);  
        delete m_pPicture; 
    }
}

bool CCVideoPic::init(const char *path, int frame, int videoStream, AVCodecContext * pCodecCtx, SwsContext *pSwsCtx, AVFrame *pFrame)
{
    m_pPicture = new AVPicture; 
    avpicture_alloc(m_pPicture, PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);

    sws_scale (pSwsCtx, pFrame->data, pFrame->linesize,  
               0, pCodecCtx->height,  
               m_pPicture->data, m_pPicture->linesize); 
    m_path = path;
    m_frame = frame;
    m_videoStream = videoStream;
    m_width = pCodecCtx->width;
    m_height = pCodecCtx->height;
    return true;
}

CCVideoDecode::CCVideoDecode()
{
	m_filepath = NULL;
	m_pFormatCtx = NULL;
	m_videoStream = -1;
    m_pSwsCtx = NULL;
    m_pCodecCtx = NULL;
    m_frameCount = 0;
}

bool CCVideoDecode::init(const char *path)
{
    m_filepath = path;
   
    // Register all formats and codecs  
    av_register_all();  
      
    /* 1、构建avformat */
    if(avformat_open_input(&m_pFormatCtx, path, NULL, NULL) != 0) {  
         CCLOG("avformat_open_input false");
         return false;  
    }
    
    /* 2、获取流信息 */
    if(avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {  
        CCLOG("avformat_find_stream_info false");
        return false;  
    }  
    
    CCLOG("avg_frame_rate =  %d / %d", m_pFormatCtx->streams[0]->avg_frame_rate.num, m_pFormatCtx->streams[0]->avg_frame_rate.den);

    m_videoStream = -1;
   
    for(int i=0; i<m_pFormatCtx->nb_streams; i++) {  
        
        if(m_videoStream == -1 && m_pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {  
            m_videoStream = i; 
            break;
        }

    } 
    
    //  没有视频流，无法播放  
    if(m_videoStream == -1) {
        CCLOGERROR("没有视频流，无法播放");  
        return false;  
    } 

    m_pCodecCtx = m_pFormatCtx->streams[m_videoStream]->codec;
    // 获取基本信息
    if (m_pCodecCtx->width) 
    {
        m_width = m_pCodecCtx->width;
        m_height = m_pCodecCtx->height;
    }
    else
    {
        CCLOGERROR("获取视频尺寸失败");  
        return false;
    }

     /*
      Duration: 00:00:07.32, start: 0.000000, bitrate: 579 kb/s
    Stream #0:0: Video: vp6a, yuva420p, 224x240,
     31 fps, 31 tbr, 1k tbn, 1k tbc
     */
    
    //算时间
    int64_t duration = 0;
    if (m_pFormatCtx->duration != AV_NOPTS_VALUE) {
        /*int hours, mins, secs, us;*/
        duration = m_pFormatCtx->duration + 5000;
        /*secs = duration / AV_TIME_BASE;
        us = duration % AV_TIME_BASE;
        mins = secs / 60;
        secs %= 60;
        hours = mins / 60;
        mins %= 60;
        CCLOG("%02d:%02d:%02d.%02d", hours, mins, secs, (100 * us) / AV_TIME_BASE);*/
    } else {
        CCLOGERROR("duration is null");
        return false;
    }

    CCLOG("avg_frame_rate =  %d / %d", m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.num, m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.den);
    
    AVRational rational;

    if(m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.den && m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.num)
    {           
        rational = m_pFormatCtx->streams[m_videoStream]->avg_frame_rate;
    }
    else if(m_pFormatCtx->streams[m_videoStream]->r_frame_rate.den && m_pFormatCtx->streams[m_videoStream]->r_frame_rate.num)
    {
        rational = m_pFormatCtx->streams[m_videoStream]->r_frame_rate;
    }
    else
    {
        CCLOGERROR("fps 获取失败");  
        return false;
    }

    double fps = av_q2d(rational);
    m_frameRate = 1.0 / fps;
    m_frames = (int)((fps * duration) / AV_TIME_BASE);
    CCLOG("m_frameRate = %f , frames = %d", m_frameRate, m_frames);



    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);

    if(pCodec == NULL) { 
        CCLOGERROR("avcodec_find_decoder error");
        return false;
    }  
      
    if(avcodec_open2(m_pCodecCtx, pCodec, NULL)) { 
        CCLOGERROR("avcodec_open2 error");
        return false;
    }  

    m_pFrame=avcodec_alloc_frame();
    CCLOG("width: %d, hight: %d", m_pCodecCtx->width, m_pCodecCtx->height);
    m_pSwsCtx = sws_getContext(m_pCodecCtx->width,  
                                     m_pCodecCtx->height,  
                                     m_pCodecCtx->pix_fmt,
                                     m_pCodecCtx->width,  
                                     m_pCodecCtx->height,  
                                     PIX_FMT_RGBA,  
                                     SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if(!m_pSwsCtx)
    {
        CCLOGERROR("sws_getContext error");
        return false;
    }
    return true;
}


unsigned int CCVideoDecode::getWidth()
{
    return m_width;
}

unsigned int CCVideoDecode::getHeight()
{
    return m_height;
}

double CCVideoDecode::getFrameRate(){
    return m_frameRate;
}

unsigned int CCVideoDecode::getFrames()
{
    return m_frames;
}

const char* CCVideoDecode::getFilePath()
{
    return m_filepath;
}

CCVideoDecode::~CCVideoDecode()
{
    CCLOGINFO("cocos2d: deallocing CCVideoDecode.");
    if(m_pSwsCtx) sws_freeContext(m_pSwsCtx); 
    

    // Free the YUV frame 
    if(m_pFrame) av_free(m_pFrame);  
    
    // Close the codec  
    if (m_pCodecCtx) avcodec_close(m_pCodecCtx);  
    if (m_pFormatCtx) avformat_close_input(&m_pFormatCtx);

}

/**
 * 解码
 * @return [description]
 */
bool CCVideoDecode::decode()
{
    if(m_frameCount == -1)
        return false;

	AVPacket packet;
	int frameFinished = 0;
    while(!frameFinished && av_read_frame(m_pFormatCtx, &packet)>=0) {  
        // Is this a packet from the video stream?  
        if(packet.stream_index== m_videoStream) {  
            // Decode video frame  
            avcodec_decode_video2(m_pCodecCtx, m_pFrame, &frameFinished, &packet);  
        }  
          
        // Free the packet that was allocated by av_read_frame  
        av_free_packet(&packet);  
    }

    m_frameCount++;
    
    CCVideoPic *pVideoPic = new CCVideoPic();
    pVideoPic->init(m_filepath, m_frameCount,m_videoStream, m_pCodecCtx, m_pSwsCtx, m_pFrame);

    CCVideoTextureCache::sharedTextureCache()->addPicData(pVideoPic);
   	
   	if (frameFinished == 0) {
   		//重头开始解码
   		//av_seek_frame(m_pFormatCtx, m_videoStream , 0, AVSEEK_FLAG_ANY);
   		m_frameCount = -1;
   	}
    return true;
}

NS_CC_END
