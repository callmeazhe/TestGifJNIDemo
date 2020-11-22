package com.example.testgifjnidemo;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private Bitmap mBitmap;
    private ImageView mImageView;
    private GifHandler mGifHandler;

    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            mImageView.setImageBitmap(mBitmap);
            int nextFrameDelay = mGifHandler.updateFrame(mBitmap);
            mHandler.sendEmptyMessageDelayed(1,nextFrameDelay);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mImageView = findViewById(R.id.image);

    }


    public void jniLoadGif(View view) {
        File file = new File(Environment.getExternalStorageDirectory(),
                "demo.gif");
        mGifHandler = new GifHandler(file.getAbsolutePath());
        int width = mGifHandler.getWidth();
        int height = mGifHandler.getHeight();
        mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        //返回刷新下一帧的时间
        int nextFrameDelay = mGifHandler.updateFrame(mBitmap);
        mHandler.sendEmptyMessageDelayed(1, nextFrameDelay);
    }
}