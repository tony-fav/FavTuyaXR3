/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common/framework/platform_init.h"
#include "atcmd.h"
#include "atcmd/at_command.h"
#include "net/wlan/wlan.h"

#include "secrets.h"
#include "net/mqtt/MQTTClient-C/MQTTClient.h"
#include "serial.h"

// int httpd_start();

int mqtt_send_battery_state_low(Client* c);
int mqtt_send_battery_state_middle(Client* c);
int mqtt_send_battery_state_high(Client* c);
int mqtt_send_moisture_state_on(Client* c);
int mqtt_send_moisture_state_off(Client* c);

int tuya_parse_status_data_unit(u8 *sdbuf, int loc, Client* c);


static u8 queue_buf[1024];
// void parse_tuya_message(u8 *buf, int i0, int total_length)
// {

// }
int mqtt_send_battery_state_low(Client* c)
{
	MQTTMessage pubmsg;
	pubmsg.payload = "5";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 0;
	return MQTTPublish(c, "stat/xr3_3D6C07/BATTERY", &pubmsg);
}

int mqtt_send_battery_state_middle(Client* c)
{
	MQTTMessage pubmsg;
	pubmsg.payload = "50";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 0;
	return MQTTPublish(c, "stat/xr3_3D6C07/BATTERY", &pubmsg);
}

int mqtt_send_battery_state_high(Client* c)
{
	MQTTMessage pubmsg;
	pubmsg.payload = "95";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 0;
	return MQTTPublish(c, "stat/xr3_3D6C07/BATTERY", &pubmsg);
}

int mqtt_send_moisture_state_on(Client* c)
{
	MQTTMessage pubmsg;
	pubmsg.payload = "ON";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 0;
	return MQTTPublish(c, "stat/xr3_3D6C07/MOISTURE", &pubmsg);
}

int mqtt_send_moisture_state_off(Client* c)
{
	MQTTMessage pubmsg;
	pubmsg.payload = "OFF";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 0;
	return MQTTPublish(c, "stat/xr3_3D6C07/MOISTURE", &pubmsg);
}

int tuya_parse_status_data_unit(u8 *sdbuf, int loc, Client* c)
{
	u8 dpid = sdbuf[loc];
	u8 type = sdbuf[loc+1];
	u16 len = sdbuf[loc+3] | (sdbuf[loc+2] << 8);
	if (len == 1) {
		u8 val = sdbuf[loc+4];
		if (dpid==1) {
			if (type==4) {
				if (val==0) {
					printf("MCU: SD: State: Alarm\n");
					mqtt_send_moisture_state_on(c);
				} else if (val==1) {
					printf("MCU: SD: State: Normal\n");
					mqtt_send_moisture_state_off(c);
				} else {
					printf("MCU: SD: State: BAD VALUE %d\n", val);
				}
			} else {
				printf("MCU: SD: State: BAD TYPE %d\n", type);
			}
		} else if (dpid==3) {
			if (type==4) {
				if (val==0) {
					printf("MCU: SD: Battery: Low\n");
					mqtt_send_battery_state_low(c);
				} else if (val==1) {
					printf("MCU: SD: Battery: Middle\n");
					mqtt_send_battery_state_middle(c);
				} else if (val==2) {
					printf("MCU: SD: Battery: High\n");
					mqtt_send_battery_state_high(c);
				} else {
					printf("MCU: SD: Battery: BAD VALUE %d\n", val);
				}
			} else {
				printf("MCU: SD: Battery: BAD TYPE %d\n", type);
			}
		} else {
			printf("MCU: SD: Unhandled DPID %d\n", dpid);
		}		
	} else {
		printf("MCU: SD: Unhandled Length %d\n", len);
	}
	return (loc+5);
}


