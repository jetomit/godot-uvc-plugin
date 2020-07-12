package org.eyeskills.godot.plugin.android.uvc;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;
import android.view.View;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;
import org.godotengine.godot.plugin.SignalInfo;

public class UvcPlugin extends GodotPlugin {
	public UvcPlugin(Godot godot) {
		super(godot);
		//System.loadLibrary("godot_uvc");

		usbDevices = new HashMap<Integer, UsbDeviceConnection>();

		Activity activity = getActivity();
		usbManager = (UsbManager)activity.getSystemService(Context.USB_SERVICE);
		IntentFilter filter = new IntentFilter("com.android.example.USB_PERMISSION");
		filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
		filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);

		Log.i("godot/usb", "registerReceiver");
		activity.registerReceiver(usbReceiver, filter);
	}

	@Override
	public String getPluginName() {
		return "Uvc";
	}

	@Override
	public List<String> getPluginMethods() {
		return Collections.singletonList("helloWorld");
	}

	@Override
	public Set<SignalInfo> getPluginSignals() {
		return new HashSet<SignalInfo>(Arrays.asList(connected, disconnected));
	}

	@Override
	public Set<String> getPluginGDNativeLibrariesPaths() {
		return Collections.singleton("godot/plugin/v1/Uvc/uvc-server.gdnlib");
	}

	public String helloWorld() {
		Log.i("godot/usb", "calling hello… ");
		//emitSignal("camera_connected", new Integer(42));
		return Integer.toString(43);
	}

	// XXX this gets called twice on ATTACHED/PERMISSION, why?
	// https://stackoverflow.com/questions/15957509/compile-and-link-against-libusb-for-android
	// https://stackoverflow.com/questions/22197425/low-level-usb-api-on-android
	private final BroadcastReceiver usbReceiver =
		new BroadcastReceiver() {
			public void onReceive(Context context, Intent intent) {
				String action = intent.getAction();
				Log.i("godot/usb", "BroadcastReceiver onReceive, action=" + action);

				if (action.equals(UsbManager.ACTION_USB_DEVICE_ATTACHED)) {
					UsbDevice device =
						(UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
					synchronized (this) {
					if (device != null) {
						String name = device.getDeviceName();
						if (usbManager.hasPermission(device)) {
							Log.i("godot/usb", "Already have permission for usb device " + name);
							connect(device);
						} else {
							Log.i("godot/usb", "Requesting permission for usb device: " + name);
							PendingIntent permissionIntent =
								PendingIntent.getBroadcast(getActivity(), 0, new Intent("com.android.example.USB_PERMISSION"), 0);
							usbManager.requestPermission(device, permissionIntent);
						}
					}
					}

				} else if (action.equals(UsbManager.ACTION_USB_DEVICE_DETACHED)) {
					UsbDevice device =
						(UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
					//synchronized (this) {
					if (device != null) {
						String name = device.getDeviceName();
						Log.i("godot/usb", "Detached usb device " + name + "(id=" + Integer.toString(device.getDeviceId()) + ")");
						UsbDeviceConnection connection = usbDevices.get(device.getDeviceId());
						if (connection != null) {
							int fd = connection.getFileDescriptor();
							usbDevices.remove(device.getDeviceId());
							//detached(fd);
							emitSignal("disconnected", new Integer(fd));
						}
					}
					//}

				} else if (action.equals("com.android.example.USB_PERMISSION")) {
					//synchronized (this) {
					UsbDevice device =
						(UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
					if (device != null) {
						if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
							connect(device);
						} else {
							Log.i("godot/usb", "permission denied for device " + device.getDeviceName());
						}
					}
					//}
				}
			}

			private void connect(UsbDevice device) {
				String name = device.getDeviceName();
				Log.i("godot/usb", "Connecting to usb device " + name + "(id=" + Integer.toString(device.getDeviceId()) + ", class=" + Integer.toString(device.getDeviceClass()) + ")");
				UsbDeviceConnection connection = usbManager.openDevice(device);
				if (connection != null) {
					//connection.claimInterface(device.getInterface(0), true);
					usbDevices.put(device.getDeviceId(), connection);
					final int fd = connection.getFileDescriptor();
					Log.i("godot/usb", "Connected to device " + name + ", fd=" + Integer.toString(fd));
					//attached(fd);
					emitSignal("connected", new Integer(fd));
					Log.i("godot/usb", "Emitted signal for device " + name + ", fd=" + Integer.toString(fd));
				} else {
					Log.e("godot/usb", "Could not connect to device " + name);
				}
			}
		};

//	public native boolean init();
//	public native void finish();
//	public native boolean attached(int fd);
//	public native void detached(int fd);

	private SignalInfo connected = new SignalInfo("connected", Integer.class);
	private SignalInfo disconnected = new SignalInfo("disconnected", Integer.class);
	private UsbManager usbManager;
	private HashMap<Integer, UsbDeviceConnection> usbDevices;
}
