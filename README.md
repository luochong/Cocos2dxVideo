Cocos2dxVideo
=============

Cocos2dx play video flv,...

Cocos2dx play video use ffmpeg , use like CCSprite

```c++
	VideoSprite *p = VideoSprite::create("/sdcard/video.flv");
	p->setPosition(ccp(100, 100));
	addChild(p);
	p->playVideo();	
```

 

Android build libffmpeg
---

https://github.com/yixia/FFmpeg-Android




