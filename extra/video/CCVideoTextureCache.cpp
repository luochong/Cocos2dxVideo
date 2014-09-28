/**
 * 视频材质缓存类
 */

#include "CCVideoTextureCache.h"
#include "CCVideoDecode.h"
#include "CCDirector.h"
#include "CCScheduler.h"
#include <sstream>
#include <queue>
#include "platform/platform.h"
#include "platform/CCThread.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_WINRT) && (CC_TARGET_PLATFORM != CC_PLATFORM_WP8)
#include <pthread.h>
#else
#include "CCPThreadWinRT.h"
#include <ppl.h>
#include <ppltasks.h>
using namespace concurrency;
#endif


using namespace std;
NS_CC_BEGIN

static pthread_mutex_t      s_asyncVideoPicQueueMutex;
static std::queue<CCVideoPic*>* s_pAsyncVideoPicQueue = NULL;

static CCVideoTextureCache *g_sharedTextureCache = NULL;

static pthread_t s_decodeThread;


static void *videoDecode(void *data)
{
    CCVideoDecode *p = (CCVideoDecode *) data;
    if(p)
    {
        while(p->decode())
        {
            //sleep ?
        }
    }
    return 0;
}



CCVideoTextureCache * CCVideoTextureCache::sharedTextureCache()
{
    if (!g_sharedTextureCache)
    {
        g_sharedTextureCache = new CCVideoTextureCache();
    }
    return g_sharedTextureCache;
}

void CCVideoTextureCache::purgeSharedTextureCache()
{
    CC_SAFE_RELEASE_NULL(g_sharedTextureCache);
}


CCVideoTextureCache::CCVideoTextureCache()
{
    CCAssert(g_sharedTextureCache == NULL, "Attempted to allocate a second instance of a singleton.");
    m_pTextures = new CCDictionary();
    m_pVideoDecodes = new CCDictionary();
}

CCVideoTextureCache::~CCVideoTextureCache()
{
    CCLOGINFO("cocos2d: deallocing CCVideoTextureCache.");
    CC_SAFE_RELEASE(m_pTextures);
    CC_SAFE_RELEASE(m_pVideoDecodes);
    pthread_mutex_destroy(&s_asyncVideoPicQueueMutex);
}


CCVideoDecode* CCVideoTextureCache::addVideo(const char *path)
{

    CCVideoDecode* pVideoDecode = (CCVideoDecode*)m_pVideoDecodes->objectForKey(path);
    if(!pVideoDecode)
    {
        pVideoDecode = new CCVideoDecode();
        if(pVideoDecode->init(path))
        {
            m_pVideoDecodes->setObject(pVideoDecode, path);
            //开启线程进行解码
            pthread_create(&s_decodeThread, NULL, videoDecode, pVideoDecode);
            pVideoDecode->release();


            if (s_pAsyncVideoPicQueue == NULL)
            {   
                s_pAsyncVideoPicQueue = new queue<CCVideoPic*>();
                pthread_mutex_init(&s_asyncVideoPicQueueMutex, NULL);
                CCDirector::sharedDirector()->getScheduler()->scheduleSelector(schedule_selector(CCVideoTextureCache::picToTexture), this, 0, false);
            }
        }
        else
        {
            CCLOGERROR("CCVideoDecode init error in CCVideoTextureCache");
            return NULL;
        }
    }
    else
    {
        pVideoDecode->retain();
    }

    return pVideoDecode;
}


void CCVideoTextureCache::addPicData(CCVideoPic *pVideoPic)
{
    pthread_mutex_lock(&s_asyncVideoPicQueueMutex);
    s_pAsyncVideoPicQueue->push(pVideoPic);
    pthread_mutex_unlock(&s_asyncVideoPicQueueMutex);
}

/**
 *  图片转纹理
 */
