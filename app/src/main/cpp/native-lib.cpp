#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include "gif_lib.h"

#define argb(a, r, g, b)(((a)&0xff)<<24 |((b)&0xff)<<16 | ((g)&0xff)<<8 | ((r)&0xff))

typedef struct GifBean {

    int current_frame;
    int total_frame;
    //延迟时间的数组
    int *delays;

} GifBean;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_testgifjnidemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    int *px = (int *) pixels;
    int pointPixel;
    GifImageDesc frameInfo = savedImage.ImageDesc;
    GifByteType gifByteType;//压缩数据
    ColorMapObject *colorMapObject = frameInfo.ColorMap;
    px = (int*)((char *) px + info.stride * frameInfo.Top);
    int *line;
    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {
        line = px;
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {
            pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
            gifByteType = savedImage.RasterBits[pointPixel];
            //解压
            GifColorType gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);

        }
        px = (int*)((char *) px + info.stride);
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_testgifjnidemo_GifHandler_loadPath(JNIEnv *env, jobject thiz, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    int err = 0;
    GifFileType *gifFileType = DGifOpenFileName(path, &err);
    DGifSlurp(gifFileType);

    //申请GifBean的地址
    GifBean *gifBean = static_cast<GifBean *>(malloc(sizeof(GifBean)));
    //重置下所申请的地址
    memset(gifBean, 0, sizeof(GifBean));
    //userData地址指向gifBean
    gifFileType->UserData = gifBean;
    //申请gifBean中delays数组需要的地址
    int delaysCount = sizeof(int) * gifFileType->ImageCount;
    gifBean->delays =
            static_cast<int *>(malloc(delaysCount));
    //重置下所申请的地址
    memset(gifBean->delays, 0, delaysCount);
    //为GifBean赋值
    gifBean->current_frame = 0;
    gifBean->total_frame = gifFileType->ImageCount;
    ExtensionBlock *extensionBlock;
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
        SavedImage frame = gifFileType->SavedImages[i];
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                extensionBlock = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if (extensionBlock) {
            //规则定义Delay Time的单位是 1/100s,这里 *10 代表以ms为单位
            int delayTime =
                    10 * (extensionBlock->Bytes[1] | extensionBlock->Bytes[2] << 8);
            gifBean->delays[i] = delayTime;
        }
    }
    env->ReleaseStringUTFChars(path_, path);
    return (jlong) gifFileType;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_testgifjnidemo_GifHandler_getWidth(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = (GifFileType *) ndk_gif;
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_testgifjnidemo_GifHandler_getHeight(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = (GifFileType *) ndk_gif;
    return gifFileType->SHeight;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_example_testgifjnidemo_GifHandler_updateFrame(JNIEnv *env, jobject thiz, jlong ndk_gif,
                                                       jobject bitmap) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    GifBean *gifBean = static_cast<GifBean *>(gifFileType->UserData);
    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    drawFrame(gifFileType, gifBean, bitmapInfo, pixels);
    gifBean->current_frame += 1;
    if (gifBean->current_frame >= gifBean->total_frame - 1) {
        gifBean->current_frame = 0;
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return gifBean->delays[gifBean->current_frame];
}