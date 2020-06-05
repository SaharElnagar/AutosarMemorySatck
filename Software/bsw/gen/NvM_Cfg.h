/*
 * NvM_Cfg.h
 *
 *
 *      Author: Sahar
 */

#ifndef BSW_GEN_NVM_CFG_H_
#define BSW_GEN_NVM_CFG_H_



/*****************************************************************************************/
/*                                  NvMCommon Container                                  */
/*****************************************************************************************/

/*ECUC_NvM_00491
 *Preprocessor switch to enable some API calls which are related to NVM API configuration classes.
 */
#define NVM_API_CONFIG_CLASS

/*ECUC_NvM_00550
 *This parameter specifies whether BswM is informed about the current status
 *of the multiblock job. True: call BswM_NvM_CurrentJobMode if ReadAll and WriteAll are started,
 *finished, canceled  False: do not inform BswM at all
 */
#define NVM_BSWM_MULTIBLOCK_JOBSTATUS_INFORMATION

/*ECUC_NvM_00492
 *Configuration ID regarding the NV memory layout.
 *This configuration ID shall be published as e.g.
 *a SW-C shall have the possibility to write it to NV memory*/
#define NVM_COMPILED_CONFIG_ID

/*ECUC_NvM_0049
 *If CRC is configured for at least one NVRAM block,
 *this parameter defines the maximum number of bytes which shall
 * be processed within one cycle of job processing*/
#define NVM_CRC_NUM_OF_BYTES

/*ECUC_NvM_00494
 *Defines the number of least significant bits which shall be used to address a certain data set of a
 *NVRAM block within the interface to the memory hardware abstraction. 0..8:
 *Number of bits which are used for dataset or redundant block addressing.
 *0: No dataset or redundant NVRAM blocks are configured at all, no selection bits required.
 *1: In case of redundant NVRAM blocks are configured, but no dataset NVRAM blocks.
 */
#define NVM_DATASET_SELECTION_BITS

/*ECUC_NvM_00495
 *Switches the development error detection and notification on or off.
 * true: detection and notification is enabled.
 * false: detection and notification is disabled.
 */
#define NVM_DEV_ERROR_DETECT

/*ECUC_NvM_00496
 *Preprocessor switch to enable switching memory drivers to fast mode during performing
 *NvM_ReadAll and NvM_WriteAll
 *true: Fast mode enabled.
 *false: Fast mode disabled.
 */
#define NVM_DRV_MODE_SWITCH

/*ECUC_NvM_00497
 *Preprocessor switch to enable the dynamic configuration management handling by the NvM_ReadAll request.
 *true: Dynamic configuration management handling enabled.
 *false: Dynamic configuration management handling disabled.
 *This parameter affects all NvM processing related to Block with ID 1 and all
 *processing related to Resistant to Changed Software. If the Dynamic Configuration is disabled,
 *Block 1 cannot be used by NvM*/
#define NVM_DYNAMIC_CONFIGURATIONS

/*ECUC_NvM_00498
 *Preprocessor switch to enable job prioritization handling
 *true: Job prioritization handling enabled.
 *false: Job prioritization handling disabled.
 */
#define NVM_JOB_PRIORITIZATION              STD_ON

/* ECUC_NvM_00500
 * Entry address of the common callback routine which shall be invoked on termination
 * of each asynchronous multi block request
 */
#define NVM_MULTI_BLOCK_CALLBACK

/* ECUC_NvM_00501
 * Preprocessor switch to enable/disable the polling mode in the NVRAM Manager and at the same time disable/enable the callback functions useable by lower layers
 * true: Polling mode enabled, callback function usage disabled.
 * false: Polling mode disabled, callback function usage enabled.*/
#define NVM_POLLING_MODE

/* ECUC_NvM_00518
 * Defines the number of retries to let the application copy data to or from the
 */
#define NVM_REPEAT_MIRROR_OPERATIONS

/* ECUC_NvM_00502
 * Preprocessor switch to enable the API NvM_SetRamBlockStatus.
 * true: API NvM_SetRamBlockStatus enabled.
 * false: API NvM_SetRamBlockStatus disabled.*/
#define NVM_SET_RAM_BLOCK_STATUS_API

/* ECUC_NvM_00504
 * Defines the number of queue entries for the standard job queue
 */
#define NVM_SIZE_STANDARD_JOB_QUEUE         (10U)

/* ECUC_NvM_00503
 * Defines the number of queue entries for the immediate priority job queue
 */
