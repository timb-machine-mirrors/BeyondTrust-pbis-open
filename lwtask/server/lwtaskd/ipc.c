
#include "includes.h"

static
LWMsgStatus
LwTaskDaemonIpcSetLogInfo(
    LWMsgCall*         pCall,  /* IN     */
    const LWMsgParams* pIn,    /* IN     */
    LWMsgParams*       pOut,   /*    OUT */
    void*              pData   /* IN     */
    );

static
LWMsgStatus
LwTaskDaemonIpcGetLogInfo(
    LWMsgCall*         pCall,  /* IN     */
    const LWMsgParams* pIn,    /* IN     */
    LWMsgParams*       pOut,   /*    OUT */
    void*              pData   /* IN     */
    );

static
LWMsgStatus
LwTaskDaemonIpcGetPid(
    LWMsgCall*         pCall,  /* IN     */
    const LWMsgParams* pIn,    /* IN     */
    LWMsgParams*       pOut,   /*    OUT */
    void*              pData   /* IN     */
    );

static
LWMsgDispatchSpec
gLwTaskDaemonIpcDispatchSpec[] =
{
    LWMSG_DISPATCH_NONBLOCK(LW_TASK_SET_LOG_INFO, LwTaskDaemonIpcSetLogInfo),
    LWMSG_DISPATCH_NONBLOCK(LW_TASK_GET_LOG_INFO, LwTaskDaemonIpcGetLogInfo),
    LWMSG_DISPATCH_NONBLOCK(LW_TASK_GET_PID,      LwTaskDaemonIpcGetPid),
    LWMSG_DISPATCH_END
};

static
LWMsgStatus
LwTaskDaemonIpcGetPid(
    LWMsgCall*         pCall,  /* IN     */
    const LWMsgParams* pIn,    /* IN     */
    LWMsgParams*       pOut,   /*    OUT */
    void*              pData   /* IN     */
    );

DWORD
LwTaskDaemonIpcAddDispatch(
    LWMsgServer* pServer /* IN OUT */
    )
{
    return MAP_LWMSG_ERROR(lwmsg_server_add_dispatch_spec(
                                    pServer,
                                    gLwTaskDaemonIpcDispatchSpec));
}

static
LWMsgStatus
LwTaskDaemonIpcSetLogInfo(
    LWMsgCall*         pCall,  /* IN     */
    const LWMsgParams* pIn,    /* IN     */
    LWMsgParams*       pOut,   /*    OUT */
    void*              pData   /* IN     */
    )
{
    DWORD dwError = 0;
    LWMsgStatus status = LWMSG_STATUS_SUCCESS;
    PLW_TASK_STATUS_REPLY pStatusResponse = NULL;

    BAIL_ON_INVALID_POINTER(pIn->data);

    dwError = LwAllocateMemory(
                    sizeof(LW_TASK_STATUS_REPLY),
                    (PVOID*)&pStatusResponse);
    BAIL_ON_LW_TASK_ERROR(dwError);

    dwError = LwTaskLogSetInfo_r((PLW_TASK_LOG_INFO)pIn->data);

    /* Transmit failure to client but do not bail out of dispatch loop */
    if (dwError)
    {
        pStatusResponse->dwError = dwError;
        pOut->tag = LW_TASK_SET_LOG_INFO_FAILED;
        pOut->data = pStatusResponse;

        dwError = 0;
        goto cleanup;
    }

    pOut->tag = LW_TASK_SET_LOG_INFO_SUCCESS;
    pOut->data = pStatusResponse;

cleanup:

    return status;

error:

    goto cleanup;
}

static
LWMsgStatus
LwTaskDaemonIpcGetLogInfo(
    LWMsgCall*         pCall,  /* IN     */
    const LWMsgParams* pIn,    /* IN     */
    LWMsgParams*       pOut,   /*    OUT */
    void*              pData   /* IN     */
    )
{
    DWORD dwError = 0;
    LWMsgStatus status = LWMSG_STATUS_SUCCESS;
    PLW_TASK_STATUS_REPLY pStatusResponse = NULL;
    PLW_TASK_LOG_INFO pLogInfo = NULL;

    dwError = LwAllocateMemory(
                    sizeof(LW_TASK_STATUS_REPLY),
                    (PVOID*)&pStatusResponse);
    BAIL_ON_LW_TASK_ERROR(dwError);

    dwError = LwTaskLogGetInfo_r(&pLogInfo);

    if (dwError)
    {
        pStatusResponse->dwError = dwError;
        pOut->tag = LW_TASK_GET_LOG_INFO_FAILED;
        pOut->data = pStatusResponse;
        pStatusResponse = NULL;

        dwError = 0;
        goto cleanup;
    }

    pOut->tag = LW_TASK_GET_LOG_INFO_SUCCESS;
    pOut->data = pLogInfo;

cleanup:

    LW_SAFE_FREE_MEMORY(pStatusResponse);

    return status;

error:

    goto cleanup;
}

static
LWMsgStatus
LwTaskDaemonIpcGetPid(
    LWMsgCall*         pCall,  /* IN     */
    const LWMsgParams* pIn,    /* IN     */
    LWMsgParams*       pOut,   /*    OUT */
    void*              pData   /* IN     */
    )
{
    DWORD dwError = 0;
    LWMsgStatus status = LWMSG_STATUS_SUCCESS;
    pid_t* pPid = NULL;

    dwError = LwAllocateMemory(sizeof(*pPid), OUT_PPVOID(&pPid));
    BAIL_ON_LW_TASK_ERROR(dwError);

    *pPid = getpid();

    pOut->tag = LW_TASK_GET_PID_SUCCESS;
    pOut->data = pPid;
    pPid = NULL;

cleanup:

    LW_SAFE_FREE_MEMORY(pPid);

    return status;

error:

    goto cleanup;
}
