#ifndef __VIDEO_SPRITE_H__
#define __VIDEO_SPRITE_H__

#include "cocos2d.h"  
#include <string>    
USING_NS_CC;

class VideoDecode;

class CC_DLL VideoSprite : public Sprite
{  
public:  
    static VideoSprite* create(const char* path);
	static VideoSprite* createWithUrl(const char* url);
	VideoSprite();
    virtual ~VideoSprite();
      
    bool init(const char* path);  
    void playVideo(void);  
    void stopVideo(void); 
    void seek(int frame);  
    void updateTexture(float dt);
   
private:
    unsigned int m_width;  
    unsigned int m_height;
    unsigned int m_frames;			// 总帧数 
    double m_frameRate;             // 帧率      
	VideoDecode* m_pDecoder;

    unsigned int m_frame_count;               // 已经播放帧数
    std::string m_strFileName;
	bool initWithFile(const char* path);
	bool initWithUrl(const char* path);
}; 

#endif //__CCVIDEO_LAYER_H__