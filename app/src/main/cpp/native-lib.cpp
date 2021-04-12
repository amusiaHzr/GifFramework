#include <jni.h>
#include <string>
#include <Android/bitmap.h>
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "fuck NDK", __VA_ARGS__)

extern "C" {
#include "gif_lib.h"
}
#define  argb(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)
#define  dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  trans_index(ext) ((ext)->Bytes[3])
#define  transparency(ext) ((ext)->Bytes[0] & 1)

int current_frame;
int total_frame;

extern "C"
JNIEXPORT jlong JNICALL
Java_com_amusia_gifframework_GifLoader_loadPath(JNIEnv *env, jclass clazz, jstring _path) {
    const char *path = env->GetStringUTFChars(_path, 0);
    int error;
    //打开Gif文件，并获取到GifFileType结构体指针
    GifFileType *gifFileType = DGifOpenFileName(path, &error);
    //为gifFileType分配内存
    DGifSlurp(gifFileType);
    if (error) {
        current_frame = 0;
        total_frame = gifFileType->ImageCount;

    }
    env->ReleaseStringUTFChars(_path, path);
    return (jlong) (gifFileType);
}


int drawFrame(GifFileType *gif, AndroidBitmapInfo info, void *pixels, bool force_dispose_1) {
    GifColorType *bg;

    GifColorType *color;

    SavedImage *frame;

    ExtensionBlock *ext = 0;

    GifImageDesc *frameInfo;

    ColorMapObject *colorMap;

    int *line;

    int width, height, x, y, j, loc, n, inc, p;

    void *px;

    width = gif->SWidth;

    height = gif->SHeight;
    frame = &(gif->SavedImages[current_frame]);

    frameInfo = &(frame->ImageDesc);

    if (frameInfo->ColorMap) {

        colorMap = frameInfo->ColorMap;

    } else {

        colorMap = gif->SColorMap;

    }


    bg = &colorMap->Colors[gif->SBackGroundColor];


    for (j = 0; j < frame->ExtensionBlockCount; j++) {

        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {

            ext = &(frame->ExtensionBlocks[j]);

            break;

        }

    }
    // For dispose = 1, we assume its been drawn
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose_1 && current_frame > 0) {
        current_frame = current_frame - 1,
                drawFrame(gif, info, pixels, true);
    } else if (ext && dispose(ext) == 2 && bg) {

        for (y = 0; y < height; y++) {

            line = (int *) px;

            for (x = 0; x < width; x++) {

                line[x] = argb(255, bg->Red, bg->Green, bg->Blue);

            }

            px = (int *) ((char *) px + info.stride);

        }

    } else if (ext && dispose(ext) == 3 && current_frame > 1) {
        current_frame = current_frame - 2,
                drawFrame(gif, info, pixels, true);

    }
    px = pixels;
    if (frameInfo->Interlace) {

        n = 0;

        inc = 8;

        p = 0;

        px = (int *) ((char *) px + info.stride * frameInfo->Top);

        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {

            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {

                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }


                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg
                                                                            : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char *) px + info.stride * inc);

            n += inc;

            if (n >= frameInfo->Height) {

                n = 0;

                switch (p) {

                    case 0:

                        px = (int *) ((char *) pixels + info.stride * (4 + frameInfo->Top));

                        inc = 8;

                        p++;

                        break;

                    case 1:

                        px = (int *) ((char *) pixels + info.stride * (2 + frameInfo->Top));

                        inc = 4;

                        p++;

                        break;

                    case 2:

                        px = (int *) ((char *) pixels + info.stride * (1 + frameInfo->Top));

                        inc = 2;

                        p++;

                }

            }

        }

    } else {

        px = (int *) ((char *) px + info.stride * frameInfo->Top);

        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {

            line = (int *) px;

            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {

                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }

                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg
                                                                            : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char *) px + info.stride);

        }
    }
    GraphicsControlBlock gcb;//获取控制信息
    DGifSavedExtensionToGCB(gif, current_frame, &gcb);
    int delay = gcb.DelayTime * 10;
    LOGD("delay %d", delay);
    return delay;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_amusia_gifframework_GifLoader_getWidth(JNIEnv *env, jclass clazz, jlong point) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(point);
    return gifFileType->SWidth;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_amusia_gifframework_GifLoader_getHeight(JNIEnv *env, jclass clazz, jlong point) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(point);
    return gifFileType->SHeight;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_amusia_gifframework_GifLoader_updateFrame(JNIEnv *env, jclass clazz, jlong point,
                                                   jobject bitmap) {

    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(point);
//
    AndroidBitmapInfo androidBitmapInfo;
    AndroidBitmap_getInfo(env, bitmap, &androidBitmapInfo);

    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
//
    //绘制
    int delay = drawFrame(gifFileType, androidBitmapInfo, pixels, false);

    AndroidBitmap_unlockPixels(env, bitmap);
//
//    //切换到下一帧
//    GifBean *gifBean = static_cast<GifBean *>(*gifFileType->UserData);
    current_frame++;
    if (current_frame >= total_frame - 1) {
        current_frame = 0;
    }
    return delay;
}