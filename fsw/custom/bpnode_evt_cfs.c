/*
 * TODO: Fill in file header, if necessary.
 */

/************************************************
 * Includes
 ************************************************/

#include <assert.h>
#include "cfe.h"
#include "bpl_evm_api.h"
#include "bpnode_evt_cfs.h"

/************************************************
 * Exported Functions
 ************************************************/

/*-----------------------------------------------
 * BPNODE_EVT_Initialize_Impl
 *-----------------------------------------------*/
BPL_Status_t BPNODE_EVT_Initialize_Impl(void)
{
    BPL_Status_t ReturnStatus = { .ReturnValue = BPL_STATUS_SUCCESS };
    OS_printf("BPNODE_EVT_Initialize_Impl called!\n");
    /* TODO: call EVS Register. */
    return ReturnStatus;
}

/*-----------------------------------------------
 * BPNODE_EVT_TranslateTypeToHost
 *-----------------------------------------------*/
uint16_t BPNODE_EVT_TranslateTypeToHost(BPL_EVM_EventType_t EventType)
{
    uint16_t HostEventType;
    switch (EventType)
    {
        case BPL_EVM_EventType_DEBUG:
            HostEventType = CFE_EVS_EventType_DEBUG;
            break;
        case BPL_EVM_EventType_INFO:
            HostEventType = CFE_EVS_EventType_INFORMATION;
            break;
        case BPL_EVM_EventType_WARNING:
            HostEventType = CFE_EVS_EventType_ERROR;
            break;
        case BPL_EVM_EventType_ERROR:
            HostEventType = CFE_EVS_EventType_ERROR;
            break;
        case BPL_EVM_EventType_CRITICAL:
            HostEventType = CFE_EVS_EventType_CRITICAL;
            break;
        default:
            /* This default case also captures the BPL_EVM_EventType_UNKNOWN case. */
            HostEventType = CFE_EVS_EventType_ERROR;
            break;
    }
    return HostEventType;
}

/*-----------------------------------------------
 * BPNODE_EVT_SendEvent_Impl
 *-----------------------------------------------*/
BPL_Status_t BPNODE_EVT_SendEvent_Impl(uint16_t EventID, BPL_EVM_EventType_t EventType,
    char const * EventText, va_list EventTextArgPtr)
{
    BPL_Status_t ReturnStatus;
    CFE_Status_t ProxyStatus;
    uint16_t HostEventType = BPNODE_EVT_TranslateTypeToHost(EventType);
    char ExpandedEventText[BPNODE_EVT_MAX_MESSAGE_LENGTH];
    int ExpandedLength;

    /*
    ** Due to how we truncate the message if its too long (as seen in code, below),
    ** we need to ensure that this buffer is at least 2 characters long.
    */
    assert(BPNODE_EVT_MAX_MESSAGE_LENGTH >= 2);
    assert(BPNODE_EVT_MAX_MESSAGE_LENGTH <= CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    memset(&ExpandedEventText, 0, sizeof(ExpandedEventText));
    ExpandedLength = vsnprintf((char *)ExpandedEventText, sizeof(ExpandedEventText), EventText, EventTextArgPtr);
    if (ExpandedLength >= (int)sizeof(ExpandedEventText))
    {
        /* Mark character before zero terminator to indicate truncation */
        ExpandedEventText[sizeof(ExpandedEventText) - 2u] = BPNODE_EVT_MSG_TRUNCATED;
        /*
        ** TODO: should we return an error here?
        ** Note: In the cFE implementation, they don't treat message truncation as an error.
        */
    }

    ProxyStatus = CFE_EVS_SendEvent(EventID, HostEventType, "%s", ExpandedEventText);

    if (ProxyStatus != CFE_SUCCESS)
    {
        OS_printf("BPNODE_EVT_SendEvent_Impl CFE_EVS_SendEvent returned error status: 0x%08X!\n",
            ProxyStatus);
        ReturnStatus.ReturnValue = BPL_STATUS_ERROR_GENERAL;
    }
    else
    {
        OS_printf("BPNODE_EVT_SendEvent_Impl called with ID 0x%08X and Type %u!\n",
            EventID, (unsigned int) EventType);
        ReturnStatus.ReturnValue = BPL_STATUS_SUCCESS;
    }

    return ReturnStatus;
}
