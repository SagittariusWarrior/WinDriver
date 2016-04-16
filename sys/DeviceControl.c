/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    DeviceControl.c

Abstract:


Environment:

    Kernel mode

--*/

#include "precomp.h"

#include "DeviceControl.tmh"


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
VOID
PLxEvtIoDeviceControl(
	IN WDFQUEUE      Queue,
	IN WDFREQUEST    Request,
	IN size_t        OutputBufferLength,
	IN size_t        InputBufferLength,
	IN ULONG         IoControlCode
)
/*++
Routine Description:

This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
requests from the system.

Arguments:

Queue - Handle to the framework queue object that is associated
with the I/O request.
Request - Handle to a framework request object.

OutputBufferLength - length of the request's output buffer,
if an output buffer is available.
InputBufferLength - length of the request's input buffer,
if an input buffer is available.

IoControlCode - the driver-defined or system-defined I/O control code
(IOCTL) that is associated with the request.
Return Value:

VOID

--*/
{
	KdPrint(("Enter PLxEvtIoDeviceControl\n"));
    NTSTATUS             status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION    devExt = NULL;
	size_t               bytesReturnUser = 0;

    TraceEvents(TRACE_LEVEL_INFORMATION, DBG_IOCTLS,
                "--> PLxEvtIoDeviceControl: Request %p", Request);

    // Get the DevExt from the Queue handle
    devExt = PLxGetDeviceContext(WdfIoQueueGetDevice(Queue));

	// Get the inBuffer from Request
	PVOID inBuffer = NULL;
	size_t InBufferSize = 0;
	if (InputBufferLength > 0)
	{
		status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &inBuffer, &InBufferSize);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, DBG_IOCTLS, 
			                "WdfRequestRetrieveBuffer failed 0x%x\n", status);
			KdPrint(("WdfRequestRetrieveInputBuffer Failed\n"));
			WdfRequestComplete(Request, status);
			return;
		}
	}

	// Get the outBuffer from Request
	PVOID outBuffer = NULL;
	size_t OutBufferSize = 0;
	if (OutputBufferLength > 0)
	{
		status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &outBuffer, &OutBufferSize);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, DBG_IOCTLS, 
			                "WdfRequestRetrieveBuffer failed 0x%x\n", status);

			KdPrint(("WdfRequestRetrieveOutputBuffer Failed\n"));
			WdfRequestComplete(Request, status);
			return;
		}
	}

	
	//
	// Handle this request's specific code.
	//
	KdPrint(("Enter switch-case, 0x%x", IoControlCode));
	switch (IoControlCode)
	{
	case IOCTL_DMA_DEVICE_HOST:
		//for (int j = 0; j < 4096; ++j)
		//	devExt->ReadCommonBufferBase[j] = (UCHAR)j;
		// write test data to bar2(i.e. DDR2@DSP£©
		PULONG pTempBarReg = (PULONG)devExt->SRAM2Base;
		pTempBarReg[1] = 25 * 1024;
		pTempBarReg[0] = devExt->ReadCommonBufferBaseLA.LowPart;
		KdPrint(("dma read start...\n"));
		while (pTempBarReg[0] != 0);
		KdPrint(("dma read end...\n"));

		for (int i = 0; i < 100; ++i)
			KdPrint(("devExt->ReadCommonBufferBase 0x%x \n", devExt->ReadCommonBufferBase[i]));

		RtlCopyMemory(outBuffer, devExt->ReadCommonBufferBase, OutputBufferLength);
		status = STATUS_SUCCESS;
		bytesReturnUser = OutputBufferLength;
		break;

	case IOCTL_DMA_HOST_DEVICE:
		KdPrint((" - SRAM      %p, length %d\n", devExt->SRAMBase, devExt->SRAMLength));
		KdPrint(("dma write start...\n"));
		RtlZeroMemory(devExt->SRAMBase, devExt->SRAMLength);
		KdPrint(("dma write end...\n"));
		status = STATUS_SUCCESS;
		bytesReturnUser = sizeof(ULONG);
		break;

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		bytesReturnUser = sizeof(ULONG);
		break;
	}

	if (status != STATUS_PENDING)
	{
		WdfRequestCompleteWithInformation(Request, status, bytesReturnUser);
	}
	KdPrint(("Leave PLxEvtIoDeviceControl\n"));
    return;
}

VOID
PLxDeviceControlRequestComplete(
    IN WDFDMATRANSACTION  DmaTransaction,
    IN NTSTATUS           Status
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    WDFREQUEST         request;
    size_t             bytesTransferred;

    //
    // Initialize locals
    //

#ifdef ASSOC_WRITE_REQUEST_WITH_DMA_TRANSACTION

    request = WdfDmaTransactionGetRequest(DmaTransaction);

#else
    //
    // If CreateDirect was used then there will be no assoc. Request.
    //
    {
        PTRANSACTION_CONTEXT transContext = PLxGetTransactionContext(DmaTransaction);

        request = transContext->Request;
        transContext->Request = NULL;
        
    }
#endif

    //
    // Get the final bytes transferred count.
    //
    bytesTransferred =  WdfDmaTransactionGetBytesTransferred( DmaTransaction );

    TraceEvents(TRACE_LEVEL_INFORMATION, DBG_DPC,
                "PLxDeviceControlRequestComplete:  Request %p, Status %!STATUS!, "
                "bytes transferred %d\n",
                 request, Status, (int) bytesTransferred );

    WdfDmaTransactionRelease(DmaTransaction);        

    WdfRequestCompleteWithInformation( request, Status, bytesTransferred);

}

