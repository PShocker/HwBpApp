#include <jni.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "log.h"
#include "Hwbp.h"


extern "C"
JNIEXPORT void JNICALL
Java_com_shocker_hwbpapp_MainActivity_testHwBp(JNIEnv *env, jobject thiz) {
    // TODO: implement testHwBp()

    int i=0;
    int ret;
    pthread_t ntid;
    LOGD("正在创建线程");
    if ((ret =pthread_create(&ntid, nullptr, setHwBreakPoint, (void *)&i)))
    {
        LOGD("can't create thread: %s\n", strerror(ret));
    }
//    LOGD("当前函数地址:%p",Java_com_shocker_hwbpapp_MainActivity_testHwBp);
    while (i<100){
        i+=1;
        sleep(1);
    }
}