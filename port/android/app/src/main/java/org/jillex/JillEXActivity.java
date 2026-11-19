package org.jillex;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Looper;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Jill-EX on Android.
 *
 * <p>SDLActivity owns the surface, input and lifecycle.  This subclass only
 * installs the small bundled configuration (jill.ini, game manifests, engine
 * profiles) into the app's writable data folder on first launch.  User game
 * data (jillt/, ONESIM/) is not bundled; it is pushed to the same folder with
 * adb or copied through a file manager (see README.txt in the data folder).
 */
public class JillEXActivity extends SDLActivity {
    private static final String TAG = "JillEX";

    /** Shared, user-visible game folder. */
    private static final String DATA_ROOT = "/sdcard/JillEX";
    private static final int REQ_STORAGE = 1001;

    /** Files copied from assets/ the first time the app runs. */
    private static final String[] ASSET_DIRS = {"games", "profiles"};
    private static final String[] ASSET_FILES = {"jill.ini", "README.txt", "DISCLAIMER.txt"};

    private static JillEXActivity sInstance;
    private static volatile boolean sStartupErrorPending = false;
    private boolean mStartupError = false;
    private boolean mWaitingForStorage = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        sInstance = this;
        sStartupErrorPending = false;
        mStartupError = false;
        Log.i(TAG, "onCreate: device=" + android.os.Build.DEVICE
                + " model=" + android.os.Build.MODEL
                + " sdk=" + android.os.Build.VERSION.SDK_INT
                + " abi=" + android.os.Build.SUPPORTED_ABIS[0]);
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate: SDL activity ready");

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        hideSystemUi();

        if (!hasStorageAccess()) {
            Log.w(TAG, "storage access missing for " + DATA_ROOT);
            mWaitingForStorage = true;
            showStorageScreen();
            return;
        }
        File dataDir = dataDirectory();
        Log.i(TAG, "onCreate: dataDir=" + dataDir);
        if (dataDir != null) {
            try {
                installAssets(dataDir);
            } catch (Throwable t) {
                Log.e(TAG, "asset install failed", t);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mWaitingForStorage && hasStorageAccess()) {
            Log.i(TAG, "storage access granted; restarting");
            mWaitingForStorage = false;
            recreate();
        }
    }

    private boolean hasStorageAccess() {
        if (Build.VERSION.SDK_INT >= 30) {
            return Environment.isExternalStorageManager();
        }
        return checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED;
    }

