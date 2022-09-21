//
// Created by Shocker on 2022/9/20.
//

#ifndef HWBPAPP_HWBP_H
#define HWBPAPP_HWBP_H
#include <dirent.h>
#include "HwBreakpointManager.h"
#include "log.h"

BOOL GetProcessTask(int pid, std::vector<int> &vOutput)
{
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    char szTaskPath[256] = {0};
    sprintf(szTaskPath, "/proc/%d/task", pid);

    dir = opendir(szTaskPath);
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) // 循环读取路径下的每一个文件/文件夹
        {
            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
            {
                continue;
            }
            else if (ptr->d_type != DT_DIR)
            {
                continue;
            }
            else if (strspn(ptr->d_name, "1234567890") != strlen(ptr->d_name))
            {
                continue;
            }

            int task = atoi(ptr->d_name);
            vOutput.push_back(task);
        }
        closedir(dir);
        return TRUE;
    }
    return FALSE;
}


void *setHwBreakPoint(void *address)
{
    CHwBreakpointManager driver;
    //连接驱动
    int err = 0;
    if (!driver.ConnectDriver(err))
    {
        LOGD("连接驱动失败\n");
        return nullptr;
    }

    //获取CPU支持硬件执行和访问断点的数量
    int bprsCount = driver.GetNumBRPS();
    int wprsCount = driver.GetNumWRPS();
    LOGD("驱动_获取CPU支持硬件执行断点的数量:%d\n", bprsCount);
    LOGD("驱动_获取CPU支持硬件访问断点的数量:%d\n", wprsCount);

    //获取当前进程所有的task
    std::vector<int> vTask;
    GetProcessTask(getpid(), vTask);
    if (vTask.size() == 0)
    {
        LOGD("获取当前进程task失败\n");
        return nullptr;
    }

    //设置进程硬件断点
    std::vector<uint64_t> vHwBpHandle;

    for (int task : vTask)
    {
        //打开task
        uint64_t hProcess = driver.OpenProcess(task);
        // LOGD("调用驱动 OpenProcess(%d) 返回值:%lu", task, hProcess);
        if (!hProcess)
        {
            LOGD("调用驱动 OpenProcess 失败\n");
            fflush(stdout);
            continue;
        }

        //设置进程硬件断点
        uint64_t hwBpHandle = driver.AddProcessHwBp(hProcess, (uint64_t)address,
                                                    HW_BREAKPOINT_LEN_4, HW_BREAKPOINT_R);
        // LOGD("调用驱动 AddProcessHwBp(%lx) 返回值:%lu", address, hwBpHandle);

        if (hwBpHandle)
        {
            vHwBpHandle.push_back(hwBpHandle);
        }
        //关闭task
        driver.CloseHandle(hProcess);
        // LOGD("调用驱动 CloseHandle:%lu", hProcess);
    }
    LOGD("请等待2秒");
    sleep(2);
    // printf("==========================================================================\n");
    //删除进程硬件断点
    for (uint64_t hwBpHandle : vHwBpHandle)
    {
        driver.DelProcessHwBp(hwBpHandle);
        // printf("调用驱动 DelProcessHwBp(%" PRIu64 ")\n", hwBpHandle);
    }
    //读取硬件断点命中信息
    for (uint64_t hwBpHandle : vHwBpHandle)
    {
        std::vector<USER_HIT_INFO> vHit;
        BOOL b = driver.ReadHwBpInfo(hwBpHandle, vHit);
//         printf("==========================================================================\n");
//         printf("调用驱动 ReadProcessHwBp(%" PRIu64 ") 返回值:%d\n", hwBpHandle, b);
        for (USER_HIT_INFO userhInfo : vHit)
        {
            LOGD("==========================================================================\n");
            LOGD("硬件断点命中地址:%p,命中次数:%zu\n", userhInfo.hit_addr, userhInfo.hit_count);
            for (int r = 0; r < 30; r += 5)
            {
                LOGD("\tX%-2d=%-12llx X%-2d=%-12llx X%-2d=%-12llx X%-2d=%-12llx X%-2d=%-12llx\n",
                     r, userhInfo.regs.regs[r],
                     r + 1, userhInfo.regs.regs[r + 1],
                     r + 2, userhInfo.regs.regs[r + 2],
                     r + 3, userhInfo.regs.regs[r + 3],
                     r + 4, userhInfo.regs.regs[r + 4]);
            }
            LOGD("\tLR=%-12llx SP=%-12llx PC=%-12llx\n",
                 userhInfo.regs.regs[30],
                 userhInfo.regs.sp,
                 userhInfo.regs.pc);

            LOGD("\tprocess status:%-12llx orig_x0:%-12llx syscallno:%-12llx\n",
                 userhInfo.regs.pstate,
                 userhInfo.regs.orig_x0,
                 userhInfo.regs.syscallno);
            LOGD("==========================================================================\n");
        }
    }

    //清空硬件断点命中信息
    driver.CleanHwBpInfo();
    LOGD("调用驱动 CleanHwBpInfo()\n");
    return nullptr;
}

#endif //HWBPAPP_HWBP_H
