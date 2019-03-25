### 介绍

该程序实现windows的录屏，投屏功能。

- 屏幕复制

![1553494000066](C:\Users\Administrator\Source\Repos\屏幕分享例程\pic\1553494000066.png)

- 拓展屏投屏

  拓展屏是使用一款空间开启的一个虚拟屏幕

  ![1553494228778](C:\Users\Administrator\Source\Repos\屏幕分享例程\pic\1553494228778.png)

- 屏幕录制

  使用ffmpeg开发库，录制的视频编码格式可指定为h264或者mpeg，会在本地生成文件，使用vlc播放器即可播放，或者使用 `ffmpeg -i save.h264 -codec copy save.mp4` 

  命令转换为mp4格式。

- SDL播放器播放测试

  SDL为windows下的一个播放器，实现实时内容播放。

![1553494687354](C:\Users\Administrator\Source\Repos\屏幕分享例程\pic\1553494687354.png)