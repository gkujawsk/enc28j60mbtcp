// This code slightly follows the conventions of, but is not derived from:
//      EHTERSHIELD_H library for Arduino etherShield
//      Copyright (c) 2008 Xing Yu.  All right reserved. (this is LGPL v2.1)
// It is however derived from the enc28j60 and ip code (which is GPL v2)
//      Author: Pascal Stang
//      Modified by: Guido Socher
//      DHCP code: Andrew Lindsay
// Hence: GPL V2
//
// 2010-05-19 <jc@wippler.nl>
//
//
// PIN Connections (Using Arduino UNO):
//   VCC -   3.3V
//   GND -    GND
//   SCK - Pin 13
//   SO  - Pin 12
//   SI  - Pin 11
//   CS  - Pin  8
//
/** @file */

#ifndef EtherCard_h
#define EtherCard_h
#ifndef __PROG_TYPES_COMPAT__
  #define __PROG_TYPES_COMPAT__
#endif


#include <Arduino.h> // Arduino 1.0
#define WRITE_RESULT size_t
#define WRITE_RETURN return 1;

#include <avr/pgmspace.h>
#include "bufferfiller.h"
#include "enc28j60.h"
#include "net.h"
#include "stash.h"

/** Enable DHCP.
*   Setting this to zero disables the use of DHCP; if a program uses DHCP it will
*   still compile but the program will not work. Saves about 60 bytes SRAM and
*   1550 bytes flash.
*/
#define ETHERCARD_DHCP 0

/** Enable client connections.
* Setting this to zero means that the program cannot issue TCP client requests
* anymore. Compilation will still work but the request will never be
* issued. Saves 4 bytes SRAM and 550 byte flash.
*/
#define ETHERCARD_TCPCLIENT 1

/** Enable TCP server functionality.
*   Setting this to zero means that the program will not accept TCP client
*   requests. Saves 2 bytes SRAM and 250 bytes flash.
*/
#define ETHERCARD_TCPSERVER 0

/** Enable UDP server functionality.
*   If zero UDP server is disabled. It is
*   still possible to register callbacks but these will never be called. Saves
*   about 40 bytes SRAM and 200 bytes flash. If building with -flto this does not
*   seem to save anything; maybe the linker is then smart enough to optimize the
*   call away.
*/
#define ETHERCARD_UDPSERVER 0

/** Enable automatic reply to pings.
*   Setting to zero means that the program will not automatically answer to
*   PINGs anymore. Also the callback that can be registered to answer incoming
*   pings will not be called. Saves 2 bytes SRAM and 230 bytes flash.
*/
#define ETHERCARD_ICMP 1

/** Enable use of stash.
*   Setting this to zero means that the stash mechanism cannot be used. Again
*   compilation will still work but the program may behave very unexpectedly.
*   Saves 30 bytes SRAM and 80 bytes flash.
*/
#define ETHERCARD_STASH 1


/** This type definition defines the structure of a UDP server event handler callback function */
typedef void (*UdpServerCallback)(
    uint16_t dest_port,    ///< Port the packet was sent to
    uint8_t src_ip[IP_LEN],    ///< IP address of the sender
    uint16_t src_port,    ///< Port the packet was sent from
    const char *data,   ///< UDP payload data
    uint16_t len);        ///< Length of the payload data

/** This type definition defines the structure of a DHCP Option callback function */
typedef void (*DhcpOptionCallback)(
    uint8_t option,     ///< The option number
    const byte* data,   ///< DHCP option data
    uint8_t len);       ///< Length of the DHCP option data



/** This class provides the main interface to a ENC28J60 based network interface card and is the class most users will use.
*   @note   All TCP/IP client (outgoing) connections are made from source port in range 2816-3071. Do not use these source ports for other purposes.
*/
class EtherCard : public Ethernet {
public:
    static uint8_t mymac[ETH_LEN];  ///< MAC address
    static uint8_t myip[IP_LEN];    ///< IP address
    static uint8_t netmask[IP_LEN]; ///< Netmask
    static uint8_t broadcastip[IP_LEN]; ///< Subnet broadcast address
    static uint8_t gwip[IP_LEN];   ///< Gateway
    static uint8_t dhcpip[IP_LEN]; ///< DHCP server IP address
    static uint8_t dnsip[IP_LEN];  ///< DNS server IP address
    static uint8_t hisip[IP_LEN];  ///< DNS lookup result
    static uint16_t hisport;  ///< TCP port to connect to (default 80)
    static bool using_dhcp;   ///< True if using DHCP
    static bool persist_tcp_connection; ///< False to break connections on first packet received
    static uint16_t delaycnt; ///< Counts number of cycles of packetLoop when no packet received - used to trigger periodic gateway ARP request

    static bool tcpconnected();

    // EtherCard.cpp
    /**   @brief  Initialise the network interface
    *     @param  size Size of data buffer
    *     @param  macaddr Hardware address to assign to the network interface (6 bytes)
    *     @param  csPin Arduino pin number connected to chip select. Default = 8
    *     @return <i>uint8_t</i> Firmware version or zero on failure.
    */
    static uint8_t begin (const uint16_t size, const uint8_t* macaddr,
                          uint8_t csPin, void (*_chipSelectLow)(uint8_t), void (*_chipSelectHigh)());

