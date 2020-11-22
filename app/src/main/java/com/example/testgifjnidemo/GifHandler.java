package com.example.testgifjnidemo;

import android.graphics.Bitmap;

public class GifHandler {

    private long gifAddr;

    public GifHandler(String path){
        this.gifAddr = loadPath(path);
    }
    public int getWidth(){
        return getWidth(gifAddr);
    }

    public int getHeight(){
        return getHeight(gifAddr);
    }

    public int updateFrame(Bitmap bitmap){
        return updateFrame(gifAddr,bitmap);
    }


    private native long loadPath(String path);

    private native int getWidth(long ndkGif);

    private native int getHeight(long ndkGif);

    private native int updateFrame(long ndkGif, Bitmap bitmap);


}
