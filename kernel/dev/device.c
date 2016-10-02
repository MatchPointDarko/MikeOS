/*
 * MikeOS: Device manager.
 */

#include <common.h>
#include <dev/device.h>

static block_device_t* devices_ptrs[MAX_DEVICES] = {NULL};
static unsigned long registered_devices = 0;

bool_t register_device(block_device_t* dev)
{
    if(dev == NULL || registered_devices == MAX_DEVICES)
    {
        return false;
    }

    //Find an empty place.
    for(int i = 0; i < MAX_DEVICES; i++)
    {
        if(devices_ptrs[i] == NULL)
        {
            devices_ptrs[i] = dev;
        }
    }

    registered_devices++;

    return false;
}

bool_t unregister_device(block_device_t* dev)
{
    if(dev == NULL)
    {
        return false;
    }

    //Look for the pointer.
    for(int i = 0; i < MAX_DEVICES; i++)
    {
        if(devices_ptrs[i] == dev)
        {
            devices_ptrs[i] = NULL;
            registered_devices--;
            return true;
        }
    }

    //No such device.
    return false;
}
