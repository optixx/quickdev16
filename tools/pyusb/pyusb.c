/*
 * PyUSB - Python module for USB Access
 *
 * Copyright 2005 - 2007 Wander Lairson Costa
 */

#if _MSC_VER >= 1400	/* Visual C++ 8.00 */
#define _CRT_SECURE_NO_DEPRECATE
#endif /* _MSC_VER */

#include "pyusb.h"
#include <stdlib.h>
#include <stdio.h>
#define DEFAULT_TIMEOUT 100

/*
 * Necessary to compile successfully in python 2.3
 * Thanks to Mark Rages for the patch
 */
#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

// PYUSB_STATIC char cvsid[] = "$Id: pyusb.c,v 1.24 2007/05/15 21:20:55 wander Exp $";

/*
 * USBError
 */
PYUSB_STATIC PyObject *PyExc_USBError;

#define PyUSB_Error() PyErr_SetString(PyExc_USBError, usb_strerror())

#define SUPPORT_NUMBER_PROTOCOL(_Arg) \
	(PyNumber_Check(_Arg) || PyString_Check(_Arg) || PyUnicode_Check(_Arg))

#if !defined(NDEBUG)
#define DUMP_PARAMS 1
#else
#define DUMP_PARAMS 0
#endif /* NDEBUG */

#ifndef NDEBUG

/*
 * Print a buffer of data
 */
PYUSB_STATIC void printBuffer(
	const char *buffer,
	int size
	)
{
	int i;

	if (!buffer) {
		fputs("NULL\n", stderr);
		return;
	}

   	for (i = 0; i < size; ++i) {
		fprintf(stderr, "%2x ", buffer[i]);
		if (i && !(i % 20)) fputc('\n', stderr);
	}

	fputc('\n', stderr);
}

#endif /* NDEBUG */

/*
 * Converts a object with number procotol to int type
 */
PYUSB_STATIC int py_NumberAsInt(
	PyObject *obj
	)
{
	PyObject *number = PyNumber_Int(obj);
	if (number) {
		int ret = PyInt_AS_LONG(number);
		Py_DECREF(number);
		return ret;
	} else {
		return 0;
	}
}

/*
 * Gets a byte from a PyObject
 * If the obj is a number, returns the number casted to a byte
 * If it is a sequence, returns the first byte representation of the sequence
 * If it is a mapping, returns the first byte representation of the first obj.values()
 */
PYUSB_STATIC u_int8_t getByte(
	PyObject *obj
	)
{
	u_int8_t byte;

	if (PyNumber_Check(obj)) {
		byte = (u_int8_t) py_NumberAsInt(obj);
	} else if (PyString_Check(obj) || PyUnicode_Check(obj)) {
		return (u_int8_t) PyString_AsString(obj)[0];
	} else if (PySequence_Check(obj)) {
		PyObject *el0;
		el0 = PySequence_GetItem(obj, 0);
		if (!el0) return 0;
		byte = getByte(el0);
		Py_DECREF(el0);
	} else if (PyMapping_Check(obj)) {
		PyObject *vals;
		vals = PyMapping_Values(obj);
		if (!vals) return 0;
		byte = getByte(vals);
		Py_DECREF(vals);
	} else {
		byte = 0;
		PyErr_BadArgument();
	}

	return byte;
}

/*
 * Gets a unsigned byte * representation of the obj
 * If the obj is a sequence, returns the elements as a c byte array representation
 * If it is a mapping, returns the c byte array representations of the obj.values()
 */
PYUSB_STATIC char *getBuffer(
	PyObject *obj,
	int *size
	)
{
	char *p = NULL;

	/*
	 * When the obj is a sequence type, we take the first byte from
	 * the each element of the sequence
	 */

	/* ok, string is a sequence too, but let us optimize it */
	if (PyString_Check(obj) || PyUnicode_Check(obj)) { 
		char *tmp;

		if (-1 != PyString_AsStringAndSize(obj, &tmp, size)) {
			p = (char *) PyMem_Malloc(*size);
			if (p) memcpy(p, tmp, *size);
		}
	} else if (PySequence_Check(obj)) {
		u_int32_t i, sz;
		PyObject *el;

		sz = PySequence_Size(obj);
		p = (char *) PyMem_Malloc(sz);

		for (i = 0; i < sz; ++i) {
			el = PySequence_GetItem(obj, i);
			p[i] = getByte(el);
			Py_DECREF(el);

			if (!p[i] && PyErr_Occurred()) {
				PyMem_Free(p);
				return NULL;
			}
		}

		*size = sz;
	} else if (PyMapping_Check(obj)) {
		PyObject *values;

		values = PyMapping_Values(obj);
		if (!values) return NULL;

		p = getBuffer(values, size);
		Py_DECREF(values);
	} else if (obj == Py_None) {
		*size = 0;
		return NULL;
	} else {
		PyErr_BadArgument();
		return NULL;
	}

	return p;
}

/*
 * Build a numeric tuple from the buffer values
 */
PYUSB_STATIC PyObject *buildTuple(
	char *buffer,
	int size
	)
{
	PyObject *ret;
	int i;

	ret = PyTuple_New(size);

	if (ret) {
		for (i = 0; i < size; ++i) {
			PyTuple_SET_ITEM(ret, i, PyInt_FromLong((u_int8_t) buffer[i]));
		}
	}

	return ret;
}

/*
 * Add a numeric constant to the dictionary
 */
PYUSB_STATIC void addConstant(
	PyObject *dict,
	const char *name,
	long value)
{
	PyObject *val;

	val = PyInt_FromLong(value); 

	if (val) { 
		PyDict_SetItemString(dict,  name, val);
		Py_DECREF(val); 
	} 
}

/*
 * Add the module constants to the module dictionary
 */
