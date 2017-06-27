rem ffmpeg -ss 00:02:30 -i video/1v.mp4 -ss 00:00:30 -o keyframe
rem ffprobe -select_streams v -show_frames video/1v.mp4
rem ffmpeg -ss 00:05:19 -i video/1v.mp4 -to 00:06:49 -vframes 1 thumbnails.jpg
rem ffmpeg  -i video/1v.mp4 -vf select="eq(pict_type\,PICT_TYPE_I)" -vsync 2 -s 73x41 -r 30 -f image2 keyframe/1v/thumbnails-%%02d.jpeg
rem ffmpeg -i "video/V_for_Vendetta.mp4" -ss 00:00:58 -to 00:01:00 -vf select="eq(pict_type\,PICT_TYPE_I)" -vsync 2 -r 30 -f image2 keyframe/V_for_Vendetta/thumbnails-%%02d.jpeg
ffmpeg -i "video/V_for_Vendetta.mp4" -ss 00:00:58.527 -to 00:01:00.688 -vf select="eq(pict_type\,PICT_TYPE_I)" -vsync 2 -r 30 -f image2 keyframe/V_for_Vendetta/thumbnails-%%02d.jpeg
pause