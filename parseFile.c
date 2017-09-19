#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include "common.h"

static void trimStr(char **s)
{
    int i;
    int len = strlen(*s);

    for (i = len - 1; i >= 0; i--)
	{
        if ((*s)[i] <= ' ')
            (*s)[i] = 0;
        else
            break;
    }

    while (**s)
	{
        if (**s <= ' ')
            (*s)++;
		else
            return;
    }
}

static void trimLineData(char *line_data_buf)
{
    char *tmp_ptr = NULL;

    if (line_data_buf == NULL)
        return;

    /*trim comment string*/
    tmp_ptr = line_data_buf;
    while (*tmp_ptr && ((tmp_ptr - line_data_buf) <= MAX_LINE_LEN))
	{
        if (*tmp_ptr == '#' || *tmp_ptr == ';')
		{
            *tmp_ptr = '\0';
            break;
        }
        tmp_ptr++;
    }

    /*trim other character*/
    tmp_ptr = line_data_buf;
    trimStr(&tmp_ptr);
    strncpy(line_data_buf, tmp_ptr, MAX_LINE_LEN);
}

int readFile(FILE *fp, pfileHandle handler, void *data)
{
    char lineBuf[MAX_LINE_LEN];
    char *name = NULL;
    char *value;
	char intStr[128];
	unsigned int  mapcode;
    unsigned short ircode = 0;
    unsigned char parse_flag = CONFIG_LEVEL;
    unsigned char last_flag = CONFIG_LEVEL;

    while (fgets(lineBuf, MAX_LINE_LEN, fp))
	{
        trimLineData(lineBuf);
		 if (IS_NULL(lineBuf))
			continue;
        name = lineBuf;
        switch (parse_flag)
		{
        case CONFIG_LEVEL:
            last_flag = CONFIG_LEVEL;
            if (strcasecmp(name, "key_begin") == 0)
			{
                parse_flag = KEYMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "mouse_begin") == 0)
			{
                parse_flag = MOUSEMAP_LEVEL;
                continue;
            }

            if (strcasecmp(name, "keyadc_begin") == 0)
			{
                parse_flag = ADCMAP_LEVEL;
                continue;
            }

            value = strchr(lineBuf, '=');
            if (value)
			{
                *value++ = 0;
                trimStr(&value);
            }

            trimStr(&name);
            if (!*name)
			{
                continue;
            }
			if ((*handler)(name, value, data) < 0)
			{
				fprintf(stderr, "invalid parameter:%s=%s\n", name, value);
				continue;
			}
			break;

        case KEYMAP_LEVEL:
            if (strcasecmp(name, "key_end") == 0)
			{
                parse_flag = last_flag;
                continue;
            }

            value = strchr(lineBuf, ' ');
            if (value)
			{
                *value++ = 0;
                trimStr(&value);
            }

            trimStr(&name);
            if (!*name)
			{
                continue;
            }

			ircode = strtoul(name, NULL, 0);

			if (ircode > 0xff)
			{
				fprintf(stderr, "invalid ircode: 0x%x\n", ircode);
				continue;
			}
			mapcode = MAPCODE(ircode, strtoul(value, NULL, 0));

			snprintf(intStr, 128, "%u", mapcode);

			if ((*handler)("mapcode", intStr, data) < 0)
			{
				fprintf(stderr, "invalid parameter:%s=%s\n", "mapcode", intStr);
				continue;
			}
			break;
        }

    }

    return 0;
}


int parseFile(const char *file, pfileHandle handler, void *data)
{
	FILE *cfgFp;
	if (NULL == (cfgFp = fopen(file, "r")))
	{
		fprintf(stderr, "fopen %s: %s\n", file, strerror(errno));
		return FAIL;
	}

	readFile(cfgFp, handler, data);

	fclose(cfgFp);

	return SUCC;
}