PYUSB_STATIC void installModuleConstants(
	PyObject *module
	)
{
	PyObject *dict;

	dict = PyModule_GetDict(module);

	addConstant(dict, "CLASS_PER_INTERFACE", USB_CLASS_PER_INTERFACE);
	addConstant(dict, "CLASS_AUDIO", USB_CLASS_AUDIO);
	addConstant(dict, "CLASS_COMM", USB_CLASS_COMM);
	addConstant(dict, "CLASS_HID", USB_CLASS_HID);
	addConstant(dict, "CLASS_PRINTER", USB_CLASS_PRINTER);
	addConstant(dict, "CLASS_MASS_STORAGE", USB_CLASS_MASS_STORAGE);
	addConstant(dict, "CLASS_HUB", USB_CLASS_HUB);
	addConstant(dict, "CLASS_DATA", USB_CLASS_DATA);
	addConstant(dict, "CLASS_VENDOR_SPEC", USB_CLASS_VENDOR_SPEC);
	addConstant(dict, "DT_DEVICE", USB_DT_DEVICE);
	addConstant(dict, "DT_CONFIG", USB_DT_CONFIG);
	addConstant(dict, "DT_STRING", USB_DT_STRING);
	addConstant(dict, "DT_INTERFACE", USB_DT_INTERFACE);
	addConstant(dict, "DT_ENDPOINT", USB_DT_ENDPOINT);
	addConstant(dict, "DT_HID", USB_DT_HID);
	addConstant(dict, "DT_REPORT", USB_DT_REPORT);
	addConstant(dict, "DT_PHYSICAL", USB_DT_PHYSICAL);
	addConstant(dict, "DT_HUB", USB_DT_HUB);
	addConstant(dict, "DT_DEVICE_SIZE", USB_DT_DEVICE_SIZE);
	addConstant(dict, "DT_CONFIG_SIZE", USB_DT_CONFIG_SIZE);
	addConstant(dict, "DT_INTERFACE_SIZE", USB_DT_INTERFACE_SIZE);
	addConstant(dict, "DT_ENDPOINT_SIZE", USB_DT_ENDPOINT_SIZE);
	addConstant(dict, "DT_ENDPOINT_AUDIO_SIZE", USB_DT_ENDPOINT_AUDIO_SIZE);
	addConstant(dict, "DT_HUB_NONVAR_SIZE", USB_DT_HUB_NONVAR_SIZE);
	addConstant(dict, "MAXENDPOINTS", USB_MAXENDPOINTS);
	addConstant(dict, "ENDPOINT_ADDRESS_MASK", USB_ENDPOINT_ADDRESS_MASK);
	addConstant(dict, "ENDPOINT_DIR_MASK", USB_ENDPOINT_DIR_MASK);
	addConstant(dict, "ENDPOINT_TYPE_MASK", USB_ENDPOINT_TYPE_MASK);
	addConstant(dict, "ENDPOINT_TYPE_CONTROL", USB_ENDPOINT_TYPE_CONTROL);
	addConstant(dict, "ENDPOINT_TYPE_ISOCHRONOUS", USB_ENDPOINT_TYPE_ISOCHRONOUS);
	addConstant(dict, "ENDPOINT_TYPE_BULK", USB_ENDPOINT_TYPE_BULK);
	addConstant(dict, "ENDPOINT_TYPE_INTERRUPT", USB_ENDPOINT_TYPE_INTERRUPT);
	addConstant(dict, "MAXINTERFACES", USB_MAXINTERFACES);
	addConstant(dict, "MAXALTSETTING", USB_MAXALTSETTING);
	addConstant(dict, "MAXCONFIG", USB_MAXCONFIG);
	addConstant(dict, "REQ_GET_STATUS", USB_REQ_GET_STATUS);
	addConstant(dict, "REQ_CLEAR_FEATURE", USB_REQ_CLEAR_FEATURE);
	addConstant(dict, "REQ_SET_FEATURE", USB_REQ_SET_FEATURE);
	addConstant(dict, "REQ_SET_ADDRESS", USB_REQ_SET_ADDRESS);
	addConstant(dict, "REQ_GET_DESCRIPTOR", USB_REQ_GET_DESCRIPTOR);
	addConstant(dict, "REQ_SET_DESCRIPTOR", USB_REQ_SET_DESCRIPTOR);
	addConstant(dict, "REQ_GET_CONFIGURATION", USB_REQ_GET_CONFIGURATION);
	addConstant(dict, "REQ_SET_CONFIGURATION", USB_REQ_SET_CONFIGURATION);
	addConstant(dict, "REQ_GET_INTERFACE", USB_REQ_GET_INTERFACE);
	addConstant(dict, "REQ_SET_INTERFACE", USB_REQ_SET_INTERFACE);
	addConstant(dict, "REQ_SYNCH_FRAME", USB_REQ_SYNCH_FRAME);
	addConstant(dict, "TYPE_STANDARD", USB_TYPE_STANDARD);
	addConstant(dict, "TYPE_CLASS", USB_TYPE_CLASS);
	addConstant(dict, "TYPE_VENDOR", USB_TYPE_VENDOR);
	addConstant(dict, "TYPE_RESERVED", USB_TYPE_RESERVED);
	addConstant(dict, "RECIP_DEVICE", USB_RECIP_DEVICE);
	addConstant(dict, "RECIP_INTERFACE", USB_RECIP_INTERFACE);
	addConstant(dict, "RECIP_ENDPOINT", USB_RECIP_ENDPOINT);
	addConstant(dict, "RECIP_OTHER", USB_RECIP_OTHER);
	addConstant(dict, "ENDPOINT_IN", USB_ENDPOINT_IN);
	addConstant(dict, "ENDPOINT_OUT", USB_ENDPOINT_OUT);
	addConstant(dict, "ERROR_BEGIN", USB_ERROR_BEGIN);
}

/*
 * Earlier versions of the PyUSB separate direction bit and
 * endpoint address with direction and address fields...
 * Windows version of the libusb does need direction
 * bit in endpoint address to works fine.
 * Thanks to Ray Schumacher.
 */
PYUSB_STATIC PyMemberDef Py_usb_Endpoint_Members[] = {
	{"address",
	 T_UBYTE,
	 offsetof(Py_usb_Endpoint, address),
	 READONLY,
	 "Contains the endpoint address."},

	{"type",
	 T_UBYTE,
	 offsetof(Py_usb_Endpoint, type),
	 READONLY,
	 "It contains one of the following values, \n"
	 "indicating the endpoint transfer type:\n"
	 "\tENDPOINT_TYPE_CONTROL\n"
	 "\tENDPOINT_TYPE_ISOCHRONOUS\n"
	 "\tENDPOINT_TYPE_BULK\n"
	 "\tENDPOINT_TYPE_INTERRUPT\n"},

	{"maxPacketSize",
	 T_USHORT,
	 offsetof(Py_usb_Endpoint, maxPacketSize),
	 READONLY,
	 "The maximum number of data bytes the endpoint\n"
	 "can transfer in a transaction."},

	{"interval",
	 T_UBYTE,
	 offsetof(Py_usb_Endpoint, interval),
	 READONLY,
	 "The maximum latency for polling interrupt endpoints, or\n"
	 "the interval for polling isochronous endpoints, or the maximum NAK\n"
	 "rate for high-speed bulk OUT or control endpoints."},

	{NULL}
};

PYUSB_STATIC PyMethodDef Py_usb_Endpoint_Methods[] = {
	{NULL, NULL}
};

PYUSB_STATIC PyTypeObject Py_usb_Endpoint_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "usb.Endpoint",  /*tp_name*/
    sizeof(Py_usb_Endpoint), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Endpoint descriptor object", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Py_usb_Endpoint_Methods,   /* tp_methods */
    Py_usb_Endpoint_Members,   /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,					       /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,						   /* destructor */
};

PYUSB_STATIC void set_Endpoint_fields(
	Py_usb_Endpoint *endpoint,
	struct usb_endpoint_descriptor *ep
	)
{
	endpoint->address = ep->bEndpointAddress;
	endpoint->type = ep->bmAttributes & 3;
	endpoint->maxPacketSize = ep->wMaxPacketSize;
	endpoint->interval = ep->bInterval;
	endpoint->refresh = ep->bRefresh;
	// endpoint->synchAddress - ep->bSynchAddress;
}

PYUSB_STATIC Py_usb_Endpoint *new_Endpoint(
	struct usb_endpoint_descriptor *ep
	)
{
	Py_usb_Endpoint *endpoint;

	endpoint = PyObject_New(Py_usb_Endpoint,
							&Py_usb_Endpoint_Type);

	if (endpoint && ep)
		set_Endpoint_fields(endpoint, ep);

	return endpoint;
}

