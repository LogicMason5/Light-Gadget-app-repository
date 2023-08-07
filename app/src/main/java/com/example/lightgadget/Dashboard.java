package com.example.lightgadget;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Typeface;
import android.graphics.drawable.AnimationDrawable;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.nightonke.boommenu.BoomButtons.BoomButton;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Objects;
import java.util.Set;

public class Dashboard extends AppCompatActivity {
    // GUI objects.
    RelativeLayout relativeLayout;
    Button exitBtn, refreshBtn, sendBtn, eraseBtn;
    TextView feedback, lastTv;
    com.google.android.material.textfield.TextInputEditText text2Send;
    ListView devicesList;
    int scrollAmount = 0;

    // Animations.
    Animation refresh_rev;

    // Bluetooth related.
    public MyBTclass bt = new MyBTclass();
    boolean btConnected = false;
    Set<BluetoothDevice> pairedDevices;
    BluetoothAdapter bluetoothAdapter;

    int refresh_counter; // Counter for the refresh function to know the first iteration to show special feedback.

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dashboard);

        relativeLayout = findViewById(R.id.layout_Dashboard);

        if (lastTv != null) {
            lastTv.setTypeface(null, Typeface.BOLD);
        }

        // GUI definitions.
        refreshBtn = findViewById(R.id.refresh);
        eraseBtn = findViewById(R.id.erase);
        exitBtn = findViewById(R.id.exit_dashboard);
        sendBtn = findViewById(R.id.send);
        feedback = findViewById(R.id.feedback);
        feedback.setMovementMethod(new ScrollingMovementMethod());
        feedback.setText(MyBTclass.log);
        text2Send = findViewById(R.id.to_send);
        devicesList = findViewById(R.id.devices_list);

        // Animations
        refresh_rev = AnimationUtils.loadAnimation(this, R.anim.rotate_360);

        // Create the bluetooth adapter if required.
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            logFeedback("ERROR: Bluetooth adapter is not available. This device may not support Bluetooth.");
            Toast.makeText(getBaseContext(), "ERROR: Bluetooth adapter is not available.", Toast.LENGTH_SHORT).show();
        } else {
            if (bluetoothAdapter.isEnabled()) { // Is turned ON.
                logFeedback("Bluetooth is already avilable.");
            } else if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED) { // Is turned OFF.
                // Ask user to activate Bluetooth.
                Intent intent_BT = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(intent_BT, 1);
            }
        }

        // Buttons callbacks.
        sendBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //Toast.makeText(getBaseContext(), "Trying to send...", Toast.LENGTH_SHORT).show();
                bt.write(Objects.requireNonNull(text2Send.getText()).toString(), getBaseContext());
                logFeedback("Sent: " + Objects.requireNonNull(text2Send.getText()));
            }
        });

        refreshBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                refreshPairedDevices();
                refreshBtn.startAnimation(refresh_rev);
                if (bt.getBTSocket() != null) {
                    bt.disconnect(getBaseContext());
                }
            }
        });

        eraseBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // Print the date on the log.
                SimpleDateFormat simpleDateFormat = new SimpleDateFormat("HH:mm:ss"); // dd/MM/yyyy - HH:mm:ss
                Date d = Calendar.getInstance().getTime();
                String timestamp = simpleDateFormat.format(d);
                MyBTclass.log = "";
                logFeedback("Log reset.");
            }
        });

        exitBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                goMain();
            }
        });

        refreshPairedDevices();
    }

    // Back button callback.
    public void onBackPressed() {
        super.onBackPressed();
        goMain();
    }
    
    private void goMain() {
        Intent intent = new Intent(Dashboard.this, Main.class);
        startActivity(intent);
        overridePendingTransition(R.anim.down_animation, R.anim.up_animation);
        finish();
    }

    // Function to refresh the paired devices.
    public void refreshPairedDevices() {
        refresh_counter += 1;
        // Initialize Array adapter that requires a .xml file (new "Layout resource file" in the "layout" directory) with a single TextView.
        ArrayAdapter devicesArray = new ArrayAdapter(Dashboard.this, R.layout.devices_tv);

        // Initialize listView.
        devicesList.setAdapter(devicesArray);
        devicesList.setOnItemClickListener(mDeviceClickListener);

        // Request required permissions.
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions( this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, 100);
        }

        // Get the linked BT devices.
        pairedDevices = bluetoothAdapter.getBondedDevices();

        // Set the linked BT devices to the listView.
        if (pairedDevices.size() > 0)
        {
            for (BluetoothDevice device: pairedDevices) {
                devicesArray.add(" " + device.getName() + "\n  " + device.getAddress());
            }
            logFeedback("Paired devices refreshed.");
        }
    }

    // List item click callback.
    final AdapterView.OnItemClickListener mDeviceClickListener = new AdapterView.OnItemClickListener() {
        public void onItemClick(AdapterView av, View v, int arg2, long arg3) {
            // Get MAC adress from device (last 17 characters of the item from the listView).
            String info = ((TextView) v).getText().toString();
            String name = info.substring(0, info.length() - 20);
            Toast.makeText(getBaseContext(), "Connecting to " + name, Toast.LENGTH_SHORT).show();

            String MAC = info.substring(info.length() - 17);
            logFeedback("Selected device: " + name);
            btConnected = bt.connect(getBaseContext(), bluetoothAdapter, MAC);
            logFeedback("btConnected: " + btConnected);
            if (btConnected) {
                logFeedback("Established connection to "+name);

                // Set device name bold in list.
                if (lastTv == null) {
                    lastTv = (TextView) v;
                    lastTv.setTypeface(null, Typeface.BOLD);
                }
                else {
                    lastTv.setTypeface(null, Typeface.NORMAL);
                    lastTv = (TextView) v;
                    lastTv.setTypeface(null, Typeface.BOLD);
                }
                bt.write("0", getBaseContext());
                String answer = bt.read(getBaseContext());
                logFeedback("Received: " + answer);
                bt.parseStrips(answer);
            }
            else {
                logFeedback("ERROR: connecting Bluetooth.");
                Toast.makeText(getBaseContext(), "ERROR: connecting Bluetooth.", Toast.LENGTH_SHORT).show();
            }
        }
    };

    // Append the feedback to its own tv and scrolls to end.
    public void logFeedback(String msg) {
        // Print the date on the log.
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("HH:mm:ss"); // dd/MM/yyyy - HH:mm:ss
        Date d = Calendar.getInstance().getTime();
        String timestamp = simpleDateFormat.format(d);
        MyBTclass.log += timestamp + "> " + msg + "\n";
        feedback.setText(MyBTclass.log);

        try {
        scrollAmount = feedback.getLayout().getLineTop(feedback.getLineCount()) - feedback.getHeight();
        }
        catch (Exception ignored) {}

        feedback.scrollTo(0, Math.max(scrollAmount, 0));
    }
}
