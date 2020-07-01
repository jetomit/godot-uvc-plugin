package org.eyeskills.godot.plugin.android.uvc;

import java.util.Collections;
import java.util.List;
import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

public class UvcPlugin extends GodotPlugin {
	public UvcPlugin(Godot godot) {
		super(godot);
		System.loadLibrary("godot_uvc");
	}

	@Override
	public String getPluginName() {
		return "Uvc";
	}

	@Override
	public List<String> getPluginMethods() {
		return Collections.singletonList("helloWorld");
	}

	public String helloWorld() {
		return Integer.toString(foo());
	}

	public native int foo();
}