PYUSB_STATIC PyMemberDef Py_usb_Interface_Members[] = {
	{"interfaceNumber",
	 T_UBYTE,
	 offsetof(Py_usb_Interface, interfaceNumber),
	 READONLY,
	 "Identifies the interface."},

	{"alternateSetting",
	 T_UBYTE,
	 offsetof(Py_usb_Interface, alternateSetting),
	 READONLY,
	 "Alternate setting number."},

	{"interfaceClass",
	 T_UBYTE,
	 offsetof(Py_usb_Interface, interfaceClass),
	 READONLY,
	 "Similar to DeviceClass in the device descriptor, but\n"
	 "for devices with a class specified by the interface."},

	{"interfaceSubClass",
	 T_UBYTE,
	 offsetof(Py_usb_Interface, interfaceSubClass),
	 READONLY,
	 "Similar to bDeviceSubClass in the device\n"
	 "descriptor, but for devices with a class defined by the interface."},

	{"interfaceProtocol",
	 T_UBYTE,
	 offsetof(Py_usb_Interface, interfaceProtocol),
	 READONLY,
	 "Similar to bDeviceProtocol in the device\n"
	 "descriptor, but for devices whose class is defined by the interface."},

	{"endpoints",
	 T_OBJECT,
	 offsetof(Py_usb_Interface, endpoints),
	 READONLY,
	 "Tuple with interface endpoints."},
	
	{NULL}
};

PYUSB_STATIC void Py_usb_Interface_del(
	PyObject *self
	)
{
	Py_XDECREF((PyObject *) ((Py_usb_Interface *) self)->endpoints);
	PyObject_Del(self);
}

PYUSB_STATIC PyMethodDef Py_usb_Interface_Methods[] = {
	{NULL}
};

PYUSB_STATIC PyTypeObject Py_usb_Interface_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "usb.Interface",    	   /*tp_name*/
    sizeof(Py_usb_Interface),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    Py_usb_Interface_del,     /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
	0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Interface descriptor object", 	   /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Py_usb_Interface_Methods,  /* tp_methods */
    Py_usb_Interface_Members,  /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,					       /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0 							/* destructor */
};

PYUSB_STATIC void set_Interface_fields(
	Py_usb_Interface *interface,
	struct usb_interface_descriptor *i
	)
{
	u_int8_t index;

	interface->interfaceNumber = i->bInterfaceNumber;
	interface->alternateSetting = i->bAlternateSetting;
	interface->interfaceClass = i->bInterfaceClass;
	interface->interfaceSubClass = i->bInterfaceSubClass;
	interface->interfaceProtocol = i->bInterfaceProtocol;
	interface->iInterface = i->iInterface;

	interface->endpoints = PyTuple_New(i->bNumEndpoints);

	if (!interface->endpoints) {
		return;
	}

	for (index = 0; index < i->bNumEndpoints; ++index)
		PyTuple_SET_ITEM(interface->endpoints, index,
						(PyObject *) new_Endpoint(i->endpoint+index));
}

PYUSB_STATIC Py_usb_Interface *new_Interface(
	struct usb_interface_descriptor *i
	)
{
	Py_usb_Interface *interface;

	interface = PyObject_NEW(Py_usb_Interface, &Py_usb_Interface_Type);

	if (interface) {
		set_Interface_fields(interface, i);

		if (PyErr_Occurred()) {
			Py_XDECREF((PyObject *) interface);
			return NULL;
		}
	}

	return interface;
}

PYUSB_STATIC PyMemberDef Py_usb_Configuration_Members[] = {
	{"totalLength",
	 T_USHORT,
	 offsetof(Py_usb_Configuration, totalLength),
	 READONLY,
	 "The number of data bytes that the device returns,\n"
	 "including the bytes for all of the configuration's interfaces and\n"
	 "endpoints."},

	{"value",
	 T_UBYTE,
	 offsetof(Py_usb_Configuration, value),
	 READONLY,
	 "Identifies the configuration."},

	{"selfPowered",
	 T_UBYTE,
	 offsetof(Py_usb_Configuration, selfPowered),
	 READONLY,
	 "True if the device is self powered."},

	{"remoteWakeup",
	 T_UBYTE,
	 offsetof(Py_usb_Configuration, remoteWakeup),
	 READONLY,
	 "True if the device supports remote wakeup feature."},

	{"maxPower",
	 T_UBYTE,
	 offsetof(Py_usb_Configuration, maxPower),
	 READONLY,
	 "Specifies the device current. This is the absolute value,\n"
	 "already multiplied by 2"},

	{"interfaces",
	 T_OBJECT,
	 offsetof(Py_usb_Configuration, interfaces),
	 READONLY,
	 "Tuple with a tuple of the configuration interfaces.\n"
	 "Each element represents a sequence of the\n"
	 "alternate settings for each interface."},

	{"iConfiguration",
	 T_UBYTE,
	 offsetof(Py_usb_Configuration, iConfiguration),
	 RO,
	 "Index to a string that describes the\n"
	 "configuration."},

	{NULL}
};

PYUSB_STATIC PyMethodDef Py_usb_Configuration_Methods[] = {
	{NULL, NULL}
};

PYUSB_STATIC void Py_usb_Configuration_del(
	PyObject *self
	)
{
	Py_XDECREF((PyObject *) ((Py_usb_Configuration *) self)->interfaces);
	PyObject_Del(self);;
}

PYUSB_STATIC PyTypeObject Py_usb_Configuration_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "usb.Configuration",   	   /*tp_name*/
    sizeof(Py_usb_Configuration),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    Py_usb_Configuration_del,  /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
	0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Configuration descriptor object",    /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Py_usb_Configuration_Methods,  /* tp_methods */
    Py_usb_Configuration_Members,  /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,					       /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0			/* destructor */
};

PYUSB_STATIC void set_Configuration_fields(
	Py_usb_Configuration *configuration,
	struct usb_config_descriptor *config
	)
{
	u_int8_t i, j, k;
	PyObject *t1;

	configuration->totalLength = config->wTotalLength;
	configuration->value = config->bConfigurationValue;
	configuration->iConfiguration = config->iConfiguration;
	configuration->selfPowered = (config->bmAttributes >> 6) & 1;
	configuration->remoteWakeup = (config->bmAttributes >> 5) & 1;
	configuration->maxPower = config->MaxPower << 2;

	configuration->interfaces = PyTuple_New(config->bNumInterfaces);

	if (!configuration->interfaces) return;

	for (i = 0; i < config->bNumInterfaces; ++i) {
		k = config->interface[i].num_altsetting;
		t1 = PyTuple_New(k);

		if (!t1) return;

		for (j = 0; j < k; ++j)
			PyTuple_SET_ITEM(t1, j,
					(PyObject *) new_Interface(config->interface[i].altsetting+j));

		PyTuple_SET_ITEM(configuration->interfaces, i, t1);
	}
}

PYUSB_STATIC Py_usb_Configuration *new_Configuration(
	struct usb_config_descriptor *conf
	)
{
	Py_usb_Configuration *configuration;

	configuration = PyObject_NEW(Py_usb_Configuration, &Py_usb_Configuration_Type);

	if (configuration) {
		set_Configuration_fields(configuration, conf);

		if (PyErr_Occurred()) {
			Py_DECREF((PyObject *) configuration);
			return NULL;
		}
	}

	return configuration;
}

