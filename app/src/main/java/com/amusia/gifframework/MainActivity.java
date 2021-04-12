package com.amusia.gifframework;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    String TAG = "GIFMain";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        String absolutePath = getExternalCacheDir().getAbsolutePath();
        verifyStoragePermissions(this);
        Log.d(TAG, "path: " + absolutePath);
    }

    public void loadGif(View view) {
        File file = new File(Environment.getExternalStorageDirectory(), "demo.gif");
        Log.d(TAG, "path: " + file.getAbsolutePath());
        GifLoader.getInstance().load(file.getAbsolutePath(), findViewById(R.id.iv_image));
        GifLoader.getInstance().load(file.getAbsolutePath(), findViewById(R.id.iv_image2));
    }

    public void verifyStoragePermissions(Activity activity) {
        int REQUEST_EXTERNAL_STORAGE = 1;
        String[] PERMISSIONS_STORAGE = {
                "android.permission.READ_EXTERNAL_STORAGE",
                "android.permission.WRITE_EXTERNAL_STORAGE"};
        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        GifLoader.getInstance().onDestroy();
    }
}