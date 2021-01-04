// Some common utilities needed for IP and web applications
// Author: Guido Socher
// Copyright: GPL V2
//
// 2010-05-20 <jc@wippler.nl>

#include "EtherCard.h"

void EtherCard::copyIp (uint8_t *dst, const uint8_t *src) {
    memcpy(dst, src, IP_LEN);
}

void EtherCard::copyMac (uint8_t *dst, const uint8_t *src) {
    memcpy(dst, src, ETH_LEN);
}

void EtherCard::printIp (const char* msg, const uint8_t *buf) {
    Serial.print(msg);
    EtherCard::printIp(buf);
    Serial.println();
}

void EtherCard::printIp (const uint8_t *buf) {
    for (uint8_t i = 0; i < IP_LEN; ++i) {
        Serial.print( buf[i], DEC );
        if (i < 3)
            Serial.print('.');
    }
}