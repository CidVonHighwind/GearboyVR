package com.a.gear.boy.go;

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

    public String getExternalFilesDir() {
        return this.getExternalFilesDir(null).toString();
    }

    public String getInternalStorageDir() {
        return Environment.getExternalStorageDirectory().toString();
    }
}
