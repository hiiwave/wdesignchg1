 /*
no ack version(13次)
message
(9)新手呼叫
(8,ID)回應新手
(7,mailmanID)通知大家七秒後開始傳送,mailman為轉寄者ID
(6,sourceID,mailmanID,Addr)每個人通知device8的Addr訊息,mailman為轉寄者ID
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "autonet.h"
int main(void)
{
	uint8_t NEW_OR_OLD = 0;
		// 1: New, 0: Old
	uint8_t MAX_ID = 4;	// 8

	uint8_t Type;
	uint16_t Addr_16;
	uint8_t Addr;
	uint8_t radio_channel;
	uint16_t radio_panID;
 
	uint8_t  myID;
	
	uint8_t tx_msg[20];
	uint8_t tx_length;
 
	uint8_t rcvd_msg[20];
	uint8_t rcvd_payload[20];
	uint8_t rcvd_length;
	uint8_t rcvd_payloadLength;
	uint8_t rcvd_rssi;

	//一堆當BOOL用的變數
	uint8_t  w;
	uint8_t ww;
	uint8_t  alone;
	uint8_t  wait_rookie;
	uint8_t  help_forward_7;

	uint8_t LIGHT = 0;

	

	//device8紀錄用
	uint8_t Addr_list[9];
	uint8_t done_list[9];

	//螢幕顯示用,不會寫
	uint8_t output_array[10];
	
	Type = Type_Light;
	Addr_16 =  0x0004;
	Addr = Addr_16& 0xff;
	radio_channel = 25;
	radio_panID = 0x00BB;

	w=1;
	ww=1;
	alone=1;
	wait_rookie=1;
	help_forward_7=1;
	
	Initial(Addr_16,Type, radio_channel, radio_panID);

	
	
	//新手呼叫
	tx_msg[0] = 9;
	tx_length = 1;
   RF_Tx(0xFFFF, tx_msg, tx_length);
	
	setTimer(1,1500,UNIT_MS);	//等1.5秒回音

	while(!checkTimer(1) && alone) {
   	if(RF_Rx(rcvd_msg, &rcvd_length, &rcvd_rssi)){
			getPayloadLength(&rcvd_payloadLength, rcvd_msg);
			getPayload(rcvd_payload, rcvd_msg, rcvd_payloadLength);

			if(rcvd_payload[0] == 8){
				myID = rcvd_payload[1] + 1;	// 收到8,計算得到自己的ID 
				alone = 0;				//有人回應
			}
		}
	}

	if(alone == 1)myID= 1;		//沒人回應，ID為1
	if(myID == MAX_ID){			//假如為8號device,廣播7)
		tx_msg[0] = 7;
		tx_msg[1] = myID;
		RF_Tx(0xFFFF, tx_msg, tx_length);
	}


	setTimer(3,500,UNIT_MS);
	while (w <= 2 * myID) {
		if (checkTimer(3)) {
			LIGHT = !LIGHT;
			setGPIO(1, LIGHT);
			w = w + 1;
		}
	}
	
	
	/*
	setTimer(8, 1000, UNIT_MS);
	while (1) {
		if (checkTimer(8)) {
			LIGHT = !LIGHT;
			setGPIO(1, LIGHT);
			
			sprintf((char *)output_array,"%d\r\n", myID);
			if (NEW_OR_OLD) {
				COM2_Tx(output_array,3);
			} else {
				COM1_Tx(output_array,3);	
			}
			
		}
	}
	*/
	

	


	//ID為8與ID非8進行不同行為
	//若為device MAX,只需等待訊息6
	if(myID == MAX_ID){
		w = 1;
		while(w <= MAX_ID)
		{
			done_list[w] = 0;		//歸零donelist,後面用來紀錄已完成ID
			w = w + 1;
		}
		w = 1;
		while(w <= MAX_ID){
			if(RF_Rx(rcvd_msg, &rcvd_length, &rcvd_rssi)){
				getPayloadLength(&rcvd_payloadLength, rcvd_msg);
				getPayload(rcvd_payload, rcvd_msg, rcvd_payloadLength);
				
				if(rcvd_payload[0] == 6 && done_list[rcvd_payload[1]] == 0){
					done_list[rcvd_payload[1]] = 1;
					Addr_list[rcvd_payload[1]] = rcvd_payload[3];
					
					
					/*
					
					
					sprintf((char *)output_array,"Time %d : Address %d : ID %d\r\n", w, rcvd_payload[3], rcvd_payload[1]);
					if (NEW_OR_OLD) {
						COM2_Tx(output_array,3);
					} else {
						COM1_Tx(output_array,3);	
					}
					
					*/
	            
	 				// show("Time"w":Adress:"rcvd_payload[3]/"ID"rcvd_payload[1]);
					// 某種顯示字串方法;不會寫
					w = w+1;
				}
				setGPIO(1, LIGHT);
				if(w == MAX_ID)
				{
					w=1;
					while(w<MAX_ID)
					{
						setTimer(7,500,UNIT_MS);
						while (ww <= 2 * Addr_list[w]) {
							if (checkTimer(7)) {
								LIGHT = !LIGHT;
								setGPIO(1, LIGHT);
								ww = ww + 1;
							}
						}		
						ww =1;
						w = w + 1;
					}
				}
			}
		}

	}
	//非8號device
	else{
		while(wait_rookie){
			if(RF_Rx(rcvd_msg, &rcvd_length, &rcvd_rssi)){
				getPayloadLength(&rcvd_payloadLength, rcvd_msg);
				getPayload(rcvd_payload, rcvd_msg, rcvd_payloadLength);
				
				if(rcvd_payload[0] == 9){	//監測9
					tx_msg[0] = 8;		//回應8+自己ID
					tx_msg[1] = myID;
					tx_length = 2;
					RF_Tx(0xFFFF, tx_msg, tx_length);
					wait_rookie = 0;		//結束等菜鳥狀態
				}
			}
		}
		while(help_forward_7){
			if(RF_Rx(rcvd_msg, &rcvd_length, &rcvd_rssi)){
				getPayloadLength(&rcvd_payloadLength, rcvd_msg);
				getPayload(rcvd_payload, rcvd_msg, rcvd_payloadLength);
				//只接收比自己大一號的訊息(為了使其七秒後有序)
				if(rcvd_payload[0] == 7 && rcvd_payload[1] == myID+1){
					tx_msg[0] = 7;
					tx_msg[1] = myID;
					tx_length = 2;
					RF_Tx(0xFFFF, tx_msg, tx_length);
					setTimer(2,7000,UNIT_MS);
					help_forward_7 = 0;		//結束協助轉送7狀態
				}
			}
		}
		//timer到期: 傳送(6,發送者ID,轉送者ID,發送者Addr)
		while(checkTimer(2)){
			tx_msg[0] = 6;
			tx_msg[1] = myID;	// source id
			tx_msg[2] = myID;	// forwarder id
			tx_msg[3] = Addr;
			tx_length = 4;
			RF_Tx(0xFFFF, tx_msg, tx_length);
		}
		//保持協助轉送狀態
		while(1){
			if(RF_Rx(rcvd_msg, &rcvd_length, &rcvd_rssi)){
				getPayloadLength(&rcvd_payloadLength, rcvd_msg);
				getPayload(rcvd_payload, rcvd_msg, rcvd_payloadLength);
				//轉送者比自己小就幫忙轉送
				if(rcvd_payload[0] == 6 && rcvd_payload[2] < myID){
					tx_msg[0] = 6;
					tx_msg[1] = rcvd_payload[1] ;
					tx_msg[2] = myID;
					tx_msg[3] = rcvd_payload[3] ;
					RF_Tx(0xFFFF, tx_msg, tx_length);
				}
			}
		}
	}
}