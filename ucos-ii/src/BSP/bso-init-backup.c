/*
 * bso-init-backup.c
 *
 *  Created on: ??þ/??þ/????
 *      Author: YUSSRY
 */


#ifdef STDOUT_IS_PS7_UART
    //XUartPs         Uart_Ps_0;                                  /* Use the PS UART for Zynq devices.                    */
    XUartPs_Config *Config_0;
    XUartPs_Handler APPUartHandler = BSP_UART_Handler;

        /*
    	 * Initialize the UART driver so that it's ready to use
    	 * Look up the configuration in the config table, then initialize it.
    	 */
    Config_0 = XUartPs_LookupConfig(UART_DEVICE_ID);
    XUartPs_CfgInitialize(&Uart_Ps_0, Config_0, Config_0->BaseAddress);
    XUartPs_SetBaudRate(&Uart_Ps_0, 115200);

/*added by nehal*/

    /*register  Xilinx-uart-common-handler to CSP vector table and enable it */
        /*
        	 * Connect the UART (SDK-Handler) to the interrupt subsystem of the application such that interrupts
        	 * can occur. This function is application specific.
        	 */
        CSP_IntVectReg((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                                    (CSP_DEV_NBR    )CSP_INT_SRC_NBR_UART_01,
                                    (CPU_FNCT_PTR   )XUartPs_InterruptHandler,

                                    &Uart_Ps_0);
      /*configure the Receive Buffer attributes (set number of requested bytes,remaining bytes , NextBytePtr*/
        Uart_Ps_0.ReceiveBuffer.RequestedBytes = Uart_Ps_0.ReceiveBuffer.RemainingBytes =  8;
        Uart_Ps_0.ReceiveBuffer.NextBytePtr = &BSPRxBuffer[0];

        CSP_IntEn(CSP_INT_CTRL_NBR_MAIN,
       		CSP_INT_SRC_NBR_UART_01);


        /*
        	 * Setup the handlers for the UART that will be called from the
        	 * interrupt context when data has been sent and received, specify
        	 * a pointer to the UART driver instance as the callback reference
        	 * so the handlers are able to access the instance data
        	 */

    XUartPs_SetHandler(&Uart_Ps_0, APPUartHandler ,
    		 (void *)0);


        /*
    	 * Enable the interrupt of the UART so interrupts will occur, setup
    	 * a local loopback so data that is sent will be received.
    	 */

    //XUartPs_SetInterruptMask(&Uart_Ps_0, (XUARTPS_IXR_RXOVR | XUARTPS_IXR_RXEMPTY |XUARTPS_IXR_RXFULL));
    u32 TestIntMask = XUARTPS_IXR_RXOVR | XUARTPS_IXR_RXFULL  | XUARTPS_IXR_OVER ;
    XUartPs_SetInterruptMask(&Uart_Ps_0, TestIntMask);
	/*
	 * */
  /*  XUartPs_SetOperMode(&Uart_Ps_0, XUARTPS_OPER_MODE_LOCAL_LOOP);*/

    /*
    	 * Set the receiver timeout. If it is not set, and the last few bytes
    	 * of data do not trigger the over-water or full interrupt, the bytes
    	 * will not be received. By default it is disabled.
    	 *
    	 * The setting of 8 will timeout after 8 x 4 = 32 character times.
    	 * Increase the time out value if baud rate is high, decrease it if
    	 * baud rate is low.
    	 */
    	/*XUartPs_SetRecvTimeout(&Uart_Ps_0, 8);*/

    /*this is called to test looping not interrupt driven receiving*/
    XUartPs_ReceiveBuffer(&Uart_Ps_0);
    	/*XUartPs_Recv(&Uart_Ps_0, &BSPRxBuffer[0], 8);*/
    /*end added by nehal*/

#elif defined(STDOUT_IS_16550)
    XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, UART_BAUD);
    XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