#if (NVM_JOB_PRIORITIZATION == STD_ON)
#define NVM_SIZE_IMMEDIATE_JOB_QUEUE        (10U)
#endif

/*****************************************************************************************/
/*                                 NvMBlockDescriptor                                    */
/*****************************************************************************************/

#define NUMBER_OF_NVM_BLOCKS                (1U)

/*ECUC_NvM_00481
 * Identification of a NVRAM block via a unique block identifier. Implementation Type: NvM_BlockIdType.
 *min = 2 max = 2^(16- NVM_DATASET_SELECTION_BITS)-1
 *Reserved NVRAM block IDs: 0 -> to derive multi block request results via
 *NvM_GetErrorStatus 1 -> redundant NVRAM block which holds the configuration ID
 *(generation tool should check that this block is correctly configured from type,CRC and size point of view)
 */
#define NVM_NVRAM_BLOCK_0_ID                (2U)

/*ECUC_NvM_00036
 *Defines CRC usage for the NVRAM block, memory space for CRC is reserved in RAM and NV memory.
 *true: CRC will be used for this NVRAM block.
 *false: CRC will not be used for this NVRAM block.
 */
#define  NVM_BLOCK_0_USE_CRC                STD_ON

/* ECUC_NvM_00476
 * Defines CRC data width for the NVRAM block. Default:
 * NVM_CRC16, i.e. CRC16 will be used if NVM_BLOCK_USE_CRC==true
 * */
#if(NVM_BLOCK_0_USE_CRC == STD_ON)
#define NVM_BLOCK_0_CRC_TYPE                NVM_CRC32
#endif

/*ECUC_NvM_00477
 *Defines the job priority for a NVRAM block (0 = Immediate priority).
 * */
#define  NVM_BLOCK_0_JOB_PRIORITY           (1U)

/*ECUC_NvM_00062
 *Defines the block management type for the NVRAM block.
 */
#define  NVM_BLOCK_0_MANAGEMENT_TYPE        NVM_BLOCK_DATASET

/*
 * ECUC_NvM_00557
 * Defines whether the RAM Block shall be
 * auto validated during shutdown phase.
 * true: if auto validation mechanism is used,
 * false: otherwise
 */
#define  NVM_BLOCK_0_USE_AUTO_VALIDATION    STD_ON

/* ECUC_NvM_00556
 * Defines whether the CRC of the RAM Block shall be compared during
 * a write job with the CRC which was calculated during the last successful
 * read or write job. true: if compare mechanism is used, false: otherwise
 */
#define  NVM_BLOCK_0_USE_CRC_COMP_MECHANISM     STD_ON

/*ECUC_NvM_00552
 * Defines if NvMSetRamBlockStatusApi shall be used for this block or not.
 * Note: If NvMSetRamBlockStatusApi is disabled this configuration parameter shall be ignored.
 * true: calling of NvMSetRamBlockStatus for this RAM block shall set the status of the RAM block.
 *false: calling of NvMSetRamBlockStatus for this RAM block shall be ignored.
 */
#define  NVM_BLOCK_0_USE_SET_RAM_BLOCK_STATUS   STD_ON

/* ECUC_NvM_00519
 * Defines whether an explicit synchronization mechanism with a RAM mirror
 * and callback routines for transferring data to and from NvM module's RAM mirror
 * is used for NV block. true if synchronization mechanism is used,
 * false otherwise.
 */
#define  NVM_BLOCK_0_USE_SYNC_MECHANISM         STD_ON

/*ECUC_NvM_00033
 *Defines an initial write protection of the NV block
 *true: Initial block write protection is enabled.
 *false: Initial block write protection is disabled.
 */
#define  NVM_BLOCK_0_WRITE_PROT                 STD_OFF

/* ECUC_NvM_00551
 * This parameter specifies whether BswM is informed about the current status of the specified block.
 * True: Call BswM_NvM_CurrentBlockMode on changes
 * False: Don't inform BswM at all
 */
#define NVM_BSWM_BLOCK_0_STATUS_INFORMATION     STD_OFF

/*ECUC_NvM_00119
 *Defines CRC (re)calculation for the permanent RAM block or NVRAM blocks which are configured to use explicit
 *synchronization mechanism.
 *true: CRC will be (re)calculated for this permanent RAM block.
 *false: CRC will not be (re)calculated for this permanent RAM block.
 */
#define NVM_CALC_RAM_BLOCK_CRC                  STD_ON

#endif /* BSW_GEN_NVM_CFG_H_ */

