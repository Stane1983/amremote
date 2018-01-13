#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_config.h"

#define LOG_TAG "remotecfg"
#define LOG_NDEBUG 0

#define CC_MAX_LINE_LEN     (400)

enum {
    CONFIG_LEVEL,
    CUSTOM_LEVEL,
    KEYMAP_LEVEL,
    REPEATKEYMAP_LEVEL,
    MOUSEMAP_LEVEL,
    ADCMAP_LEVEL,
    FACTORYCUSTMAP_LEVEL
};

extern unsigned short adc_map[2];
extern unsigned int adc_move_enable;

static void str_trim(char **s) {
    int i;
    int len = strlen(*s);

    for (i = len - 1; i >= 0; i--) {
        if ((*s)[i] <= ' ') {
            (*s)[i] = 0;
        } else {
            break;
        }
    }

    while (**s) {
        if (**s <= ' ') {
            (*s)++;
        } else {
            return;
        }
    }
}

static void trim_line_data(char *line_data_buf) {
    char *tmp_ptr = NULL;

    if (line_data_buf == NULL) {
        return;
    }

    //trim comment string
    tmp_ptr = line_data_buf;
    while (*tmp_ptr && ((tmp_ptr - line_data_buf) <= CC_MAX_LINE_LEN)) {
        if (*tmp_ptr == '#' || *tmp_ptr == ';') {
            *tmp_ptr = '\0';
            break;
        }

        tmp_ptr++;
    }

    //trim other character
    tmp_ptr = line_data_buf;
    str_trim(&tmp_ptr);
    memmove(line_data_buf, tmp_ptr, strlen(tmp_ptr) + 1);
}

static int remote_config_set(char *name, char *value, remote_config_t *config) {
    unsigned int i;
    unsigned int *config_para = (unsigned int*) &config->factory_infcode;

    for (i = 0; i < ARRAY_SIZE(config_item); i++) {
        if (strcmp(config_item[i], name) == 0) {
            config_para[i] = strtoul(value, NULL, 0);
            printf("curpara:%s  %08x\n", name, config_para[i]);
            return 0;
        }
    }

    return -1;
}