PYUSB_STATIC PyMemberDef Py_usb_Device_Members[] = {
	{"usbVersion",
	 T_STRING_INPLACE,
	 offsetof(Py_usb_Device, usbVersion),
	 READONLY,
	 "String containing the USB specification number that\n"
	 "the device and its descriptors comply with."},

	{"deviceClass",
	 T_UBYTE,
	 offsetof(Py_usb_Device, deviceClass),
	 READONLY,
	 "For devices that belong to a class, this field may\n"
	 "name the class."},

	{"deviceSubClass",
	 T_UBYTE,
	 offsetof(Py_usb_Device, deviceSubClass),
	 READONLY,
	 "For devices that belong to a class, this field may\n"
	 "specify a subclass within the class."},

	{"deviceProtocol",
	 T_UBYTE,
	 offsetof(Py_usb_Device, deviceProtocol),
	 READONLY,
	 "This field may specify a protocol defined by the\n"
	 "selected class or subclass."},
	
	{"maxPacketSize",
	 T_UBYTE,
	 offsetof(Py_usb_Device, maxPacketSize),
	 READONLY,
	 "The maximum packet size for Endpoint 0."},

	{"idVendor",
	 T_USHORT,
	 offsetof(Py_usb_Device, idVendor),
	 READONLY,
	 "Unique vendor identifier."},

	{"idProduct",
	 T_USHORT,
	 offsetof(Py_usb_Device, idProduct),
	 READONLY,
	 "The manufacturer assigns a Product ID to identify the\n"
	 "device."},

	{"deviceVersion",
	 T_STRING_INPLACE,
	 offsetof(Py_usb_Device, deviceVersion),
	 READONLY,
	 "String containing the device's release number."},

	{"filename",
	 T_STRING_INPLACE,
	 offsetof(Py_usb_Device, filename),
	 READONLY,
	 ""},
	
	{"configurations",
	 T_OBJECT,
	 offsetof(Py_usb_Device, configurations),
	 READONLY,
	 "Tuple with the device configurations."},

	{"iManufacturer",
	 T_UBYTE,
	 offsetof(Py_usb_Device, iManufacturer),
	 RO,
	 "An index that points to a string describing the\n"
	 "manufacturer."},

	{"iProduct",
	 T_UBYTE,
	 offsetof(Py_usb_Device, iProduct),
	 RO,
	 "An index that points to a string describing the product."},

	{"iSerialNumber",
	 T_UBYTE,
	 offsetof(Py_usb_Device, iSerialNumber),
	 RO,
	 "An index that points to a string containing the\n"
	 "device's serial number."},

	{NULL}
};

PYUSB_STATIC PyObject *Py_usb_Device_open(
	PyObject *self,
	PyObject *args
	)
{
	return (PyObject *) new_DeviceHandle((Py_usb_Device *) self);
}

PYUSB_STATIC PyMethodDef Py_usb_Device_Methods[] = {
	{"open",
	 Py_usb_Device_open,
	 METH_NOARGS,
	 "open() -> DeviceHandle\n\n"
	 "Open the device for use.\n"
	 "Returns a DeviceHandle object."},

	{NULL, NULL}
};

PYUSB_STATIC void Py_usb_Device_del(
	PyObject *self
	)
{
	Py_XDECREF(((Py_usb_Device *) self)->configurations);
	PyObject_Del(self);;
}

PYUSB_STATIC PyTypeObject Py_usb_Device_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "usb.Device",   	   	   /*tp_name*/
    sizeof(Py_usb_Device),     /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    Py_usb_Device_del,         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
	0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Device descriptor object",    	   /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Py_usb_Device_Methods,     /* tp_methods */
    Py_usb_Device_Members,	   /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,					       /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0					/* destructor */
};

PYUSB_STATIC void set_Device_fields(
	Py_usb_Device *device,
	struct usb_device *dev
	)
{
	struct usb_device_descriptor *desc = &dev->descriptor;
	u_int8_t i;

	device->usbVersion[0] = ((desc->bcdUSB >> 12) & 0xf) + '0';
	device->usbVersion[1] = ((desc->bcdUSB >> 8) & 0xf) + '0';
	device->usbVersion[2] = '.';
	device->usbVersion[3] = ((desc->bcdUSB >> 4) & 0xf) + '0';
	device->usbVersion[4] = (desc->bcdUSB & 0xf) + '0';
	device->usbVersion[5] = '\0';

	device->deviceVersion[0] = ((desc->bcdDevice >> 12) & 0xf) + '0';
	device->deviceVersion[1] = ((desc->bcdDevice >> 8) & 0xf) + '0';
	device->deviceVersion[2] = '.';
	device->deviceVersion[3] = ((desc->bcdDevice >> 4) & 0xf) + '0';
	device->deviceVersion[4] = (desc->bcdDevice & 0xf) + '0';
	device->deviceVersion[5] = '\0';

	device->deviceClass = desc->bDeviceClass;
	device->deviceSubClass = desc->bDeviceSubClass;
	device->deviceProtocol = desc->bDeviceProtocol;
	device->maxPacketSize = desc->bMaxPacketSize0;
	device->idVendor = desc->idVendor;
	device->idProduct = desc->idProduct;
	device->iManufacturer = desc->iManufacturer;
	device->iProduct = desc->iProduct;
	device->iSerialNumber = desc->iSerialNumber;
	strcpy(device->filename, dev->filename);
	device->dev = dev;

	if (!dev->config) {
		device->configurations = PyTuple_New(0);
		return;
	}

	device->configurations = PyTuple_New(desc->bNumConfigurations);

	if (!device->configurations) return;

	for (i = 0; i < desc->bNumConfigurations; ++i)
		PyTuple_SET_ITEM(device->configurations, i, (PyObject *) new_Configuration(dev->config+i));
}

PYUSB_STATIC Py_usb_Device *new_Device(
	struct usb_device *dev
	)
{
	Py_usb_Device *device;

	device = PyObject_NEW(Py_usb_Device, &Py_usb_Device_Type);

	if (device) {
		set_Device_fields(device, dev);

		if (PyErr_Occurred()) {
			Py_DECREF((PyObject *) device);
			return NULL;
		}
	}

	return device;
}

PYUSB_STATIC PyMemberDef Py_usb_Bus_Members[] = {
	{"dirname",
	 T_STRING_INPLACE,
	 offsetof(Py_usb_Bus, dirname),
	 READONLY,
	 ""},

	{"location",
	 T_UINT,
	 offsetof(Py_usb_Bus, location),
	 READONLY,
	 ""},

	{"devices",
	 T_OBJECT,
	 offsetof(Py_usb_Bus, devices),
	 READONLY,
	 "Tuple with the bus devices"},

	{NULL}
};

PYUSB_STATIC PyMethodDef Py_usb_Bus_Methods[] = {
	{NULL, NULL}
};

PYUSB_STATIC void Py_usb_Bus_del(
	PyObject *self
	)
{
	Py_XDECREF(((Py_usb_Bus *) self)->devices);
	PyObject_Del(self);
}

PYUSB_STATIC PyTypeObject Py_usb_Bus_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "usb.Bus",   	   	   	   /*tp_name*/
    sizeof(Py_usb_Bus),        /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    Py_usb_Bus_del,            /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
	0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Bus object",    	  	   /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Py_usb_Bus_Methods,        /* tp_methods */
    Py_usb_Bus_Members,	   	   /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,					       /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0							/* destructor */
};

