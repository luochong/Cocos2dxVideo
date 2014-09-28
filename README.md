Cocos2dxVideo
=============

Cocos2dx play video flv,...

Cocos2dx play video use ffmpeg , use like CCSprite

```c++
	CCVideoLayer *p = CCVideoLayer::create("/sdcard/video.flv");
	p->setPosition(ccp(100, 100));
	addChild(p);
	p->playVideo();
	//p->setVideoEndCallback(callback);
	//p->stopVideo();
	
```

tolua quick-cocos2d-x

CCVideoLayerExtend.lua
 

Android build libffmpeg
---

https://github.com/yixia/FFmpeg-Android


IOS build libffmpeg
---

TODO