    /**
     * /sdcard/JillEX is outside the app sandbox.  API 30+ needs the special
     * "All files access" grant; older builds use the legacy storage
     * permission.  The screen blocks SDL startup (the surface view is
     * replaced) until the grant is present.
     */
    private void showStorageScreen() {
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setBackgroundColor(0xFF101010);
        layout.setPadding(48, 32, 48, 32);

        TextView text = new TextView(this);
        text.setTextColor(0xFFE8E8E8);
        text.setTextSize(16);
        text.setText("Jill-EX uses this folder for its game data:\n\n"
                + DATA_ROOT + "\n\n"
                + "Android needs storage access to read and write it.  Tap GRANT "
                + "ACCESS and allow 'All files access' (or 'Files and media'), "
                + "then return here.\n\n"
                + "With adb you can grant it without the settings page:\n\n"
                + "adb shell appops set org.jillex MANAGE_EXTERNAL_STORAGE allow\n\n"
                + "Game data is not bundled: push jillt/ and ONESIM/ into the "
                + "folder after granting access.");

        Button grant = new Button(this);
        grant.setText("GRANT ACCESS");
        grant.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                requestStorageAccess();
            }
        });

        layout.addView(text);
        layout.addView(grant);

        ScrollView scroll = new ScrollView(this);
        scroll.setBackgroundColor(0xFF101010);
        scroll.addView(layout);
        setContentView(scroll);
    }

    private void requestStorageAccess() {
        if (Build.VERSION.SDK_INT >= 30) {
            try {
                Intent intent = new Intent(
                        Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                        Uri.parse("package:" + getPackageName()));
                startActivity(intent);
            } catch (Exception e) {
                try {
                    startActivity(new Intent(
                            Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION));
                } catch (Exception e2) {
                    Log.e(TAG, "cannot open all-files settings", e2);
                }
            }
        } else {
            requestPermissions(
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    REQ_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
                                           int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQ_STORAGE && hasStorageAccess()) {
            mWaitingForStorage = false;
            recreate();
        }
    }

    /**
     * Called from native code (JNI) when SDL_main returns a non-zero status,
     * e.g. missing/corrupt game files.  Keeps the activity alive and replaces
     * the game surface with a readable message so the failure is visible
     * on-device instead of bouncing back to the launcher.
     */
    public static void showStartupError(final String message) {
        // Set this synchronously: SDLActivity calls finish() from the SDL
        // thread as soon as SDL_main returns, which can be before the posted
        // UI runnable below has had a chance to run.
        sStartupErrorPending = true;
        final JillEXActivity activity = sInstance;
        Log.e(TAG, "startup error: " + message);
        if (activity == null) {
            return;
        }
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                activity.showErrorScreen(message);
            }
        });
    }

    private void showErrorScreen(String message) {
        mStartupError = true;

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setBackgroundColor(0xFF101010);
        layout.setPadding(48, 32, 48, 32);

        TextView text = new TextView(this);
        text.setTextColor(0xFFE8E8E8);
        text.setTextSize(16);
        text.setText("Jill-EX could not start.\n\n"
                + message + "\n\n"
                + "Game data is not bundled.  Copy your own extracted folders to:\n"
                + DATA_ROOT + "\n\n"
                + "adb push jillt /sdcard/JillEX/\n"
                + "adb push ONESIM /sdcard/JillEX/\n\n"
                + "Then press RETRY.\n\n"
                + "Last lines of jill.log:\n"
                + readLogTail());

        Button retry = new Button(this);
        retry.setText("RETRY");
        retry.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                recreate();
            }
        });

        layout.addView(text);
        layout.addView(retry);

        ScrollView scroll = new ScrollView(this);
        scroll.setBackgroundColor(0xFF101010);
        scroll.addView(layout);
        setContentView(scroll);
    }

    /** Tail of jill.log, so the on-device error screen names the failing file. */
    private String readLogTail() {
        File log = new File(dataDirectory(), "jill.log");
        if (!log.isFile()) {
            return "(no jill.log yet)";
        }
        try (java.io.RandomAccessFile raf = new java.io.RandomAccessFile(log, "r")) {
            final int want = (int) Math.min(raf.length(), 4000);
            raf.seek(raf.length() - want);
            byte[] buffer = new byte[want];
            raf.readFully(buffer);
            return new String(buffer, "UTF-8");
        } catch (IOException e) {
            return "(cannot read jill.log: " + e + ")";
        }
    }

    @Override
    public void finish() {
        // SDLActivity.finish() runs on the SDL thread when SDL_main returns.
        // Keep the error screen up; Back/Recents (UI thread) still close it.
        if (sStartupErrorPending && Looper.myLooper() != Looper.getMainLooper()) {
            Log.i(TAG, "finish() from SDL thread ignored: startup error screen");
            return;
        }
        super.finish();
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "onDestroy: finishing=" + isFinishing());
        if (sInstance == this) {
            sInstance = null;
        }
        super.onDestroy();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUi();
        }
    }

    /**
     * Keep immersive sticky mode so the status/navigation bars do not eat the
     * letterbox area used by the touch controls.
     */
    private void hideSystemUi() {
        View decor = getWindow().getDecorView();
        decor.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    /**
     * Shared game folder.  It lives outside the app sandbox, so callers must
     * have checked hasStorageAccess() first (onCreate and the startup-error
     * screen do).
     */
    private File dataDirectory() {
        File dir = new File(DATA_ROOT);
        if (!dir.exists() && !dir.mkdirs()) {
            Log.w(TAG, "cannot create data directory " + dir);
        }
        return dir;
    }

    /**
     * Copies bundled configuration into the data folder.  Existing files are
     * left alone so user edits and saves survive app updates.  The engine also
     * writes jill.ini back on exit, so it must never be overwritten.
     */
    private void installAssets(File dataDir) {
        for (String name : ASSET_FILES) {
            copyAsset(name, new File(dataDir, name), false);
        }
        for (String dirName : ASSET_DIRS) {
            File outDir = new File(dataDir, dirName);
            if (!outDir.isDirectory() && !outDir.mkdirs()) {
                Log.w(TAG, "cannot create " + outDir);
                continue;
            }
            try {
                String[] entries = getAssets().list(dirName);
                if (entries == null) {
                    continue;
                }
                for (String entry : entries) {
                    copyAsset(dirName + "/" + entry, new File(outDir, entry), false);
                }
            } catch (IOException e) {
                Log.w(TAG, "cannot list assets/" + dirName, e);
            }
        }
        File external = new File(dataDir, "external");
        if (!external.isDirectory() && !external.mkdirs()) {
            Log.w(TAG, "cannot create " + external);
        }
    }

    private void copyAsset(String assetPath, File target, boolean overwrite) {
        if (target.exists() && !overwrite) {
            return;
        }
        try (InputStream in = getAssets().open(assetPath);
             OutputStream out = new FileOutputStream(target)) {
            byte[] buffer = new byte[16 * 1024];
            int read;
            while ((read = in.read(buffer)) > 0) {
                out.write(buffer, 0, read);
            }
        } catch (IOException e) {
            Log.w(TAG, "cannot install asset " + assetPath, e);
        }
    }
}