int parse_and_set_config_from_file(FILE *fp, remote_config_t *remote) {
    char line_data_buf[CC_MAX_LINE_LEN];
    char *name = NULL;
    char *value;
    unsigned short ircode = 0, keycode = 0, adccode, index, custcode;
    unsigned char parse_flag = CONFIG_LEVEL;
    unsigned char last_flag = CONFIG_LEVEL;
    unsigned char has_custom_config = 0;

    while (fgets(line_data_buf, CC_MAX_LINE_LEN, fp)) {
        trim_line_data(line_data_buf);

        name = line_data_buf;

        switch (parse_flag) {
        case CONFIG_LEVEL:
            last_flag = CONFIG_LEVEL;
            if (strcasecmp(name, "custom_begin") == 0) {
                parse_flag = CUSTOM_LEVEL;
                continue;
            }

            if (strcasecmp(name, "key_begin") == 0) {
                parse_flag = KEYMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "repeat_key_begin") == 0) {
                parse_flag = REPEATKEYMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "mouse_begin") == 0) {
                parse_flag = MOUSEMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "keyadc_begin") == 0) {
                parse_flag = ADCMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "factorycust_begin") == 0) {
                parse_flag = FACTORYCUSTMAP_LEVEL;
                continue;
            }

            value = strchr(line_data_buf, '=');
            if (value) {
                *value++ = 0;
                str_trim(&value);
            }

            str_trim(&name);
            if (!*name) {
                continue;
            }

            if (remote_config_set(name, value, remote)) {
                printf("config file has not supported parameter:%s=%s\r\n", name, value);
            }
            continue;
        case CUSTOM_LEVEL:
            last_flag = CUSTOM_LEVEL;
            if (strcasecmp(name, "custom_end") == 0) {
                parse_flag = CONFIG_LEVEL;
                has_custom_config = 1;
                set_config(remote);
                continue;
            }

            if (strcasecmp(name, "key_begin") == 0) {
                parse_flag = KEYMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "repeat_key_begin") == 0) {
                parse_flag = REPEATKEYMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "mouse_begin") == 0) {
                parse_flag = MOUSEMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "keyadc_begin") == 0) {
                parse_flag = ADCMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "factorycust_begin") == 0) {
                parse_flag = FACTORYCUSTMAP_LEVEL;
                continue;
            }

            value = strchr(line_data_buf, '=');
            if (value) {
                *value++ = 0;
                str_trim(&value);
            }

            str_trim(&name);
            if (!*name) {
                continue;
            }

            if (remote_config_set(name, value, remote)) {
                printf("config file has not supported parameter:%s=%s\r\n", name, value);
            }
            continue;
        case KEYMAP_LEVEL:
            if (strcasecmp(name, "key_end") == 0) {
                parse_flag = last_flag;
                continue;
            }

            value = strchr(line_data_buf, ' ');
            if (value) {
                *value++ = 0;
                str_trim(&value);
            }

            str_trim(&name);
            if (!*name) {
                continue;
            }
            ircode = strtoul(name, NULL, 0);
            if (ircode > 0xff) {
                continue;
            }

            keycode = strtoul(value, NULL, 0) & 0xffff;
            if (keycode) {
                remote->key_map[ircode] = keycode;
                printf("KEYMAP_LEVEL: ircode = 0x%x, keycode = %d\n", ircode, keycode);
            }
            continue;
        case REPEATKEYMAP_LEVEL:
            if (strcasecmp(name, "repeat_key_end") == 0) {
                parse_flag = last_flag;
                continue;
            }

            value = strchr(line_data_buf, ' ');
            if (value) {
                *value++ = 0;
                str_trim(&value);
            }

            str_trim(&name);
            if (!*name) {
                continue;
            }
            ircode = strtoul(name, NULL, 0);
            if (ircode > 0xff) {
                continue;
            }

            keycode = strtoul(value, NULL, 0) & 0xffff;
            if (keycode) {
                remote->repeat_key_map[ircode] = keycode;
                printf("REPEATKEYMAP_LEVEL: ircode = 0x%x, keycode = %d\n", ircode, keycode);
            }
            continue;
        case MOUSEMAP_LEVEL:
            if (strcasecmp(name, "mouse_end") == 0) {
                parse_flag = last_flag;
                continue;
            }

            value = strchr(line_data_buf, ' ');
            if (value) {
                *value++ = 0;
                str_trim(&value);
            }

            str_trim(&name);
            if (!*name) {
                continue;
            }
            ircode = strtoul(name, NULL, 0);
            if (ircode > 3) {
                continue;
            }

            keycode = strtoul(value, NULL, 0) & 0xff;
            remote->mouse_map[ircode] = keycode;
            printf("MOUSEMAP_LEVEL: ircode = 0x%x, keycode = %d\n", ircode, keycode);
            continue;
        case ADCMAP_LEVEL:
            if (strcasecmp(name, "keyadc_end") == 0) {
                parse_flag = last_flag;
                continue;
            }

            value = strchr(line_data_buf, ' ');
            if (value) {
                *value++ = 0;
                str_trim(&value);
            }

            str_trim(&name);
            if (!*name) {
                continue;
            }

            adccode = strtoul(name, NULL, 0);
            if (adccode > 3) {
                continue;
            }
            if (adc_move_enable == 0) {
                adc_move_enable = 1;
            }

            keycode = strtoul(value, NULL, 0) & 0xff;
            adc_map[adccode] = keycode;
            continue;
		case FACTORYCUSTMAP_LEVEL:
            if (strcasecmp(name, "factorycust_end") == 0) {
                parse_flag = last_flag;
                continue;
            }

            value = strchr(line_data_buf, ' ');
            if (value) {
                *value++ = 0;
                str_trim(&value);
            }

            str_trim(&name);
            if (!*name) {
                continue;
            }
            index = strtoul(name, NULL, 0);
            if (ircode > 0xff) {
                continue;
            }

            custcode = strtoul(value, NULL, 0) & 0xffff;
            if (keycode) {
                remote->factory_customercode_map[index] = custcode;
                printf("FACTORYCUSTMAP_LEVEL: index = 0x%x, custcode = 0x%x\n", index, custcode);
            }
            continue;
        }
    }
    if (has_custom_config == 0) {
        set_config(remote);
    }
    return 0;
}
