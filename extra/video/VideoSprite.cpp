/**
 * 视频播放层
 * 依赖ffmpeg库
 * 
 * @author leoluo<luochong1987@gmail.com>
 * 
 */
  
#include "VideoSprite.h"  
#include "VideoTextureCache.h" 
#include "VideoDecode.h"

VideoSprite* VideoSprite::create(const char* path)
{  
	VideoSprite* video = new VideoSprite();
	video->autorelease();
    if (video && video->initWithFile(path)) {
         return video;
    }
    return NULL;  
}  

VideoSprite* VideoSprite::createWithUrl(const char* url)
{
	VideoSprite* video = new VideoSprite();
	video->autorelease();
	if (video && video->initWithUrl(url)) {
		return video;
	}
	return NULL;
}
VideoSprite::VideoSprite()
{  
    m_frameRate = 1.0 / 31;
    m_frame_count = 1;
    m_width = 100;
    m_height = 100;
}  
  
VideoSprite::~VideoSprite()
{  

}
  
bool VideoSprite::initWithFile(const char* path)
{  
	std::string fullPath = FileUtils::getInstance()->fullPathForFilename(path);
	assert(FileUtils::getInstance()->isFileExist(fullPath));

    m_strFileName = fullPath;

	m_pDecoder = VideoTextureCache::getInstance()->addVideo(m_strFileName.c_str());
    if(!m_pDecoder)
    {
        CCLOGERROR("videoDecode get error in %s", "CCVideoLayer");
        return false;
    }

    m_width = m_pDecoder->getWidth();
    m_height = m_pDecoder->getHeight();
    m_frames = m_pDecoder->getFrames();   // 总帧数 
    m_frameRate = m_pDecoder->getFrameRate();             // 帧率

    // 渲染的纹理  
    Texture2D *texture = new Texture2D();
    unsigned int length = m_width * m_height * 4;
    unsigned char* tempData = new unsigned char[length];
    for(unsigned int i = 0; i < length; ++i)
    {
        tempData[i] = 0;
    }

    texture->initWithData(tempData, m_width * m_height, Texture2D::PixelFormat::RGBA8888, m_width, m_height, Size(m_width, m_height));
    
    initWithTexture(texture);

    this->setContentSize(Size(m_width, m_height));

    delete [] tempData;

    return true;  
}


bool VideoSprite::initWithUrl(const char* path)
{
	m_strFileName = path;

	m_pDecoder = VideoTextureCache::getInstance()->addVideo(m_strFileName.c_str());
	if (!m_pDecoder)
	{
		CCLOGERROR("videoDecode get error in %s", "CCVideoLayer");
		return false;
	}

	m_width = m_pDecoder->getWidth();
	m_height = m_pDecoder->getHeight();
	m_frames = m_pDecoder->getFrames();   // 总帧数 
	m_frameRate = m_pDecoder->getFrameRate();             // 帧率

															// 渲染的纹理  
	Texture2D *texture = new Texture2D();
	unsigned int length = m_width * m_height * 4;
	unsigned char* tempData = new unsigned char[length];
	for (unsigned int i = 0; i < length; ++i)
	{
		tempData[i] = 0;
	}

	texture->initWithData(tempData, m_width * m_height, Texture2D::PixelFormat::RGBA8888, m_width, m_height, Size(m_width, m_height));

	initWithTexture(texture);

	this->setContentSize(Size(m_width, m_height));

	delete[] tempData;

	return true;
}
  
void VideoSprite::playVideo()
{  
	updateTexture(0);
    this->schedule(schedule_selector(VideoSprite::updateTexture), m_frameRate);
}  
  
void VideoSprite::stopVideo(void)
{  
    this->unscheduleAllCallbacks();
}
  
void VideoSprite::seek(int frame)
{  
    m_frame_count = frame;
	updateTexture(0);
}  
  
void VideoSprite::updateTexture(float dt)
{ 
    Texture2D *texture = VideoTextureCache::getInstance()->getTexture(m_strFileName.c_str(), m_frame_count);
    if(texture)
    {
        m_frame_count++;
        setTexture(texture);
		
        if(m_frame_count > m_frames)
        {
            m_frame_count = 1;  //循环播放
        }
		m_pDecoder->setNeedMaxFrame(m_frame_count + 10, m_frame_count - 1);
		VideoTextureCache::getInstance()->removeTexture(m_strFileName.c_str(), m_frame_count - 10);
    }
    else
    {
        CCLOG("获取纹理失败 CCVideoLayer::update filename = %s , frame = %d", m_strFileName.c_str(),m_frame_count);
    }

}