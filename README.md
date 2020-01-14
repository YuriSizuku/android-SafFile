# AndroidSafFile
A wrapper of Android Storage Access Framework for writing files on Ext SD card in both Java and JNI native code.  

Use DocumentFile for Java code to write,  
and [xhook](https://github.com/iqiyi/xHook) to hook native code (such as fopen, mkdir, remove),  
so that the native code can use SAF to write file.  

## [Java usage]  
See `SafFile.java' in detail.  
1. Use `requestDocUri` to get the `docUri`,
   and use `saveDocUriFromResult` in `onActivityResult` to save the `docUri` in `SharedPreference` 
2. You can write the file on EXT sdcard by `writeFileSaf` as well as the other Saf fucntions.

## [JNI Usage]  
1. Use `build.cmd` to build `libyurihook.so`, then put `libyurihook.so` and `libxhook.so` to your app's lib  
2. Load the `libyurihook.so` in your app  
3. Use`nativeInitSafJavaCallbacks()` and  `nativeHookFile(String hooksoStr, String soPath)`  
   for jni file hook.  
   `hooksoStr`, which passed to `xhook_register`, can use regex to match the hooked path(all of them are hooked), such as ".*\\.so$"  
   `soPath`, if not null,  it will dlopen that path before hook  
