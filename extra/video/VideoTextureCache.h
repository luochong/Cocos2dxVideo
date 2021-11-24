/**
 * 视频纹理缓存
 * @author leoluo<luochong1987@gmail.com>
 */
#ifndef __VIDEOTEXTURE_CACHE_H__
#define __VIDEOTEXTURE_CACHE_H__

#include "cocos2d.h"
#include "VideoDecode.h"
USING_NS_CC;

class CC_DLL VideoTextureCache : public Ref
{
	private:
		std::unordered_map<std::string, std::unordered_map<std::string, Texture2D*>> m_pTextures;
		std::unordered_map<std::string, VideoDecode*> decoderMap;
		std::string getTextureKeyName(std::string path, int frame);
	public:
		VideoTextureCache();
		virtual ~VideoTextureCache();

		static VideoTextureCache * getInstance();
		
		void addImageWidthData(const char *filename, int frame, const void *data, Texture2D::PixelFormat pixelFormat, unsigned int pixelsWide, unsigned int pixelsHigh, const Size& contentSize);
		
		Texture2D* getTexture(const char *filename, int frame);

	    void removeAllTextures();

		VideoDecode* addVideo(const char *path);
	    void removeVideo(const char *path);
		void removeTexture(const char *path, int frame);

	    void picToTexture(float dt);
};

#endif //__CCVIDEOTEXTURE_CACHE_H__

