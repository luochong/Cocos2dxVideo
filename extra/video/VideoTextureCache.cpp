/**
 * 视频材质缓存类
 */

#include "VideoTextureCache.h"
#include "VideoDecode.h"
#include <thread>

static VideoTextureCache *_instance = NULL;

static std::deque<VideoPic*> s_AsyncVideoPicQueue;
std::mutex mutex;

void *videoDecode(void *data)
{
    VideoDecode *decoder = (VideoDecode*) data;
    if(decoder)
    {
		VideoPic* picStruct = nullptr;
		while (true) {
			if (decoder->getNeedMaxFrame() == 0) {
				break;
			}
			picStruct = decoder->decode();
			if (picStruct != nullptr) {
				mutex.lock();
				s_AsyncVideoPicQueue.push_back(picStruct);
				mutex.unlock();
			}
		}
    }
    return 0;
}

VideoTextureCache * VideoTextureCache::getInstance()
{
    if (!_instance)
    {
		_instance = new VideoTextureCache();
    }
    return _instance;
}


VideoTextureCache::VideoTextureCache()
{
    CCAssert(_instance == NULL, "Attempted to allocate a second instance of a singleton.");

	Director::getInstance()->getScheduler()->scheduleSelector(schedule_selector(VideoTextureCache::picToTexture), this, 0, false);
}

VideoTextureCache::~VideoTextureCache()
{
    CCLOGINFO("cocos2d: deallocing VideoTextureCache.");
}

VideoDecode* VideoTextureCache::addVideo(const char *path)
{
	VideoDecode* pVideoDecode = nullptr;
	if (decoderMap.find(path) == decoderMap.end()) {
		pVideoDecode = new VideoDecode();
		if (pVideoDecode->init(path))
		{
			std::unordered_map<std::string, Texture2D*> videoMap;
			m_pTextures[path] = videoMap;

			//开启线程进行解码
			std::thread worker(videoDecode, pVideoDecode);
			worker.detach();

			decoderMap[path] = pVideoDecode;
			return pVideoDecode;
		}
		else
		{
			CCLOGERROR("VideoDecode init error in VideoTextureCache");
			return nullptr;
		}
	}
	else {
		return decoderMap[path];
	}
}

/**
 *  图片转纹理
 */
void VideoTextureCache::picToTexture(float dt)
{
    VideoPic *pVideoPic = NULL;
    int length = s_AsyncVideoPicQueue.size();
    for(int i = 0; i < length; i++)
    {
		mutex.lock();
		pVideoPic = s_AsyncVideoPicQueue.at(0);
		s_AsyncVideoPicQueue.pop_front();
		mutex.unlock();

        if (pVideoPic)
        {
			addImageWidthData(pVideoPic->m_path, pVideoPic->m_frame, 
				pVideoPic->m_pPicture, 
				Texture2D::PixelFormat::RGBA8888,
                pVideoPic->m_width, pVideoPic->m_height, 
                Size(pVideoPic->m_width, pVideoPic->m_height)
            );
            pVideoPic->release();
        }
    }
}
std::string VideoTextureCache::getTextureKeyName(std::string path, int frame)
{
	std::ostringstream keystream;
	keystream << path << "_" << frame;

	std::string key = keystream.str();
	return key;
}
void VideoTextureCache::addImageWidthData(const char *path, int frame, const void *data, Texture2D::PixelFormat pixelFormat, 
											unsigned int pixelsWidth, unsigned int pixelsHight, const Size& contentSize)
{
	if (m_pTextures.find(path) != m_pTextures.end()) {
		std::unordered_map<std::string, Texture2D*> videoMap = m_pTextures[path];

		std::string key = getTextureKeyName(path, frame);
		if(videoMap.find(key) == videoMap.end()) {
			Texture2D* texture = new Texture2D();
			if (texture &&
				texture->initWithData(data, pixelsWidth * pixelsHight, pixelFormat, pixelsWidth, pixelsHight, contentSize))
			{
				videoMap[key] = texture;
				m_pTextures[path] = videoMap;
			}
		}
	}
}

void VideoTextureCache::removeVideo(const char *path)
{
	if (m_pTextures.find(path) != m_pTextures.end()) {
		std::unordered_map<std::string, Texture2D*> videoMap = m_pTextures[path];
		std::unordered_map<std::string, Texture2D*>::iterator iter;
		for (iter = videoMap.begin(); iter != videoMap.end(); ++iter) {
			iter->second->release();
		}
		m_pTextures.erase(path);
	}
}
void VideoTextureCache::removeTexture(const char *path, int frame)
{
	if (m_pTextures.find(path) != m_pTextures.end()) {
		std::unordered_map<std::string, Texture2D*> videoMap = m_pTextures[path];
		std::string key = getTextureKeyName(path, frame);
		if (videoMap.find(key) != videoMap.end()) {
			videoMap[key]->release();
			videoMap.erase(key);
		}
	}
}

Texture2D* VideoTextureCache::getTexture(const char *path, int frame)
{
	if (m_pTextures.find(path) != m_pTextures.end()) {
		std::string key = getTextureKeyName(path, frame);
		std::unordered_map<std::string, Texture2D*> videoMap = m_pTextures[path];
		return videoMap[key];
	}
	decoderMap.erase(path);
	return nullptr;
}
    
void VideoTextureCache::removeAllTextures()
{
	std::unordered_map<std::string, std::unordered_map<std::string, Texture2D*>>::iterator iter;
	for (iter = m_pTextures.begin(); iter != m_pTextures.end(); ++iter) {
		removeVideo(iter->first.c_str());
	}
}

