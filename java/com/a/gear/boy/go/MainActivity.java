package com.a.gear.boy.go;

import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.content.Intent;

import com.oculus.vrappframework.VrActivity;

import java.io.File;

public class MainActivity extends VrActivity {
    public static final String TAG = "GearboyVR";

    /** Load jni .so on initialization */
    static {
        Log.d(TAG, "LoadLibrary");
        System.loadLibrary("ovrapp");
    }

    public static native long nativeSetAppInterface(
            VrActivity act, String fromPackageNameString, String commandString, String uriString);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        CreateFolder();

        Intent intent = getIntent();
        String commandString = VrActivity.getCommandStringFromIntent(intent);
        String fromPackageNameString = VrActivity.getPackageStringFromIntent(intent);
        String uriString = VrActivity.getUriStringFromIntent(intent);

        setAppPtr(nativeSetAppInterface(this, fromPackageNameString, commandString, uriString));
    }

    public void CreateFolder() {
        // states folder is needed to create state files
        String storageDir = Environment.getExternalStorageDirectory().toString() + "/Roms/GB/States/";
        File folder = new File(storageDir);
        if (!folder.exists()) {
            folder.mkdirs();
            Log.d("MainActivity", "created /Roms/GB/States/ Directory");
        }
    }

    public int getInt(){
        Intent batteryIntent = registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));

        int level = batteryIntent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
        int scale = batteryIntent.getIntExtra(BatteryManager.EXTRA_SCALE, 100);

        return (int)((level / (float)scale) * 100);
    }

    public String getExternalFilesDir() {
        return this.getExternalFilesDir(null).toString();
    }

    public String getInternalStorageDir() {
        return Environment.getExternalStorageDirectory().toString();
    }
}