    /**   @brief  Configure network interface with static IP
    *     @param  my_ip IP address (4 bytes). 0 for no change.
    *     @param  gw_ip Gateway address (4 bytes). 0 for no change. Default = 0
    *     @param  dns_ip DNS address (4 bytes). 0 for no change. Default = 0
    *     @param  mask Subnet mask (4 bytes). 0 for no change. Default = 0
    *     @return <i>bool</i> Returns true on success - actually always true
    */
    static bool staticSetup (const uint8_t* my_ip,
                             const uint8_t* gw_ip = 0,
                             const uint8_t* dns_ip = 0,
                             const uint8_t* mask = 0);

    // tcpip.cpp
    /**   @brief  Parse received data
    *     @param  plen Size of data to parse (e.g. return value of packetReceive()).
    *     @return <i>uint16_t</i> Offset of TCP payload data in data buffer or zero if packet processed
    *     @note   Data buffer is shared by receive and transmit functions
    *     @note   Only handles ARP and IP
    */
    static uint16_t packetLoop (uint16_t plen);

    /**   @brief  Accept a TCP/IP connection
    *     @param  port IP port to accept on - do nothing if wrong port
    *     @param  plen Number of bytes in packet
    *     @return <i>uint16_t</i> Offset within packet of TCP payload. Zero for no data.
    */
    static uint16_t accept (uint16_t port, uint16_t plen);

    /**   @brief  Set the gateway address
    *     @param  gwipaddr Gateway address (4 bytes)
    */
    static void setGwIp (const uint8_t *gwipaddr);

    /**   @brief  Updates the broadcast address based on current IP address and subnet mask
    */
    static void updateBroadcastAddress();

    /**   @brief  Check if got gateway hardware address (ARP lookup)
    *     @return <i>unit8_t</i> True if gateway found
    */
    static uint8_t clientWaitingGw ();

    /**   @brief  Prepare a TCP request
    *     @param  result_cb Pointer to callback function that handles TCP result
    *     @param  datafill_cb Pointer to callback function that handles TCP data payload
    *     @param  port Remote TCP/IP port to connect to
    *     @return <i>unit8_t</i> ID of TCP/IP session (0-7)
    *     @note   Return value provides id of the request to allow up to 7 concurrent requests
    */
    static uint8_t clientTcpReq (uint8_t (*result_cb)(uint8_t,uint8_t,uint16_t,uint16_t),
                                 uint16_t (*datafill_cb)(uint8_t),uint16_t port);

    /**   @brief  Resister the function to handle ping events
    *     @param  cb Pointer to function
    */
    static void registerPingCallback (void (*cb)(uint8_t*));

    /**   @brief  Send ping
    *     @param  destip Pointer to 4 byte destination IP address
    */
    static void clientIcmpRequest (const uint8_t *destip);

    /**   @brief  Check for ping response
    *     @param  ip_monitoredhost Pointer to 4 byte IP address of host to check
    *     @return <i>uint8_t</i> True (1) if ping response from specified host
    */
    static uint8_t packetLoopIcmpCheckReply (const uint8_t *ip_monitoredhost);

    // new stash-based API
    /**   @brief  Send TCP request
    */
    static uint8_t tcpSend ();

    /**   @brief  Get TCP reply
    *     @return <i>char*</i> Pointer to TCP reply payload. NULL if no data.
    */
    static const char* tcpReply (uint8_t fd, uint16_t& len);

    /**   @brief  Configure TCP connections to be persistent or not
    *     @param  persist True to maintain TCP connection. False to finish TCP connection after first packet.
    */
    static void persistTcpConnection(bool persist);

    // webutil.cpp
    /**   @brief  Copies an IP address
    *     @param  dst Pointer to the 4 byte destination
    *     @param  src Pointer to the 4 byte source
    *     @note   There is no check of source or destination size. Ensure both are 4 bytes
    */
    static void copyIp (uint8_t *dst, const uint8_t *src);

    /**   @brief  Copies a hardware address
    *     @param  dst Pointer to the 6 byte destination
    *     @param  src Pointer to the 6 byte destination
    *     @note   There is no check of source or destination size. Ensure both are 6 bytes
    */
    static void copyMac (uint8_t *dst, const uint8_t *src);

    /**   @brief  Output to serial port in dotted decimal IP format
    *     @param  buf Pointer to 4 byte IP address
    *     @note   There is no check of source or destination size. Ensure both are 4 bytes
    */
    static void printIp (const uint8_t *buf);

    /**   @brief  Output message and IP address to serial port in dotted decimal IP format
    *     @param  msg Pointer to null terminated string
    *     @param  buf Pointer to 4 byte IP address
    *     @note   There is no check of source or destination size. Ensure both are 4 bytes
    */
    static void printIp (const char* msg, const uint8_t *buf);

    /**   @brief  Convert a 16-bit integer into a string
    *     @param  value The number to convert
    *     @param  ptr The string location to write to
    */
    char* wtoa(uint16_t value, char* ptr);

    /**   @brief  Return the sequence number of the current TCP package
    */
    static uint32_t getSequenceNumber();

    /**   @brief  Return the payload length of the current Tcp package
    */
    static uint16_t getTcpPayloadLength();

    boolean tcpClosed();

};

extern EtherCard ether; //!< Global presentation of EtherCard class

#endif
