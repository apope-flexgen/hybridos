/*
 * fims.h
 *
 *  Created on: May 30, 2018
 *      Author: jcalcagni
 */

#ifndef FIMS_H_
#define FIMS_H_

#define SOCKET_NAME "/tmp/FlexGen_FIMS_Server.socket"
// arbitrarily large to support tx100 volume of data, can be reduced with naked body publishes
#define MAX_SUBSCRIPTIONS 256
#define FIMS_DATA_LAYOUT_VERSION 1 // this is the current layout version for decoding data received over the unix socket. If this changes we'll know

#define AESKEY_FILE "/usr/local/etc/config/fims_aes"
#define AESSIZE 32
#define AES_BYTE_ERR -2
#define FIMS_CONN_CLOSED -1



#endif /* FIMS_H_ */