PYUSB_STATIC Py_usb_Bus *new_Bus(
	struct usb_bus *b
	)
{
	Py_usb_Bus *bus;
	u_int32_t i;
	struct usb_device *dev;

	bus = PyObject_NEW(Py_usb_Bus, &Py_usb_Bus_Type);

	if (bus) {
		bus->location = b->location;
		strcpy(bus->dirname, b->dirname);
		for(dev = b->devices, i = 0; dev; dev = dev->next) ++i;
		bus->devices = PyTuple_New(i);

		if(!bus->devices) {
			Py_DECREF((PyObject *) bus);
			return NULL;
		}

		for(dev = b->devices, i=0; dev; dev = dev->next, ++i)
			PyTuple_SET_ITEM(bus->devices, i, (PyObject *) new_Device(dev));

		if (PyErr_Occurred()) {
			Py_DECREF((PyObject *) bus);
			bus = NULL;
		}
	}

	return bus;
}

PYUSB_STATIC PyMemberDef Py_usb_DeviceHandle_Members[] = {
	{NULL}
};

/*
 * def controlMsg(requestType, request, buffer, value = 0, index = 0, timeout = 100)
 */
PYUSB_STATIC PyObject *Py_usb_DeviceHandle_controlMsg(
	PyObject *self,
	PyObject *args,
	PyObject *kwds
	)
{
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;
	int requestType;
	int request;
	int value = 0;
	int index = 0;
	char *bytes;
	PyObject *data;
	int size;
	int timeout = DEFAULT_TIMEOUT;
	int ret;
	int as_read = 0;

	static char *kwlist[] = {
		"requestType",
		"request",
		"buffer",
		"value",
		"index",
		"timeout",
		NULL
	};

	if (!PyArg_ParseTupleAndKeywords(args,
									 kwds,
									 "iiO|iii",
									 kwlist,
									 &requestType,
									 &request,
									 &data,
									 &value,
									 &index,
									 &timeout)) {
		return NULL;
	}

	/*
	 * If is a number, should be a read operation...
	 */
	if (PyNumber_Check(data)) {
		size = py_NumberAsInt(data);
		if (PyErr_Occurred()) return NULL;
		bytes = (char *) PyMem_Malloc(size);
		if (!bytes) return NULL;
		as_read = 1;
	} else {
		bytes = (char *) getBuffer(data, &size);
		if (PyErr_Occurred()) return NULL;
	}


#if DUMP_PARAMS

	fprintf(stderr, "controlMsg params:\n"
		   "\trequestType: %d\n"
		   "\trequest: %d\n"
		   "\tvalue: %d\n"
		   "\tindex: %d\n"
		   "\ttimeout: %d\n",
		   requestType,
		   request,
		   value,
		   index,
		   timeout);

	if (as_read) {
		fprintf(stderr, "\tbuffer: %d\n", size);
	} else {
		fprintf(stderr, "controlMsg buffer param:\n");
		printBuffer(bytes, size);
	}

#endif /* DUMP_PARAMS */

	Py_BEGIN_ALLOW_THREADS
	ret = usb_control_msg(_self->deviceHandle,
						  requestType,
						  request,
						  value,
						  index,
						  bytes,
						  size,
						  timeout);
	Py_END_ALLOW_THREADS

	if (ret < 0) {
		PyMem_Free(bytes);
		PyUSB_Error();
		return NULL;
	} else if (as_read) {
		PyObject *retObj = buildTuple(bytes, ret);
		PyMem_Free(bytes);
		return retObj;
	} else {
		PyMem_Free(bytes);
		return PyInt_FromLong(ret);
	}
}

/*
 * def setConfiguration(configuration)
 */
