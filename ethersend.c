/* RAW ethernet communication.
 *     Sender file: The user running this file will be the first one to send a message. After sending the data, this user can receive a message.
 * Note: Source and destination MAC addresses must be correct.
 *
 * By: I.B.O. <https://github.com/iBahena>
 *
 * Source: http://aschauf.landshut.org/fh/linux/udp_vs_raw/ch01s03.html
 */
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>

#define false 0
#define true (!false)

// MAIN PROGRAM
main() {
	while(1) 
	{
		printf("\n~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n");		
		printf("%d\n",snd());
		printf("\n\t *** Sending data... *** \n");
		printf("\n~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n\n");
		
		sleep(2);
		
		printf("\n~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n");
		printf("\n\n\t *** Receiving data... *** \n");
		printf("\n%d\n",rcv());
		printf("\n~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n\n");
	}
}


// SENDS DATA...
int snd() 
{
	int s; /*socket descriptor*/

	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	
	if (s == -1) 
	{
		return -1;
	}

	printf("\n\t     ...here: \n");
	
	/*target address*/
	struct sockaddr_ll socket_address;

	/*buffer for ethernet frame*/
	void* buffer = (void*)malloc(ETH_FRAME_LEN);
	 
	/*pointer to ethenet header*/
	unsigned char* etherhead = buffer;

	/*userdata in ethernet frame*/
	unsigned char* data = buffer + 14;

	/*another pointer to ethernet header*/
	struct ethhdr *eh = (struct ethhdr *)etherhead;
	 
	int send_result = 0;

	/*our MAC address*/
	unsigned char src_mac[6] = {0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1};

	/*other host MAC address*/
	unsigned char dest_mac[6] = {0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0xF2};

	/*prepare sockaddr_ll*/

	/*RAW communication*/
	socket_address.sll_family   = PF_PACKET;

	/*we don't use a protocoll above ethernet layer
	  ->just use anything here*/
	socket_address.sll_protocol = htons(ETH_P_IP);

	/*index of the network device
	see full code later how to retrieve it*/
	socket_address.sll_ifindex  = 2;

	/*ARP hardware identifier is ethernet*/
	socket_address.sll_hatype   = ARPHRD_ETHER;

	/*target is another host*/
	socket_address.sll_pkttype  = PACKET_OTHERHOST;

	/*address length*/
	socket_address.sll_halen    = ETH_ALEN;
	/*MAC - begin*/
	socket_address.sll_addr[0]  = 0x00;
	socket_address.sll_addr[1]  = 0x04;
	socket_address.sll_addr[2]  = 0x75;
	socket_address.sll_addr[3]  = 0xC8;
	socket_address.sll_addr[4]  = 0x28;
	socket_address.sll_addr[5]  = 0xE5;
	/*MAC - end*/
	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/

	/*set the frame header*/
	memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
	memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
	eh->h_proto = 0x00;

	/*fill the frame with some data*/
	int size = 264;
	printf("\n >> Type Message: ");
	data[size] = fgets(data, size, stdin);

	/*send the packet*/
	send_result = sendto(s, buffer, ETH_FRAME_LEN, 0, 
		      (struct sockaddr*)&socket_address, sizeof(socket_address));

	if (send_result == -1) 
	{ 
		return -1; 
	} else {
		return 0;
	}
}


// RECEIVES DATA...
int rcv() 
{
	int s; /*socketdescriptor*/
	int i;

	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	
	if (s == -1) {
		return -1;
	}

	printf("\n\t     ...here: \n\n");

	/*target address*/
	struct sockaddr_ll socket_address;

	/*buffer for ethernet frame*/
	char buffer[ETH_FRAME_LEN];

	/*pointer to ethenet header*/
	unsigned char* etherhead = buffer;

	/*userdata in ethernet frame*/
	unsigned char* data = buffer + 14;  // Skip Ethernet header

	/*another pointer to ethernet header*/
	struct ethhdr *eh = (struct ethhdr *)etherhead;

	socket_address.sll_family   = AF_PACKET;
	socket_address.sll_protocol = htons(ETH_P_IP);
	bind(s, (struct sockaddr *) & socket_address, sizeof(socket_address));

	int length = 0; /*length of the received frame*/ 
	
	/*other host MAC address*/
	unsigned char src_mac[6] = {0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0xF2};
	
	// Limit the data received to that of the other host device.
	do {
		length = recvfrom(s, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
	} while ( eh->h_proto != 0x00 );

	int status;
	
	next:
	if ( ((etherhead[6]!=src_mac[0]) || (etherhead[7]!=src_mac[1]) || (etherhead[8]!=src_mac[2]) || (etherhead[9]!=src_mac[3]) || (etherhead[10]!=src_mac[4]) || (etherhead[11]!=src_mac[5])) ) 
	{
		status = false;
	} else if ( ((etherhead[6]==src_mac[0]) && (etherhead[7]==src_mac[1]) && (etherhead[8]==src_mac[2]) && (etherhead[9]==src_mac[3]) && (etherhead[10]==src_mac[4]) && (etherhead[11]==src_mac[5])) ) 
	{
		status = true;
	}

	int count;
	for (count = 0; count < 1000; count++) {		
		if (status == false) {
			do {
				length = recvfrom(s, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
			} while ( eh->h_proto != 0x00 );
			goto next;
		} else {
			goto done;
		}	
	}	

	done:
	while ( status == true ) {
		if (length == -1) 
		{
			return -1;
		} else {
			printf("\n *Received Message:  ");
	
			for (i = 0; i < (length-14); i++) 
			{
				printf("%c",buffer[i+14]);
			}
		
			printf("\n\t------------------------------------");
			printf("\n\t  MAC sender:    %02x:%02x:%02x:%02x:%02x:%02x\n", etherhead[6],etherhead[7],etherhead[8], etherhead[9],etherhead[10],etherhead[11]);
			printf("\t  MAC Receiver:  %02x:%02x:%02x:%02x:%02x:%02x\n", etherhead[0],etherhead[1],etherhead[2], etherhead[3],etherhead[4],etherhead[5]);
			printf("\t------------------------------------");  

			printf("\n\n *Frame Length:  ");

			return length;
		} 
	}
}