int main(void)
{
	platform_init();

	const char *ssid = my_wifi_ssid;    /* set your AP's ssid */
	const char *psk = my_wifi_pass; /* set your AP's password */
	wlan_sta_set((uint8_t *)ssid, strlen(ssid), (uint8_t *)psk);
	wlan_sta_enable();
	OS_Sleep(3); 
	wlan_sta_set((uint8_t *)ssid, strlen(ssid), (uint8_t *)psk);
	wlan_sta_enable();
	
	int rc = 0;
	unsigned char buf[350];
	unsigned char readbuf[100];
	Network n;
	Client c;

	NewNetwork(&n);
	printf("MQTT: NewNetwork\r\n");

	rc = ConnectNetwork(&n, my_mqtt_ip, 1883);
	printf("MQTT: ConnectNetwork: %d\r\n", rc);
	OS_Sleep(1);

	MQTTClient(&c, &n, 1000, buf, 350, readbuf, 100);
	printf("MQTT: Client\r\n");
	// OS_Sleep(1);

	MQTTMessage pubmsg;
	MQTTPacket_connectData data;
	MQTTPacket_willOptions will;
	will.struct_id[0] = 'M';
	will.struct_id[1] = 'Q';
	will.struct_id[2] = 'T';
	will.struct_id[3] = 'W';
	will.struct_version = 0;
	will.topicName.cstring = "tele/xr3_3D6C07/LWT";
	will.message.cstring = "Offline";
	will.qos = 0;
	will.retained = 0;
	data.willFlag = 1;
	data.will = will;
	data.MQTTVersion = 3;
	data.clientID.cstring = "xr3_3D6C07";
	data.username.cstring = my_mqtt_user;
	data.password.cstring = my_mqtt_pass;
	data.keepAliveInterval = 30;
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
	printf("MQTT: Connect: %d\r\n", rc);
	// OS_Sleep(1);

	printf("MQTT: Send Online to LWT\n");
	pubmsg.payload = "Online";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 0;
	rc = MQTTPublish(&c, "tele/xr3_3D6C07/LWT", &pubmsg);
	printf("MQTT: End Send: %d\r\n", rc);
	// OS_Sleep(1);

	printf("MQTT: HA Discovery Status\n");
	pubmsg.payload = "{\"name\":\"H2O Status\",\"stat_t\":\"tele/xr3_3D6C07/LWT\",\"pl_on\":\"Online\",\"pl_off\":\"Offline\",\"dev_cla\":\"connectivity\",\"uniq_id\":\"3D6C07_status\",\"dev\":{\"ids\":[\"3D6C07\"],\"name\":\"XR3 Feit H2O 3D6C07\",\"mdl\":\"Feit Electric H2O/WIFI\",\"sw\":\"Fav XR3\",\"mf\":\"Fav\"}}";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 1;
	rc = MQTTPublish(&c, "homeassistant/binary_sensor/3D6C07_status/config", &pubmsg);
	printf("MQTT: End Send: %d\r\n", rc);

	printf("MQTT: HA Discovery Moisture\n");
	pubmsg.payload = "{\"name\":\"H2O Moisture\",\"stat_t\":\"stat/xr3_3D6C07/MOISTURE\",\"dev_cla\":\"moisture\",\"frc_upd\":true,\"uniq_id\":\"3D6C07_moisture\",\"dev\":{\"ids\":[\"3D6C07\"]}}";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 1;
	rc = MQTTPublish(&c, "homeassistant/binary_sensor/3D6C07_moisture/config", &pubmsg);
	printf("MQTT: End Send: %d\r\n", rc);

	
	printf("MQTT: HA Discovery Battery\n");
	pubmsg.payload = "{\"name\":\"H2O Battery\",\"stat_t\":\"stat/xr3_3D6C07/BATTERY\",\"dev_cla\":\"battery\",\"unit_of_meas\":\"%\",\"uniq_id\":\"3D6C07_battery\",\"dev\":{\"ids\":[\"3D6C07\"]}}";
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = 0;
	pubmsg.retained = 1;
	rc = MQTTPublish(&c, "homeassistant/sensor/3D6C07_battery/config", &pubmsg);
	printf("MQTT: End Send: %d\r\n", rc);

	// httpd_start();
	// atcmd_start();

	// tuya serial time
	serial_init(UART1_ID, 9600, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1, 0);
	serial_start();
	s32 dcnt;

	// uint8_t tuya_msg[] = {0x55,0xAA,0x00,0x02,0x00,0x00,0x01}; // wifi command word but no data? not in docs
	// serial_write(tuya_msg, sizeof(tuya_msg));

	printf("MCU: Tell Tuya MCU I'm Online and Connected to Cloud\n");
	uint8_t tuya_wifi_connected_msg[] = {0x55,0xAA,0x00,0x02,0x00,0x01,0x04,0x06}; // tells MCU i'm connected to wifi and the cloud ;-)
	serial_write(tuya_wifi_connected_msg, sizeof(tuya_wifi_connected_msg));
	
	uint8_t tuya_respond_wifi_func[] = {0x55, 0xAA, 0x00, 0x07, 0x00, 0x02, 0x01, 0x50, 0x59}; // wifi works, signal 80
	

	// uint8_t tuya_product_info_message[] = {0x55,0xAA,0x00,0x01,0x00,0x00,0x00}; // tells MCU i'm connected to wifi and the cloud ;-)
	// serial_write(tuya_product_info_message, sizeof(tuya_product_info_message));
	
	u16 len = 0;
	int msg_cnt = 0;
	// int total_msgs = 0;
	// int checksum = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	// u8 asked_for_info = 0;
	printf("trying serial read loop\n");
	while (1)
	{
		dcnt = serial_read(queue_buf, sizeof(queue_buf));
		if (dcnt > 0) {
			for (i = 0; i < dcnt; i ++)
			{
				printf("%02X", queue_buf[i]);
			}
			printf("\n");
		}

		msg_cnt = 0;
		j = 0;
		while (j < dcnt) {
			if (queue_buf[j]==0x55) {
				j++;
				if (queue_buf[j]==0xAA) {
					j++;
					if (queue_buf[j]==0x00) {
						j++;
						msg_cnt++;
						// separate commands
						if (queue_buf[j]==0x01) {
							printf("MCU: CW 01: Query Product Info\n");
						} else if (queue_buf[j]==0x02) {
							printf("MCU: CW 02: WIFI Status\n");
						} else if (queue_buf[j]==0x03) {
							printf("MCU: CW 03: Reset WIFI\n");
						} else if (queue_buf[j]==0x04) {
							printf("MCU: CW 04: Reset WIFI Select Mode\n");
						} else if (queue_buf[j]==0x05) {
							printf("MCU: CW 05: Report Real Time Status\n");
							j++;
							len = queue_buf[j+1] | (queue_buf[j] << 8);
							j = j + 2;
							k = j + len;
							printf("MCU: Data Unit is %d Long\n", len);
							while (j < k) j = tuya_parse_status_data_unit(queue_buf, j, &c);
						} else if (queue_buf[j]==0x06) {
							printf("MCU: CW 06: Obtain the Local Time\n");
						} else if (queue_buf[j]==0x07) {
							printf("MCU: CW 07: WIFI Function Test\n");
							serial_write(tuya_respond_wifi_func, sizeof(tuya_respond_wifi_func)); // tell tuya 80% signal
						} else if (queue_buf[j]==0x08) {
							printf("MCU: CW 08: Report Real Time Status with Record Storage\n");
						} else if (queue_buf[j]==0x09) {
							printf("MCU: CW 09: Send Module Command\n");
						} else if (queue_buf[j]==0x0A) {
							printf("MCU: CW 0A: WIFI Module Firmware Upgrade\n");
						} else if (queue_buf[j]==0x0B) {
							printf("MCU: CW 0B: Signal Strength of Currently Connected Router\n");
						} else if (queue_buf[j]==0x0C) {
							printf("MCU: CW 0C: MCU Firmware Upgrade\n");
						} else if (queue_buf[j]==0x0D) {
							printf("MCU: CW 0D: MCU Firmware Upgrade\n");
						} else if (queue_buf[j]==0x0E) {
							printf("MCU: CW 0D: MCU Firmware Upgrade\n");
						} else if (queue_buf[j]==0x10) {
							printf("MCU: CW 10: Obtain DP Cache Command\n");
						} else {
							printf("MCU: CW %02X: Unknown\n", queue_buf[j]);
						}
					}
				}
			}
			j++;
		}
		if (msg_cnt > 0) printf("MCU: Received %d Messages\n", msg_cnt);
	}

	return 0;
}
