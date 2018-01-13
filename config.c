#define LOG_TAG "remotecfg"
#define LOG_NDEBUG 0

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "remote_config.h"
#include "keydefine.h"

#define DEVICE_NAME             "/dev/amremote"
#define PRINT_CONFIG

int set_config(remote_config_t *remote)
{
    unsigned int i, val;
    unsigned int *para = (unsigned int*)&remote->factory_infcode;
    int device_fd = open(DEVICE_NAME, O_RDWR);
    if (device_fd <= 0) {
        printf("Can't open %s .\n", DEVICE_NAME);
        return -1;
    }
    remote->factory_code >>= 16;

    for (i = 0; i < ARRAY_SIZE(config_item); i++) {
        if (para[i] != 0xffffffff) {
            switch (i) {
                case 4:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                    printf("%20s = 0x%x\n", config_item[i], para[i]);
                    break;
                default:
                    printf("%20s = %d\n", config_item[i], para[i]);
                    break;
            }

            ioctl(device_fd, remote_ioc_table[i], &para[i]);
        }
    }
    ioctl(device_fd, REMOTE_IOC_RESET_KEY_MAPPING, NULL);

    for (i = 0; i < 256; i++)
        if (remote->key_map[i] != KEY_RESERVED) {
            val = (i<<16) | remote->key_map[i];
            ioctl(device_fd, REMOTE_IOC_SET_KEY_MAPPING, &val);
        }

    for (i = 0; i < 256; i++)
        if (remote->repeat_key_map[i] != KEY_RESERVED ) {
            val = (i<<16) | remote->repeat_key_map[i];
            ioctl(device_fd, REMOTE_IOC_SET_REPEAT_KEY_MAPPING, &val);
        }

    for (i = 0; i < 4; i++)
        if (remote->mouse_map[i] != 0xffff) {
            val = (i<<16) | remote->mouse_map[i];
            ioctl(device_fd, REMOTE_IOC_SET_MOUSE_MAPPING, &val);
        }

    close(device_fd);

    return 0;
}
