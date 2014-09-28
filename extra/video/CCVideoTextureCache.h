/**
 * 视频纹理缓存
 * @author leoluo<luochong1987@gmail.com>
 */
#ifndef __CCVIDEOTEXTURE_CACHE_H__
#define __CCVIDEOTEXTURE_CACHE_H__

#include "cocoa/CCObject.h"
#include "cocoa/CCDictionary.h"
#include "textures/CCTexture2D.h"
#include <string>
#include "CCVideoDecode.h"
 
NS_CC_BEGIN
class CC_DLL CCVideoTextureCache : public CCObject
{
	protected:
		CCDictionary* m_pTextures;
		CCDictionary* m_pVideoDecodes;
	public:
		CCVideoTextureCache();
		virtual ~CCVideoTextureCache();

		static CCVideoTextureCache * sharedTextureCache();
		static void purgeSharedTextureCache();
		
		CCTexture2D* addImageWidthData(const char *filename, int frame, const void *data, CCTexture2DPixelFormat pixelFormat, unsigned int pixelsWide, unsigned int pixelsHigh, const CCSize& contentSize);
		CCTexture2D* getTexture(const char *filename, int frame);

	    void removeAllTextures();

	    void removeTexture(const char *filename, int frame);

	    /**
	     * 视频解码
	     */
	    CCVideoDecode* addVideo(const char *path);
	    void removeVideo(const char *path);

	    void addPicData(CCVideoPic *pVideoPic);
	    void picToTexture();

};
NS_CC_END
#endif //__CCVIDEOTEXTURE_CACHE_H__

