/**
 * 视频播放层
 * 依赖ffmpeg库
 * 
 * @author leoluo<luochong1987@gmail.com>
 * 
 */
  
#include "CCVideoLayer.h"  
#include "CCVideoTextureCache.h" 
#include "CCVideoDecode.h"
#include "script_support/CCScriptSupport.h"

extern "C" { 
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
}  

NS_CC_BEGIN  
CCVideoLayer* CCVideoLayer::create(const char* path)  
{  
    CCVideoLayer* video = new CCVideoLayer();  
    if (video && video->init(path)) {  
         video->autorelease();
         return video;
    }
    CC_SAFE_DELETE(video);
    return NULL;  
}  
  
CCVideoLayer::CCVideoLayer()  
{  
    m_frameRate = 1.0 / 31;
    m_frame_count = 1;  
    m_enableTouchEnd = false;  
    m_width = 100;
    m_height = 100;
    m_playEndScriptHandler = 0;
}  
  
CCVideoLayer::~CCVideoLayer()  
{  
 CCVideoTextureCache::sharedTextureCache()->removeVideo(m_strFileName.c_str());
 unregisterPlayScriptHandler();
}
  
bool CCVideoLayer::init(const char* path)  
{  
    m_strFileName = path;

    CCVideoDecode *pVideoDecode = CCVideoTextureCache::sharedTextureCache()->addVideo(path);
    if(!pVideoDecode)
    {
        CCLOGERROR("videoDecode get error in %s", "CCVideoLayer");
        return false;
    }

    m_width = pVideoDecode->getWidth();
    m_height = pVideoDecode->getHeight();
    m_frames = pVideoDecode->getFrames();   // 总帧数 
    m_frameRate = pVideoDecode->getFrameRate();             // 帧率


    //AVPicture *picture = new AVPicture;  
    //avpicture_alloc(picture, PIX_FMT_RGBA, m_width, m_height);



    // 渲染的纹理  
    CCTexture2D *texture = new CCTexture2D();

    // Convert "RRRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA" to "RRRRGGGGBBBBAAAA"
/*  我还会回来的
    unsigned int length = m_width * m_height;
    unsigned int* inPixel32 = (unsigned int*)picture->data[0];  
    unsigned char* tempData = new unsigned char[m_width * m_height * 2];
    unsigned short* outPixel16 = (unsigned short*)tempData;
    
    for(unsigned int i = 0; i < length; ++i, ++inPixel32)
    {
        *outPixel16++ = 
        ((((*inPixel32 >> 0) & 0xFF) >> 4) << 12) | // R
        ((((*inPixel32 >> 8) & 0xFF) >> 4) <<  8) | // G
        ((((*inPixel32 >> 16) & 0xFF) >> 4) << 4) | // B
        ((((*inPixel32 >> 24) & 0xFF) >> 4) << 0);  // A
    }
*/

    unsigned int length = m_width * m_height * 4;
    unsigned char* tempData = new unsigned char[length];
    for(unsigned int i = 0; i < length; ++i)
    {
        tempData[i] = 0;
    }

    texture->initWithData(tempData, kCCTexture2DPixelFormat_RGBA8888, m_width, m_height, CCSize(m_width, m_height));  
    
    initWithTexture(texture);

    this->setContentSize(CCSize(m_width, m_height));

    delete [] tempData;

    /*if(picture)
    {
        avpicture_free(picture);  
        delete picture; 
    }*/

    return true;  
}


  
void CCVideoLayer::playVideo()  
{  
    update(0);
    this->schedule(schedule_selector(CCVideoLayer::update), m_frameRate);
}  
  
void CCVideoLayer::stopVideo(void)  
{  
    this->unscheduleAllSelectors(); 
}  
  

  
void CCVideoLayer::seek(int frame)  
{  
    m_frame_count = frame;
    update(0);
}  
  
void CCVideoLayer::update(float dt)  
{ 

    CCTexture2D *texture = NULL;
    texture = CCVideoTextureCache::sharedTextureCache()->getTexture(m_strFileName.c_str(), m_frame_count);
    if(texture)
    {
        m_frame_count++;
        setTexture(texture);
        //this->stop();
        if(m_frame_count > m_frames)
        {
            m_frame_count = 1; 
            //this->stop();  
            if (m_videoEndCallback) {  
                m_videoEndCallback();
            }
            if(m_playEndScriptHandler)
                CCScriptEngineManager::sharedManager()->getScriptEngine()->executeEvent(m_playEndScriptHandler, "playEnd");
        }  
    }
    else
    {
        CCLOG("获取纹理失败 CCVideoLayer::update filename = %s , frame = %d", m_strFileName.c_str(),m_frame_count);
    }

}  
  
void CCVideoLayer::draw(void)  
{
    CCSprite::draw();
} 


void CCVideoLayer::registerPlayScriptHandler(int nHandler)
{
    unregisterPlayScriptHandler();
    m_playEndScriptHandler = nHandler;
    LUALOG("[LUA] Add CCVideoLayer event handler: %d", m_playEndScriptHandler);
}

void CCVideoLayer::unregisterPlayScriptHandler(void)
{
    if (m_playEndScriptHandler)
    {
        CCScriptEngineManager::sharedManager()->getScriptEngine()->removeScriptHandler(m_playEndScriptHandler);
        LUALOG("[LUA] Remove CCVideoLayer event handler: %d", m_playEndScriptHandler);
        m_playEndScriptHandler = 0;
    }
}


  
void CCVideoLayer::setVideoEndCallback(std::function<void(void)> func)  
{  
    m_videoEndCallback = func;  
}  
  
  
NS_CC_END  