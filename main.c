//
//  main.c
//  libusb-test
//
//  Created by fairy-slipper on 4/18/16.
//  Copyright © 2016 fairy-slipper. All rights reserved.
//


#include <libusb-1.0/libusb.h>

#include <stdio.h>
#include <sys/types.h>

static void print_devs(libusb_device **devs)
{
    libusb_device *dev;
    int i = 0;
    
    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            fprintf(stderr, "failed to get device descriptor");
            return;
        }
        
        printf("%04x:%04x (bus %d, device %d)\n",
               desc.idVendor, desc.idProduct,
               libusb_get_bus_number(dev), libusb_get_device_address(dev));
    }
}

int main(void)
{
    libusb_context *ctx = NULL;
    libusb_device **devs;
    libusb_device_handle *handle = NULL;
    int r;
    ssize_t cnt;
    
    r = libusb_init(NULL);
    if (r < 0)
        return r;
    
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
        return (int) cnt;
    
    struct libusb_device_descriptor desc;
    
    r = libusb_get_device_descriptor(devs[4], &desc);
    if (r < 0) {
        fprintf(stderr, "libusb_get_device_descriptor error %d\n", r);
    } else {
        printf("got device descriptor %x %x\n", desc.idVendor, desc.idProduct);
    }
    
    handle = libusb_open_device_with_vid_pid(NULL, desc.idVendor, desc.idProduct);
    if (handle == NULL) {
        fprintf(stderr, "failed to open device\n");
    } else {
        printf("opened device\n");
    }
    
    print_devs(devs);
    libusb_free_device_list(devs, 1);

    libusb_set_auto_detach_kernel_driver(handle, 0);    
    libusb_release_interface(handle, 0);

    if(libusb_kernel_driver_active(handle, 0) == 1) {
        printf("driver in use\n");
        if(libusb_detach_kernel_driver(handle, 0) == 0) {
            printf("driver detached\n");
        }
    }

    if((r = libusb_set_configuration(handle, 0))<0) {
	printf("Couldn't configure %i\n", r);
    } else {
	printf("device configured\n");
    }

    r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        fprintf(stderr, "usb_claim_interface error %d %d\n", r, LIBUSB_ERROR_TIMEOUT);
    } else {
        printf("claimed interface\n");
    }

    int size;
    unsigned char datain[8]="\0";
    for(int i=0;i<30;i++)
    {
        int rr = libusb_interrupt_transfer(handle,
                                      0x81,
                                      datain,
                                      sizeof(datain),
                                      &size,
                                      1000);
        printf("libusb_interrupt_transfer %i %i %i %i %i\n", rr, LIBUSB_ERROR_TIMEOUT, LIBUSB_ERROR_BUSY, LIBUSB_ERROR_NOT_FOUND, LIBUSB_ERROR_NO_DEVICE);
        printf("size %i\n", size);
        printf("data: ");
        for(int j=0; j<size; j++)
            printf("%02x ", (unsigned char)(datain[j]));
        printf("\n");
    }
    
    libusb_release_interface(handle, 0);
    
    libusb_close(handle);
    
    libusb_exit(NULL);
    return 0;
}
