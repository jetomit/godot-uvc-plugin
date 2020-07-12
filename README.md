# Android UVC plugin for Godot

[This plugin does not work yet.]

Add support for UVC cameras to Godot’s CameraServer, based on libuvc. On Android, a Java wrapper takes care of requesting permissions and opening the device.

## Compiling

Clone the project and its dependencies:

    git clone --recurse-submodules <godot-uvc-plugin repo>

Build the plugin:

    export ANDROID_SDK_ROOT=/path/to/SDK
    cd libs
    wget https://downloads.tuxfamily.org/godotengine/3.2.2/godot-lib.3.2.2.stable.release.aar
    cd godot-cpp
    cmake .
    cd ../..
    ./gradlew build

The plugin should be in `build/outputs/aar/`.

## Demo

Generate a signing key for debug packages:

    keytool -keyalg RSA -genkeypair -dname "CN=Android Debug,O=Android,C=US" \
        -validity 9999 -deststoretype pkcs12 -keystore debug.keystore \
        -alias android -keypass android -storepass android 

Set the path and password in `export_presets.cfg`:

    keystore/debug="/path/to/debug.keystore"
    keystore/debug_user="android"
    keystore/debug_password="android"

Import the project into Godot, install [Android build templates](https://downloads.tuxfamily.org/godotengine/3.2.2/Godot_v3.2.2-stable_export_templates.tpz) from the Project menu, and export to get an APK.
