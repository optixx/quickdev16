/*
 * =====================================================================================
 *
 *            .d8888b  88888b.   .d88b.  .d8888b  888d888 8888b.  88888b.d88b.
 *            88K      888 "88b d8P  Y8b 88K      888P"      "88b 888 "888 "88b
 *            "Y8888b. 888  888 88888888 "Y8888b. 888    .d888888 888  888  888
 *                 X88 888  888 Y8b.          X88 888    888  888 888  888  888
 *             88888P' 888  888  "Y8888   88888P' 888    "Y888888 888  888  888
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */



/* Name: requests.h
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: requests.h 692 2008-11-07 15:07:40Z cs $
 */

/* This header is shared between the firmware and the host software. It
 * defines the USB request numbers (and optionally data types) used to
 * communicate between the host and the device.
 */

#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#define USB_UPLOAD_INIT         0
#define USB_UPLOAD_ADDR         1

#define USB_DOWNLOAD_INIT       2
#define USB_DOWNLOAD_ADDR       3

#define USB_CRC                 4
#define USB_CRC_ADDR            5

#define USB_BULK_UPLOAD_INIT    6
#define USB_BULK_UPLOAD_ADDR    7
#define USB_BULK_UPLOAD_NEXT    8
#define USB_BULK_UPLOAD_END     9
#define USB_MODE_SNES           10
#define USB_MODE_AVR            11
#define USB_AVR_RESET           12

#endif /* __REQUESTS_H_INCLUDED__ */