PYUSB_STATIC PyObject *Py_usb_DeviceHandle_setConfiguration(
	PyObject *self,
	PyObject *args
	)
{
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;
	int configuration;
	int ret;

	if (SUPPORT_NUMBER_PROTOCOL(args)) {
		configuration = (int) PyInt_AS_LONG(args);
	} else if (PyObject_TypeCheck(args, &Py_usb_Configuration_Type)) {
		configuration = ((Py_usb_Configuration *) args)->value;
	} else {
		PyErr_BadArgument();
		return NULL;
	}

#if DUMP_PARAMS
	
	fprintf(stderr,
			"setConfiguration params:\n\tconfiguration: %d\n",
			configuration);

#endif /* DUMP_PARAMS */

	Py_BEGIN_ALLOW_THREADS
	ret = usb_set_configuration(_self->deviceHandle, configuration);
	Py_END_ALLOW_THREADS

	if (ret < 0) {
		PyUSB_Error();
		return NULL;
	} else {
		Py_RETURN_NONE;
	}
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_claimInterface(
	PyObject *self,
	PyObject *args
	)
{
	int interfaceNumber;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (SUPPORT_NUMBER_PROTOCOL(args)) {
		interfaceNumber = py_NumberAsInt(args);
		if (PyErr_Occurred()) return NULL;
	} else if (PyObject_TypeCheck(args, &Py_usb_Interface_Type)) {
		interfaceNumber = ((Py_usb_Interface *) args)->interfaceNumber;
	} else {
		PyErr_BadArgument();
		return NULL;
	}

#if DUMP_PARAMS

	fprintf(stderr,
			"claimInterface params:\n\tinterfaceNumber: %d\n",
			interfaceNumber);

#endif /* DUMP_PARAMS */

	if (usb_claim_interface(_self->deviceHandle, interfaceNumber)) {
		PyUSB_Error();
		return NULL;
	} else {
		_self->interfaceClaimed = interfaceNumber;
	}

	Py_RETURN_NONE;
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_detachKernelDriver(
	PyObject *self,
	PyObject *args
	)
{
	int interfaceNumber;
	int ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (SUPPORT_NUMBER_PROTOCOL(args)) {
		interfaceNumber = py_NumberAsInt(args);
		if (PyErr_Occurred()) return NULL;
	} else if (PyObject_TypeCheck(args, &Py_usb_Interface_Type)) {
		interfaceNumber = ((Py_usb_Interface *) args)->interfaceNumber;
	} else {
		PyErr_BadArgument();
		return NULL;
	}

#if DUMP_PARAMS

	fprintf(stderr,
			"detachKernelDriver params:\n\tinterfaceNumber: %d\n",
			interfaceNumber);

#endif /* DUMP_PARAMS */
	
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
	Py_BEGIN_ALLOW_THREADS
	ret = usb_detach_kernel_driver_np(_self->deviceHandle, interfaceNumber);
	Py_END_ALLOW_THREADS

	if (ret) {
		PyUSB_Error();
		return NULL;
	} 
#endif

	Py_RETURN_NONE;
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_releaseInterface(
	PyObject *self,
	PyObject *args
	)
{
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (-1 != _self->interfaceClaimed) {
		int ret;
		Py_BEGIN_ALLOW_THREADS
		ret = usb_release_interface(_self->deviceHandle, _self->interfaceClaimed);
		Py_END_ALLOW_THREADS

		if (ret) {
			PyUSB_Error();
			return NULL;
		} else {
			_self->interfaceClaimed = -1;
		}
	} else {
		PyErr_SetString(PyExc_ValueError, "No interface claimed");
		return NULL;
	}

	Py_RETURN_NONE;
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_setAltInterface(
	PyObject *self,
	PyObject *args
	)
{
	int altInterface, ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (SUPPORT_NUMBER_PROTOCOL(args)) {
		altInterface = (int) py_NumberAsInt(args);
		if (PyErr_Occurred()) return NULL;
	} else if (PyObject_TypeCheck(args, &Py_usb_Interface_Type)) {
		altInterface = ((Py_usb_Interface *) args)->alternateSetting;
	} else {
		PyErr_BadArgument();
		return NULL;
	}

#if DUMP_PARAMS

	fprintf(stderr,
			"setAltInterface params:\n\taltInterface: %d\n",
			altInterface);

#endif /* DUMP_PARAMS */

	Py_BEGIN_ALLOW_THREADS
	ret = usb_set_altinterface(_self->deviceHandle, altInterface);
	Py_END_ALLOW_THREADS

	if (ret < 0) {
		PyUSB_Error();
		return NULL;
	} else {
		Py_RETURN_NONE;
	}
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_bulkWrite(
	PyObject *self,
	PyObject *args
	)
{
	int endpoint;
	int timeout = DEFAULT_TIMEOUT;
	char *data;
	int size;
	PyObject *bytes;
	int ret;
	PyObject *retObj;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (!PyArg_ParseTuple(args,
						  "iO|i",
						  &endpoint,
						  &bytes,
						  &timeout)) {
		return NULL;
	}

	data = getBuffer(bytes, &size);
	if (PyErr_Occurred()) return NULL;

#if DUMP_PARAMS

	fprintf(stderr,
			"bulkWrite params:\n"
			"\tendpoint: %d\n"
			"\ttimeout: %d\n",
			endpoint,
			timeout);

	fprintf(stderr, "bulkWrite buffer param:\n");
	printBuffer(data, size);

#endif /* DUMP_PARAMS */

	Py_BEGIN_ALLOW_THREADS
	ret = usb_bulk_write(_self->deviceHandle, endpoint, data, size, timeout);
	Py_END_ALLOW_THREADS

	PyMem_Free(data);

	if (ret < 0) {
		PyUSB_Error();
		return NULL;
	} else {
		retObj = PyInt_FromLong(ret);
	}

	return retObj;
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_bulkRead(
	PyObject *self,
	PyObject *args
	)
{
	int endpoint;
	int timeout = DEFAULT_TIMEOUT;
	char *buffer;
	int size;
	PyObject *ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (!PyArg_ParseTuple(args,
						 "ii|i",
						 &endpoint,
						 &size,
						 &timeout)) {
		return NULL;
	}

#if DUMP_PARAMS

	fprintf(stderr,
			"bulkRead params:\n"
			"\tendpoint: %d\n"
			"\tsize: %d\n"
			"\ttimeout: %d\n",
			endpoint,
			size,
			timeout);

#endif /* DUMP_PARAMS */

	buffer = (char *) PyMem_Malloc(size);
	if (!buffer) return NULL;

	Py_BEGIN_ALLOW_THREADS
	size = usb_bulk_read(_self->deviceHandle, endpoint, buffer, size, timeout);
	Py_END_ALLOW_THREADS

	if (size < 0) {
		PyMem_Free(buffer);
		PyUSB_Error();
		return NULL;
	} else {
		ret = buildTuple(buffer, size);
		PyMem_Free(buffer);
	}

	return ret;
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_interruptWrite(
	PyObject *self,
	PyObject *args
	)
{
	int endpoint;
	int timeout = DEFAULT_TIMEOUT;
	char *data;
	int size;
	PyObject *bytes;
	int ret;
	PyObject *retObj;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (!PyArg_ParseTuple(args,
						  "iO|i",
						  &endpoint,
						  &bytes,
						  &timeout)) {
		return NULL;
	}

	data = getBuffer(bytes, &size);
	if (PyErr_Occurred()) return NULL;

#if DUMP_PARAMS

	fprintf(stderr,
			"interruptWrite params:\n"
			"\tendpoint: %d\n"
			"\ttimeout: %d\n",
			endpoint,
			timeout);

	fprintf(stderr, "interruptWrite buffer param:\n");
	printBuffer(data, size);

#endif /* DUMP_PARAMS */

	Py_BEGIN_ALLOW_THREADS
	ret = usb_interrupt_write(_self->deviceHandle, endpoint, data, size, timeout);
	Py_END_ALLOW_THREADS

	PyMem_Free(data);

	if (ret < 0) {
		PyUSB_Error();
		return NULL;
	} else {
		retObj = PyInt_FromLong(ret);
	}

	return retObj;
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_interruptRead(
	PyObject *self,
	PyObject *args
	)
{
	int endpoint;
	int timeout = DEFAULT_TIMEOUT;
	char *buffer;
	int size;
	PyObject *ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (!PyArg_ParseTuple(args,
						 "ii|i",
						 &endpoint,
						 &size,
						 &timeout)) {
		return NULL;
	}
	
#if DUMP_PARAMS

	fprintf(stderr,
			"interruptRead params:\n"
			"\tendpoint: %d\n"
			"\tsize: %d\n"
			"\ttimeout: %d\n",
			endpoint,
			size,
			timeout);

#endif /* DUMP_PARAMS */

	buffer = (char *) PyMem_Malloc(size);
	if (!buffer) return NULL;

	Py_BEGIN_ALLOW_THREADS
	size = usb_interrupt_read(_self->deviceHandle, endpoint, buffer, size, timeout);
	Py_END_ALLOW_THREADS

	if (size < 0) {
		PyMem_Free(buffer);
		PyUSB_Error();
		return NULL;
	} else {
		ret = buildTuple(buffer, size);
		PyMem_Free(buffer);
	}

	return ret;
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_resetEndpoint(
	PyObject *self,
	PyObject *args
	)
{
	int endpoint, ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	endpoint = py_NumberAsInt(args);
	if (PyErr_Occurred()) return NULL;

#if DUMP_PARAMS

	fprintf(stderr,
			"resetEndpoint params:\n"
			"\tendpoint: %d\n",
			endpoint);

#endif /* DUMP_PARAMS */
	Py_BEGIN_ALLOW_THREADS
	ret = usb_resetep(_self->deviceHandle, endpoint);
	Py_END_ALLOW_THREADS

	if (ret) {
		PyUSB_Error();
		return NULL;
	} else {
		Py_RETURN_NONE;
	}
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_reset(
	PyObject *self,
	PyObject *args
	)
{
	int ret;

	Py_BEGIN_ALLOW_THREADS
	ret = usb_reset(((Py_usb_DeviceHandle *) self)->deviceHandle);
	Py_END_ALLOW_THREADS

	if (ret) {
		PyUSB_Error();
		return NULL;
	} else {
		Py_RETURN_NONE;
	}
}

PYUSB_STATIC PyObject *Py_usb_DeviceHandle_clearHalt(
	PyObject *self,
	PyObject *args
	)
{
	int endpoint, ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	endpoint = py_NumberAsInt(args);
	if (PyErr_Occurred()) return NULL;

#if DUMP_PARAMS

	fprintf(stderr,
			"clearHalt params:\n"
			"\tendpoint: %d\n",
			endpoint);

#endif /* DUMP_PARAMS */

	Py_BEGIN_ALLOW_THREADS
	ret = usb_clear_halt(_self->deviceHandle, endpoint);
	Py_END_ALLOW_THREADS

	if (ret) {
		PyUSB_Error();
		return NULL;
	} else {
		Py_RETURN_NONE;
	}
}

/*
 * def getString(index, len, langid = -1)
 */
PYUSB_STATIC PyObject *Py_usb_DeviceHandle_getString(
	PyObject *self,
	PyObject *args
	)
{
	int langid=-1, index;
	unsigned long len;
	PyObject *retStr;
	char *buffer;
	int ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (!PyArg_ParseTuple(args,
						 "ik|i",
						 &index,
						 &len,
						 &langid)) {
		return NULL;
	}

#if DUMP_PARAMS

	fprintf(stderr,
			"getString params:\n"
			"\tindex: %d\n"
			"\tlen: %lu\n"
			"\tlangid: %d\n",
			index,
			len,
			langid);

#endif /* DUMP_PARAMS */

	++len;	/* for NULL termination */
	buffer = (char *) PyMem_Malloc(len);
	if (!buffer) return NULL;

	Py_BEGIN_ALLOW_THREADS

	if (-1 == langid) {
		ret = usb_get_string_simple(_self->deviceHandle, index, buffer, len);
	} else {
		ret = usb_get_string(_self->deviceHandle, index, langid, buffer, len);
	}

	Py_END_ALLOW_THREADS

	if (ret < 0) {
		PyMem_Free(buffer);
		PyUSB_Error();
		return NULL;
	}

	retStr = PyString_FromStringAndSize(buffer, ret);
	PyMem_Free(buffer);
	return retStr;
}

/*
 * def getDescriptor(type, index, len, endpoint = -1)
 */
PYUSB_STATIC PyObject *Py_usb_DeviceHandle_getDescriptor(
	PyObject *self,
	PyObject *args
	)
{
	int endpoint=-1, type, index;
	int len;
	PyObject *retSeq;
	char *buffer;
	int ret;
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;

	if (!PyArg_ParseTuple(args,
						 "iii|i",
						 &type,
						 &index,
						 &len,
						 &endpoint)) {
		return NULL;
	}

#if DUMP_PARAMS

	fprintf(stderr,
			"getDescriptor params:\n"
			"ttype: %d\n"
			"\tindex: %d\n"
			"\tlen: %d\n"
			"\tendpoint: %d\n",
			type,
			index,
			len,
			endpoint);

#endif /* DUMP_PARAMS */

	buffer = (char *) PyMem_Malloc(len);
	if (!buffer) return NULL;

	Py_BEGIN_ALLOW_THREADS

	if (-1 == endpoint) {
		ret = usb_get_descriptor(_self->deviceHandle, type, index, buffer, len);
	} else {
		ret = usb_get_descriptor_by_endpoint(_self->deviceHandle, endpoint, type, index, buffer, len);
	}

	Py_END_ALLOW_THREADS

	if (ret < 0) {
		PyMem_Free(buffer);
		PyUSB_Error();
		return NULL;
	}

	retSeq = buildTuple(buffer, ret);
	PyMem_Free(buffer);
	return retSeq;
}

PYUSB_STATIC PyMethodDef Py_usb_DeviceHandle_Methods[] = {
	{"controlMsg",
	 (PyCFunction) Py_usb_DeviceHandle_controlMsg,
	 METH_VARARGS | METH_KEYWORDS,
	 "controlMsg(requestType, request, buffer, value=0, index=0, timeout=100) -> bytesWritten|buffer\n\n"
	 "Performs a control request to the default control pipe on a device.\n"
	 "Arguments:\n"
	 "\trequestType: specifies the direction of data flow, the type\n"
	 "\t             of request, and the recipient.\n"
	 "\trequest: specifies the request.\n"
	 "\tbuffer: if the transfer is a write transfer, buffer is a sequence \n"
	 "\t        with the transfer data, otherwise, buffer is the number of\n"
	 "\t        bytes to read.\n"
	 "\tvalue: specific information to pass to the device. (default: 0)\n"
	 "\tindex: specific information to pass to the device. (default: 0)\n"
	 "\ttimeout: operation timeout in miliseconds. (default: 100)\n"
	 "Returns the number of bytes written."},

	{"setConfiguration",
	 Py_usb_DeviceHandle_setConfiguration,
 	 METH_O,
	 "setConfiguration(configuration) -> None\n\n"
	 "Sets the active configuration of a device.\n"
	 "Arguments:\n"
	 "\tconfiguration: a configuration value or a Configuration object."},	 

	{"claimInterface",
	 Py_usb_DeviceHandle_claimInterface,
	 METH_O,
	 "claimInterface(interface) -> None\n\n"
	 "Claims the interface with the Operating System.\n"
	 "Arguments:\n"
	 "\tinterface: interface number or an Interface object."},
	
	{"detachKernelDriver",
	 Py_usb_DeviceHandle_detachKernelDriver,
	 METH_O,
	 "detachKernelDriver(interface) -> None\n\n"
	 "Detaches a kernel driver from the interface (if one is attached,\n"
	 "we have permission and the operation is supported by the OS)\n"
	 "Arguments:\n"
	 "\tinterface: interface number or an Interface object."},

	{"releaseInterface",
	 Py_usb_DeviceHandle_releaseInterface,
	 METH_NOARGS,
	 "releaseInterface() -> None\n\n"
	 "Releases an interface previously claimed with claimInterface."},
	
	{"setAltInterface",
	 Py_usb_DeviceHandle_setAltInterface,
	 METH_O,
	 "setAltInterface(alternate) -> None\n\n"
	 "Sets the active alternate setting of the current interface.\n"
	 "Arguments:\n"
	 "\talternate: an alternate setting number or an Interface object."},

	{"bulkWrite",
	 Py_usb_DeviceHandle_bulkWrite,
	 METH_VARARGS,
	 "bulkWrite(endpoint, buffer, timeout=100) -> bytesWritten\n\n"
	 "Performs a bulk write request to the endpoint specified.\n"
	 "Arguments:\n"
	 "\tendpoint: endpoint number.\n"
	 "\tbuffer: sequence data buffer to write.\n"
	 "\t      This parameter can be any sequence type\n"
	 "\ttimeout: operation timeout in miliseconds. (default: 100)\n"
	 "Returns the number of bytes written."},

	{"bulkRead",
	 Py_usb_DeviceHandle_bulkRead,
	 METH_VARARGS,
	 "bulkRead(endpoint, size, timeout=100) -> buffer\n\n"
	 "Performs a bulk read request to the endpoint specified.\n"
	 "Arguments:\n"
	 "\tendpoint: endpoint number.\n"
	 "\tsize: number of bytes to read.\n"
	 "\ttimeout: operation timeout in miliseconds. (default: 100)\n"
	 "Returns a tuple with the data read."},

	{"interruptWrite",
	 Py_usb_DeviceHandle_interruptWrite,
	 METH_VARARGS,
	 "interruptWrite(endpoint, buffer, timeout=100) -> bytesWritten\n\n"
	 "Performs a interrupt write request to the endpoint specified.\n"
	 "Arguments:\n"
	 "\tendpoint: endpoint number.\n"
	 "\tbuffer: sequence data buffer to write.\n"
	 "\t      This parameter can be any sequence type\n"
	 "\ttimeout: operation timeout in miliseconds. (default: 100)\n"
	 "Returns the number of bytes written."},

	{"interruptRead",
	 Py_usb_DeviceHandle_interruptRead,
	 METH_VARARGS,
	 "interruptRead(endpoint, size, timeout=100) -> buffer\n\n"
	 "Performs a interrupt read request to the endpoint specified.\n"
	 "Arguments:\n"
	 "\tendpoint: endpoint number.\n"
	 "\tsize: number of bytes to read.\n"
	 "\ttimeout: operation timeout in miliseconds. (default: 100)\n"
	 "Returns a tuple with the data read."},

	{"resetEndpoint",
	 Py_usb_DeviceHandle_resetEndpoint,
	 METH_O,
	 "resetEndpoint(endpoint) -> None\n\n"
	 "Resets all state (like toggles) for the specified endpoint.\n"
	 "Arguments:\n"
	 "\tendpoint: endpoint number.\n"},

	{"reset",
	 Py_usb_DeviceHandle_reset,
	 METH_NOARGS,
	 "reset() -> None\n\n"
	 "Resets the specified device by sending a RESET\n"
	 "down the port it is connected to.\n"},

	{"clearHalt",
	 Py_usb_DeviceHandle_clearHalt,
	 METH_O,
	 "clearHalt(endpoint) -> None\n\n"
	 "Clears any halt status on the specified endpoint.\n"
	 "Arguments:\n"
	 "\tendpoint: endpoint number.\n"},

	{"getString",
	 Py_usb_DeviceHandle_getString,
	 METH_VARARGS,
	 "getString(index, len, langid = -1) -> string\n\n"
	 "Retrieves the string descriptor specified by index\n"
	 "and langid from a device.\n"
	 "Arguments:\n"
	 "\tindex: index of descriptor in the device.\n"
	 "\tlen: number of bytes of the string\n"
	 "\tlangid: Language ID. If it is omittedi, will be\n"
	 "\t        used the first language.\n"},

	{"getDescriptor",
	 Py_usb_DeviceHandle_getDescriptor,
	 METH_VARARGS,
	 "getDescriptor(type, index, len, endpoint = -1) -> descriptor\n\n"
	 "Retrieves a descriptor from the device identified by the type\n"
	 "and index of the descriptor.\n"
	 "Arguments:\n"
	 "\ttype: descriptor type.\n"
	 "\tindex: index of the descriptor.\n"
	 "\tlen: descriptor length.\n"
	 "\tendpoint: endpoint number from descriptor is read. If it is\n"
	 "\t          omitted, the descriptor is read from default control pipe.\n"},

	{NULL, NULL}
};

PYUSB_STATIC void Py_usb_DeviceHandle_del(
	PyObject *self
	)
{
	Py_usb_DeviceHandle *_self = (Py_usb_DeviceHandle *) self;
	struct usb_dev_handle *h = _self->deviceHandle;

	if (h) {
		if (-1 != _self->interfaceClaimed) {
			usb_release_interface(_self->deviceHandle, 
								  _self->interfaceClaimed);
		}

		usb_close(_self->deviceHandle);
	}

	PyObject_Del(self);
}

PYUSB_STATIC PyTypeObject Py_usb_DeviceHandle_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "usb.DeviceHandle",   	   /*tp_name*/
    sizeof(Py_usb_DeviceHandle), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    Py_usb_DeviceHandle_del,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
	0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "DeviceHandle object",     /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Py_usb_DeviceHandle_Methods, /* tp_methods */
    Py_usb_DeviceHandle_Members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,					       /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0						/* destructor */
};

PYUSB_STATIC Py_usb_DeviceHandle *new_DeviceHandle(
	Py_usb_Device *device
	)
{
	Py_usb_DeviceHandle *dh;
	struct usb_dev_handle *h;

	dh = PyObject_NEW(Py_usb_DeviceHandle, &Py_usb_DeviceHandle_Type);

	if (dh) {
		h = usb_open(device->dev);

		if (!h) {
			PyUSB_Error();
			Py_DECREF((PyObject *) dh);
			return NULL;
		}

		dh->deviceHandle = h;
		dh->interfaceClaimed = -1;
	}

	return dh;
}

/*
 * Global functions
 */

PYUSB_STATIC PyObject *busses(
	PyObject *self,
	PyObject *args
	)
{
	PyObject *tuple;
	struct usb_bus *bus, *b;
	u_int32_t i;

	if (usb_find_busses() < 0) {
		PyUSB_Error();
		return NULL;
	}

	if (usb_find_devices() < 0) {
		PyUSB_Error();
		return NULL;
	}

	bus = usb_get_busses();

	if (!bus) {
		PyUSB_Error();
		return NULL;
	}

	for(i=0,b=bus;b;b=b->next) ++i;
	tuple = PyTuple_New(i);
	if (!tuple) return NULL;

	for(b=bus,i=0;b;++i,b=b->next)
		PyTuple_SET_ITEM(tuple, i, (PyObject *) new_Bus(b));

	if (PyErr_Occurred()) {
		Py_DECREF(tuple);
		return NULL;
	}

	return tuple;
}

PYUSB_STATIC PyMethodDef usb_Methods[] = {
	{"busses", busses, METH_NOARGS, "Returns a tuple with the usb busses"},
	{NULL, NULL}
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif /* PyMODINIT_FUNC */

/*
 * Entry point for the module
 */
PyMODINIT_FUNC initusb(void)
{
	PyObject *module;

	module = Py_InitModule3("usb", usb_Methods,"USB access module");
	if (!module) return;

	PyExc_USBError = PyErr_NewException("usb.USBError", PyExc_IOError, NULL);
	if (!PyExc_USBError) return;
	PyModule_AddObject(module, "USBError", PyExc_USBError);
	Py_INCREF(PyExc_USBError);

	if (PyType_Ready(&Py_usb_Endpoint_Type) < 0) return;
	Py_INCREF(&Py_usb_Endpoint_Type);
	PyModule_AddObject(module, "Endpoint", (PyObject *) &Py_usb_Endpoint_Type);

	if (PyType_Ready(&Py_usb_Interface_Type) < 0) return;
	Py_INCREF(&Py_usb_Interface_Type);
	PyModule_AddObject(module, "Interface", (PyObject *) &Py_usb_Interface_Type);
	
	if (PyType_Ready(&Py_usb_Configuration_Type) < 0) return;
	Py_INCREF(&Py_usb_Configuration_Type);
	PyModule_AddObject(module, "Configuration", (PyObject *) &Py_usb_Configuration_Type);

	if (PyType_Ready(&Py_usb_Device_Type) < 0) return;
	Py_INCREF(&Py_usb_Device_Type);
	PyModule_AddObject(module, "Device", (PyObject *) &Py_usb_Device_Type);

	if (PyType_Ready(&Py_usb_Bus_Type) < 0) return;
	Py_INCREF(&Py_usb_Bus_Type);
	PyModule_AddObject(module, "Bus", (PyObject *) &Py_usb_Bus_Type);

	if (PyType_Ready(&Py_usb_DeviceHandle_Type) < 0) return;
	Py_INCREF(&Py_usb_DeviceHandle_Type);
	PyModule_AddObject(module, "DeviceHandle", (PyObject *) &Py_usb_DeviceHandle_Type);

	installModuleConstants(module);

	usb_init();
}

/*
 * vim:ts=4
 */
