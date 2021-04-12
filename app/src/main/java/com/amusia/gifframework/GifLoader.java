package com.amusia.gifframework;

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.ImageView;

import java.util.HashMap;
import java.util.Map;

public class GifLoader {
    static {
        System.loadLibrary("native-lib");
    }

    private static GifLoader instance;
    String TAG = "GifLoader";
    Bitmap bitmap;
    ImageView imageView;
    long gifPoint;
    private Handler mHandler;

    public static GifLoader getInstance() {
        if (instance == null) {
            synchronized (GifLoader.class) {
                if (instance == null) {
                    instance = new GifLoader();
                }
            }
        }
        return instance;
    }

    public GifLoader load(String path, ImageView imageView) {
        gifPoint = GifLoader.loadPath(path);
        int height = getHeight();
        int width = getWidth();
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        int delay = updateFrame(bitmap);
        this.imageView = imageView;
        imageView.setImageBitmap(bitmap);
        mHandler.sendEmptyMessageDelayed(1, delay);
        return this;
    }

    private void initHandler() {
        mHandler = new Handler(Looper.getMainLooper(), message -> {
            int delay = updateFrame(bitmap);
            mHandler.sendEmptyMessageDelayed(1, delay);
            imageView.setImageBitmap(bitmap);
            return false;
        });
    }

    private GifLoader() {
        initHandler();
    }

    public void onDestroy() {
        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
        if (bitmap != null) {
            if (!bitmap.isRecycled()) {
                bitmap.recycle();
            }
            bitmap = null;
        }
        if (imageView != null) {
            imageView = null;
        }

    }

    public int getWidth() {
        return getWidth(gifPoint);
    }

    public int getHeight() {
        return getHeight(gifPoint);
    }

    public int updateFrame(Bitmap bitmap) {
        return updateFrame(gifPoint, bitmap);
    }


    public static native long loadPath(String path);

    public static native int getWidth(long point);

    public static native int getHeight(long point);

    public static native int updateFrame(long point, Bitmap bitmap);

}
