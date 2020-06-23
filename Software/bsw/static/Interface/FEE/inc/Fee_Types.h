/*
 * Fee_Types.h
 *
 *  Created on: Apr 26, 2020
 *      Author: Sahar
 */

#ifndef BSP_FEE_FEE_TYPES_H_
#define BSP_FEE_FEE_TYPES_H_


/*Struct to hold Configuration of each block*/
typedef struct{
   uint16 FeeBlockNumber ;
   uint16 FeeBlockSize ;
   boolean FeeImmediateData ;
   uint32 FeeNumberOfWriteCycles ;
}FeeBlockConfiguration ;


#endif /* BSP_FEE_FEE_TYPES_H_ */

