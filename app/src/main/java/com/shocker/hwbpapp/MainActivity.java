package com.shocker.hwbpapp;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.shocker.hwbpapp.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'hwbpapp' library on application startup.
    static {
        System.loadLibrary("hwbpapp");
    }

    private ActivityMainBinding binding;

    private boolean isClick=false;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
    }

    //点击按钮执行的方法
    public void OnClickButton(View view) {
        if (!isClick) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    isClick = true;
                    testHwBp();
                }
            }).start();
        }else {
            Toast.makeText(getApplicationContext(),"您已经点击过测试按钮,请重新打开app",Toast.LENGTH_LONG).show();
        }
    }

    /**
     * A native method that is implemented by the 'hwbpapp' native library,
     * which is packaged with this application.
     */
    public native void testHwBp();
}