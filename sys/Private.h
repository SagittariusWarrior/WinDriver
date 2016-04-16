/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Private.h

Abstract:

Environment:

    Kernel mode

--*/


#if !defined(_PCI9656_H_)
#define _PCI9659_H_




/* PCIE registers */
#define PCIE_BASE_ADDRESS            0x21800000
#define OB_SIZE                      0x30
#define PRIORITY                     0x3C
#define EP_IRQ_CLR                   0x68
#define EP_IRQ_STATUS                0x6C
#define LEGACY_A_IRQ_STATUS_RAW      0x180
#define LEGACY_A_IRQ_ENABLE_SET      0x188
#define LEGACY_A_IRQ_ENABLE_CLR      0x18C
#define OB_OFFSET_INDEX(n)           (0x200 + (8 * (n)))
#define OB_OFFSET_HI(n)              (0x204 + (8 * (n)))
#define IB_BAR(n)                    (0x300 + (0x10 * (n)))
#define IB_START_LO(n)               (0x304 + (0x10 * (n)))
#define IB_START_HI(n)               (0x308 + (0x10 * (n)))
#define IB_OFFSET(n)                 (0x30C + (0x10 * (n)))


#define LL2_START                    0x00800000
#define MSMC_START                   0x0C000000  /* Shared L2 */
#define DDR_START                    0x80000000
#define PCIE_DATA                    0x60000000  

/* For 1MB outbound translation window size */
#define PCIE_ADLEN_1MB               0x00100000
#define PCIE_1MB_BITMASK             0xFFF00000



//
// The device extension for the device object
//
typedef struct _DEVICE_EXTENSION {

    WDFDEVICE               Device;

    // Following fields are specific to the hardware
    // Configuration


	// DSP C665x BAR0 registers
	PBAR0_REGS			    Bar0Regs;



    // HW Resources
    PPCI9656_REGS           Regs;             // Registers address
    PUCHAR                  RegsBase;         // Registers base address
    ULONG                   RegsLength;       // Registers base length

    PUCHAR                  PortBase;         // Port base address
    ULONG                   PortLength;       // Port base length

    PUCHAR                  SRAMBase;         // SRAM base address
    ULONG                   SRAMLength;       // SRAM base length

    PUCHAR                  SRAM2Base;        // SRAM (alt) base address
    ULONG                   SRAM2Length;      // SRAM (alt) base length

    WDFINTERRUPT            Interrupt;     // Returned by InterruptCreate

    union {
        INT_CSR bits;
        ULONG   ulong;
    }                       IntCsr;

    union {
        DMA_CSR bits;
        UCHAR   uchar;
    }                       Dma0Csr;

    union {
        DMA_CSR bits;
        UCHAR   uchar;
    }                       Dma1Csr;

    // DmaEnabler
    WDFDMAENABLER           DmaEnabler;
    ULONG                   MaximumTransferLength;

    // Write
    WDFQUEUE                WriteQueue;

    WDFDMATRANSACTION       WriteDmaTransaction;

    ULONG                   WriteTransferElements;
    WDFCOMMONBUFFER         WriteCommonBuffer;
    size_t                  WriteCommonBufferSize;
    _Field_size_(WriteCommonBufferSize) PUCHAR WriteCommonBufferBase;
    PHYSICAL_ADDRESS        WriteCommonBufferBaseLA;  // Logical Address

    // Read
    ULONG                   ReadTransferElements;
    WDFCOMMONBUFFER         ReadCommonBuffer;
    size_t                  ReadCommonBufferSize;
    _Field_size_(ReadCommonBufferSize) PUCHAR ReadCommonBufferBase;
    PHYSICAL_ADDRESS        ReadCommonBufferBaseLA;   // Logical Address

    WDFDMATRANSACTION       ReadDmaTransaction;

    WDFQUEUE                ReadQueue;

	// Device Control
	WDFQUEUE                DeviceControlQueue;

    ULONG                   HwErrCount;

}  DEVICE_EXTENSION, *PDEVICE_EXTENSION;

//
// This will generate the function named PLxGetDeviceContext to be use for
// retreiving the DEVICE_EXTENSION pointer.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, PLxGetDeviceContext)

#if !defined(ASSOC_WRITE_REQUEST_WITH_DMA_TRANSACTION)
//
// The context structure used with WdfDmaTransactionCreate
//
typedef struct TRANSACTION_CONTEXT {

    WDFREQUEST     Request;

} TRANSACTION_CONTEXT, * PTRANSACTION_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(TRANSACTION_CONTEXT, PLxGetTransactionContext)

#endif

//
// Function prototypes
//
DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD PLxEvtDeviceAdd;

EVT_WDF_OBJECT_CONTEXT_CLEANUP PlxEvtDriverContextCleanup;

EVT_WDF_DEVICE_D0_ENTRY PLxEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT PLxEvtDeviceD0Exit;
EVT_WDF_DEVICE_PREPARE_HARDWARE PLxEvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE PLxEvtDeviceReleaseHardware;

EVT_WDF_IO_QUEUE_IO_READ PLxEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE PLxEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL PLxEvtIoDeviceControl;

EVT_WDF_INTERRUPT_ISR PLxEvtInterruptIsr;
EVT_WDF_INTERRUPT_DPC PLxEvtInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE PLxEvtInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE PLxEvtInterruptDisable;

NTSTATUS
PLxSetIdleAndWakeSettings(
    IN PDEVICE_EXTENSION FdoData
    );

NTSTATUS
PLxInitializeDeviceExtension(
    IN PDEVICE_EXTENSION DevExt
    );

NTSTATUS
PLxPrepareHardware(
    IN PDEVICE_EXTENSION DevExt,
    IN WDFCMRESLIST     ResourcesTranslated
    );

NTSTATUS
PLxInitRead(
    IN PDEVICE_EXTENSION DevExt
    );

NTSTATUS
PLxInitWrite(
    IN PDEVICE_EXTENSION DevExt
    );

//
// WDFINTERRUPT Support
//
NTSTATUS
PLxInterruptCreate(
    IN PDEVICE_EXTENSION DevExt
    );

VOID
PLxReadRequestComplete(
    IN WDFDMATRANSACTION  DmaTransaction,
    IN NTSTATUS           Status
    );

VOID
PLxWriteRequestComplete(
    IN WDFDMATRANSACTION  DmaTransaction,
    IN NTSTATUS           Status
    );

NTSTATUS
PLxInitializeHardware(
    IN PDEVICE_EXTENSION DevExt
    );

VOID
PLxShutdown(
    IN PDEVICE_EXTENSION DevExt
    );

EVT_WDF_PROGRAM_DMA PLxEvtProgramReadDma;
EVT_WDF_PROGRAM_DMA PLxEvtProgramWriteDma;

VOID
PLxHardwareReset(
    IN PDEVICE_EXTENSION    DevExt
    );

NTSTATUS
PLxInitializeDMA(
    IN PDEVICE_EXTENSION DevExt
    );

#pragma warning(disable:4127) // avoid conditional expression is constant error with W4

#endif  // _PCI9656_H_

