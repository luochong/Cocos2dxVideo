#ifndef __CCVIDEO_LAYER_H__
#define __CCVIDEO_LAYER_H__


#include "cocos2d.h"  
#include <string>    
#ifdef __APPLE__  
#include <tr1/functional>  
namespace std {  
    namespace tr1 {}  
    using namespace tr1;  
    using tr1::function;  
}  
//using namespace std::tr1;  
#else  
#include <functional>  
#endif  

  
NS_CC_BEGIN  
  
class CCVideoLayer : public CCSprite  
{  
public:  
    static CCVideoLayer* create(const char* path); 


    CCVideoLayer();  
    virtual ~CCVideoLayer();  
      
    bool init(const char* path);  
    void playVideo(void);  
    void stopVideo(void); 
    void seek(int frame);  
    void draw(void);  
    void update(float dt);  
      
    void registerPlayScriptHandler(int nHandler);
    void setVideoEndCallback(std::function<void()> func);       // 关闭视频回调  
private:
    void unregisterPlayScriptHandler(void);
    unsigned int m_width;  
    unsigned int m_height;
    unsigned int m_frames;   // 总帧数 
    double m_frameRate;             // 帧率      

    int m_playEndScriptHandler;
    unsigned int m_frame_count;               // 已经播放帧数
    std::string m_strFileName;

    bool m_enableTouchEnd;  
    std::function<void()> m_videoEndCallback;  
};  
  
  
NS_CC_END

#endif //__CCVIDEO_LAYER_H__