void CCVideoTextureCache::picToTexture()
{
    CCVideoPic *pVideoPic = NULL;
    int length = m_pVideoDecodes->count();
    for(int i = 0; i < length; i++)
    {
        pthread_mutex_lock(&s_asyncVideoPicQueueMutex);
        if (!s_pAsyncVideoPicQueue->empty())
        {
            pVideoPic = s_pAsyncVideoPicQueue->front();
            s_pAsyncVideoPicQueue->pop();
            pthread_mutex_unlock(&s_asyncVideoPicQueueMutex);
            if(pVideoPic)
            {
                
                // Convert "RRRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA" to "RRRRGGGGBBBBAAAA"
                /*  我还会回来的
                unsigned int length = pVideoPic->m_width * pVideoPic->m_height;
                unsigned int* inPixel32 = (unsigned int*)pVideoPic->m_pPicture->data[pVideoPic->m_videoStream];  
                unsigned char* tempData = new unsigned char[pVideoPic->m_width * pVideoPic->m_height * 2];
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
                addImageWidthData(pVideoPic->m_path, pVideoPic->m_frame, 
                    pVideoPic->m_pPicture->data[pVideoPic->m_videoStream], 
                    kCCTexture2DPixelFormat_RGBA8888, 
                    pVideoPic->m_width, pVideoPic->m_height, 
                    CCSize(pVideoPic->m_width, pVideoPic->m_height)
                );

               // delete [] tempData;
                pVideoPic->release();
            }
        }
        else
        {
            pthread_mutex_unlock(&s_asyncVideoPicQueueMutex);
            break;
        }
    }
}

void CCVideoTextureCache::removeVideo(const char *path)
{
    CCVideoDecode* pVideoDecode = (CCVideoDecode*)m_pVideoDecodes->objectForKey(path);
    if(pVideoDecode)
    {
        unsigned int rcount =  pVideoDecode->retainCount();
        if(rcount == 1)
        {
            unsigned int frames = pVideoDecode->getFrames();
            for(; frames > 0; frames--)
            {
                removeTexture(path, frames);
            }
            m_pVideoDecodes->removeObjectForKey(path);
        }
        else
        {
            pVideoDecode->release();
        }
    }
}


CCTexture2D* CCVideoTextureCache::getTexture(const char *filename, int frame)
{
    std::ostringstream keystream;
    keystream << filename << "_" << frame;
    CCTexture2D * texture = NULL;
	texture = (CCTexture2D*)m_pTextures->objectForKey(keystream.str());
	return texture;
}
    	
CCTexture2D* CCVideoTextureCache::addImageWidthData(const char *filename, int frame, const void *data, CCTexture2DPixelFormat pixelFormat, unsigned int pixelsWide, unsigned int pixelsHigh, const CCSize& contentSize)
{
    std::ostringstream keystream;
    keystream << filename << "_" << frame;

    //CCLOG("cocos2d: create texture for file:%s in CCVideoTextureCache", keystream.str().c_str());
    std::string key = keystream.str();
    CCTexture2D * texture = NULL;
	texture = (CCTexture2D*)m_pTextures->objectForKey(key);
	if(!texture)
	{
		texture = new CCTexture2D();
        if( texture && 
        	texture->initWithData(data, pixelFormat, pixelsWide, pixelsHigh, contentSize) )
        {
            //CCLOG("cocos2d: create texture for file:%s in CCVideoTextureCache", key.c_str());
            m_pTextures->setObject(texture, key);
            texture->release();
		}
		else
        {
            CCLOG("cocos2d: Couldn't create texture for file:%s in CCVideoTextureCache", key.c_str());
        }
	}
    else
    {
        CCLOG("纹理已经存在 - %s", key.c_str());
    }

	return texture;
}

void CCVideoTextureCache::removeAllTextures()
{
    m_pTextures->removeAllObjects();
}


/** 
 * Deletes a texture from the cache given a its key name
 */
void CCVideoTextureCache::removeTexture(const char *filename, int frame)
{
    std::ostringstream keystream;
    keystream << filename << "_" << frame;
    m_pTextures->removeObjectForKey(keystream.str());
}
NS_CC_END


