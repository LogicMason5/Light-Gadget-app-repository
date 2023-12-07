package com.example.lightgadget;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.pm.PackageManager;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class MyBTclass extends Thread {
    static BluetoothSocket bluetoothSocket = null;
    static BluetoothServerSocket bluetoothServerSocket = null;
    static InputStream inputStream = null;
    static OutputStream outputStream = null;
    static final UUID mUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    String readMessage = "default";
    int maxCommandLength = 100;
    static int[] stripsNum;
    static String[] stripsName;
    static String log = ">> ... \n";

    static String connected_MAC = null;

    // Connect to a specific MAC adress.
    public boolean connect(Context context, BluetoothAdapter bluetoothAdapter, String MAC) {
        // Check if bluetooth is available.
        if (bluetoothAdapter != null && ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED) {
            if (bluetoothSocket != null) disconnect(context);
            if (bluetoothAdapter.isEnabled() && bluetoothSocket == null) { // BluetoothAdapter is ON.
                // Create a bluetooth device by its MAC adress.
                BluetoothDevice btDevice = null;
                try {
                    btDevice = bluetoothAdapter.getRemoteDevice(MAC);
                } catch (Exception e) {
                    Toast.makeText(context, "ERROR: unavailable MAC address.", Toast.LENGTH_SHORT).show();
                }

                // Create the socket.
                try {
                    bluetoothSocket = btDevice.createRfcommSocketToServiceRecord(mUUID);
                } catch (IOException e) {
                    Toast.makeText(context, "ERROR: creating socket.", Toast.LENGTH_SHORT).show();
                }

                // Create the server socket.
                try {
                    bluetoothServerSocket = bluetoothAdapter.listenUsingInsecureRfcommWithServiceRecord("BTSS", mUUID);
                } catch (IOException e) {
                    Toast.makeText(context, "ERROR: creating server socket.", Toast.LENGTH_SHORT).show();
                }

                // Connect the socket.
                try {
                    bluetoothSocket.connect();
                } catch (IOException e) {
                    Toast.makeText(context, "ERROR: connecting socket.", Toast.LENGTH_SHORT).show();
                }

                // Get Streams to manage the communication.
                if (bluetoothSocket.isConnected()) {
                    try {
                        inputStream = bluetoothSocket.getInputStream();
                        outputStream = bluetoothSocket.getOutputStream();
                    } catch (IOException e) {
                        Toast.makeText(context, "ERROR: getting Streams.", Toast.LENGTH_SHORT).show();
                    }

                    connected_MAC = MAC;
                    return true;
                }
                else {
                    Toast.makeText(context, "Bluetooth socket not connected properly.", Toast.LENGTH_SHORT).show();
                }
            }
            else {
                Toast.makeText(context, "BluetoothAdapter is OFF.", Toast.LENGTH_SHORT).show();
            }
        }
        else {
            Toast.makeText(context, "This device do not support Bluetooth.", Toast.LENGTH_SHORT).show();
        }
        return false;
    }

    // disconnect BT communication.
    public boolean disconnect(Context context) {
        try {
            bluetoothSocket.close();
            connected_MAC = null;
        } catch (IOException e) {
//            Toast.makeText(context, "ERROR: socket not closed.", Toast.LENGTH_SHORT).show();
            return false;
        }

        try {
            bluetoothServerSocket.close();
        } catch (IOException e) {
//            Toast.makeText(context, "ERROR: server socket not closed.", Toast.LENGTH_SHORT).show();
            return false;
        }

        bluetoothSocket = null;
        bluetoothServerSocket = null;

//        Toast.makeText(context, "BT disconnected.", Toast.LENGTH_SHORT).show();

        return true;
    }

    // Send data by BT.
    public void write(String input, Context context) {
        // Check if there is an available BT connection and tryu to send the message.
        if (bluetoothSocket != null) {
            if (bluetoothSocket.isConnected()) {
                try {
                    inputStream.skip(inputStream.available());  // Flush buffer to get a clean answer.
                    outputStream.write(input.getBytes());
                }
                catch (IOException e) {
//                    Toast.makeText(context, "ERROR: sending: " + input, Toast.LENGTH_SHORT).show();
                }
            }
            else {
//                Toast.makeText(context, "ERROR: sending: socket is not connected.", Toast.LENGTH_SHORT).show();
            }
        }
        else {
//            Toast.makeText(context, "ERROR: sending: socket is not defined.", Toast.LENGTH_SHORT).show();
        }
    }

    // Receive data by BT.
    public String read(Context context) {
        readMessage = "default";
        if (bluetoothSocket != null) {
            readMessage = "";
            byte b;
            char c;
            if (bluetoothSocket.isConnected()) {
                for (int i=0; i < maxCommandLength; i++) {
                    try {
                        b = (byte) inputStream.read();
                        c = (char) b;
                        if (c == '#') break;
                        else readMessage += c;
                    }
                    catch (IOException e) {
//                        Toast.makeText(context, "ERROR: reading input stream.", Toast.LENGTH_SHORT).show();
                    }
                }
            }
            else {
//                Toast.makeText(context, "ERROR: reading: socket is not connected.", Toast.LENGTH_SHORT).show();
            }
        }
        else {
//            Toast.makeText(context, "ERROR: reading: socket is not defined.", Toast.LENGTH_SHORT).show();
        }
        return readMessage;
    }

    // Parse the received strips info.
    public void parseStrips(@NonNull String strips) {
        strips = strips.replace("#","");
        String[] stripsSplit = strips.split(";");
        stripsName = new String[stripsSplit.length];
        stripsNum = new int[stripsSplit.length];
        for (int i=0; i<stripsSplit.length; i++) {
            String[] strip = stripsSplit[i].split(":");
            stripsName[i] = strip[0];
            stripsNum[i] = Integer.parseInt(strip[1]);
        }
    }

    // Returns the current BT socket.
    public BluetoothSocket getBTSocket() {
        return bluetoothSocket;
    }

    public String getBTMAC() {
        return connected_MAC;
    }

    // Returns the current strips names array.
    public String[] getNames() {
        return stripsName;
    }
    
    // Returns the current strips nums array.
    public int[] getNums() {
        return stripsNum;
    }
}

