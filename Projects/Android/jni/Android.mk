LOCAL_PATH := $(call my-dir)

# build JNI
include $(CLEAR_VARS)

include ../../../../../cflags.mk

LOCAL_MODULE			:= ovrapp
LOCAL_SRC_FILES			:= ../../../Src/OvrApp.cpp ../../../Src/Audio/OpenSLWrap.cpp ../../../Src/LayerBuilder.cpp ../../../Src/FontMaster.cpp ../../../Src/DrawHelper.cpp ../../../Src/Emulator.cpp ../../../Src/TextureLoader.cpp

LOCAL_STATIC_LIBRARIES	:= vrsound vrmodel vrlocale vrgui vrappframework libovrkernel freetype gearboy
LOCAL_SHARED_LIBRARIES	:= vrapi

# for native audio
LOCAL_LDLIBS    += -lOpenSLES

APP_STL := gnustl_static
LOCAL_C_INCLUDES := ../../../../../FreeType ../../../../../Gearboy ../../../../../

include $(BUILD_SHARED_LIBRARY)

$(call import-module,FreeType)
$(call import-module,Gearboy/jni)
$(call import-module,LibOVRKernel/Projects/Android/jni)
$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppFramework/Projects/Android/jni)
$(call import-module,VrAppSupport/VrGUI/Projects/Android/jni)
$(call import-module,VrAppSupport/VrLocale/Projects/Android/jni)
$(call import-module,VrAppSupport/VrModel/Projects/Android/jni)
$(call import-module,VrAppSupport/VrSound/Projects/Android/jni)