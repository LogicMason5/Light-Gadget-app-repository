package com.example.lightgadget;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Typeface;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.*;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import java.text.SimpleDateFormat;
import java.util.*;

public class Dashboard extends AppCompatActivity {

    // GUI
    private TextView feedback;
    private TextView lastSelectedDevice;
    private Button sendBtn;
    private EditText text2Send;
    private ListView devicesList;
    private Animation refreshAnim;

    // Bluetooth
    private BluetoothAdapter bluetoothAdapter;
    private Set<BluetoothDevice> pairedDevices;
    private final MyBTclass bt = new MyBTclass();
    private boolean btConnected = false;

    private int refreshCounter = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dashboard);

        initViews();
        initBluetooth();
        initListeners();

        refreshPairedDevices();
    }

    /* -----------------------------
     *           INIT
     * ----------------------------- */

    private void initViews() {
        feedback = findViewById(R.id.feedback);
        feedback.setMovementMethod(new ScrollingMovementMethod());
        feedback.setText(MyBTclass.log);

        text2Send = findViewById(R.id.to_send);
        devicesList = findViewById(R.id.devices_list);
        sendBtn = findViewById(R.id.send);

        refreshAnim = AnimationUtils.loadAnimation(this, R.anim.rotate_360);

        ableSending(false);
    }

    private void initBluetooth() {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if (bluetoothAdapter == null) {
            logFeedback("ERROR: No Bluetooth adapter on this device.");
            Toast.makeText(this, "Bluetooth not supported.", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!bluetoothAdapter.isEnabled()) {
            requestEnableBluetooth();
        } else {
            logFeedback("Bluetooth is already enabled.");
        }
    }

    private void requestEnableBluetooth() {
        if (checkPermission()) {
            startActivityForResult(new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE), 1);
        } else {
            requestPermission();
        }
    }

    private void initListeners() {
        findViewById(R.id.refresh).setOnClickListener(v -> {
            refreshPairedDevices();
            v.startAnimation(refreshAnim);
        });

        findViewById(R.id.erase).setOnClickListener(v -> {
            MyBTclass.log = "";
            logFeedback("Log reset.");
        });

        findViewById(R.id.exit_dashboard).setOnClickListener(v -> goMain());

        sendBtn.setOnClickListener(v -> {
            String msg = text2Send.getText().toString();
            bt.write(msg, this);
            logFeedback("Sent: " + msg);
        });
    }

    /* -----------------------------
     *      PERMISSIONS
     * ----------------------------- */

    private boolean check
