/**
 * 视频解码类
 * 目前只支持视频流、不包括音频
 */
#ifndef __VIDEO_DECODE_H__
#define __VIDEO_DECODE_H__

#include "cocos2d.h"
USING_NS_CC;

extern "C" { 
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"  
    #include "libswscale/swscale.h"  
}

struct AVFormatContext;  
struct AVCodecContext;  
struct AVFrame;  
struct AVPicture;  
struct SwsContext; 


class VideoPic : public Ref
{
	public:
		VideoPic(const char *path, int frame, unsigned int width, unsigned int height, unsigned char* data);
		~VideoPic();

		const char *m_path;
		int m_frame;
		int m_width;
		int m_height;
		unsigned char* m_pPicture;
};

 
class CC_DLL VideoDecode : public Ref
{
	private:
		AVFormatContext *m_pFormatCtx;  
	    AVCodecContext *m_pCodecCtx;  
	    AVFrame *m_pFrame;  
	    
	    int m_videoStream;
	    SwsContext *m_pSwsCtx;

	    int m_frameCount;   //解码到第几帧
		int m_frameMax;
	    char *m_filepath; //视频文件路径
	    double m_frameRate; //帧率
	    unsigned int m_frames;   // 总帧数
	    unsigned int m_width;  
    	unsigned int m_height;   
	public:
		VideoDecode();
		~VideoDecode();
		bool init(const char *path);
		unsigned int getWidth();
		unsigned int getHeight();
		double getFrameRate();
		unsigned int getFrames();
		const char* getFilePath();
		void setNeedMaxFrame(int maxCount, int startCount);
		int getNeedMaxFrame();
		VideoPic* decode();
};

#endif //__CCVIDEO_DECODE_H__
