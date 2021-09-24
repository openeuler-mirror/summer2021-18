#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.c"
#include <ctype.h>

map_int_t m; //define the map as global
char pidbuffer[40];

int pid_sort(char *s, int linenum, FILE *fout) //matching the pid number, do the counting
{

    char *pt;
    char pidnumber[5];
    char pid[10] = "pid ";
    char pid1[5] = "pid 1";
    int number;

    if (pt = strstr(s, pid))
    {
        sscanf(pt, "%*[^0-9]%d", &number);
        sprintf(pidnumber, "%d", number);
        strcat(pid, pidnumber);
        memcpy(pidbuffer, pid, sizeof(pid));
        return 1;
    }
    return 0;
}

int zone_sort(char *s, int linenum, FILE *fout) //matching the pid number, do the counting
{

    char *pt1;
    char *pt2;
    char *pt;
    char zonenumber[12];
    char nodenumber[12];
    char node[7] = "node=";
    char zone[7] = "zone=";
    char Movable[8] = "Movable";
    char Unmovable[10] = "Unmovable";
    char HighAtomic[11] = "HighAtomic";
    char Reclaimable[12] = "Reclaimable";
    char Isolate[8] = "Isolate";
    char CMA[4] = "CMA";
    int type = 0;
    char output[80];
    char pidnumber[5];
    int number1;
    int number2;
    char zonetype[1035];

    if (pt = strstr(s, Movable))
    {
        type = 1;
    }

    if (pt = strstr(s, Unmovable))
    {
        type = 2;
    }

    if (pt = strstr(s, HighAtomic))
    {
        type = 3;
    }

    if (pt = strstr(s, Reclaimable))
    {
        type = 4;
    }

    if (pt = strstr(s, Isolate))
    {
        type = 5;
    }

    if (pt = strstr(s, CMA))
    {
        type = 6;
    }

    if (pt1 = strstr(s, node))
    {
        memcpy(output, pidbuffer, sizeof(pidbuffer));
        sscanf(pt1, "%*[^0-9]%d", &number1);
        sprintf(nodenumber, "%d", number1);
        strcat(node, nodenumber);
        strcat(output, ", ");
        strcat(output, node);

        pt2 = strstr(s, zone);
        sscanf(pt2, "%*[^0-9]%d", &number2);
        sprintf(zonenumber, "%d", number2);

        switch (number2)
        {
        case 0:
            memcpy(zonetype, "DMA   ", 7);
            break;

        case 1:
            memcpy(zonetype, "DMA32 ", 7);
            break;

        default:
            memcpy(zonetype, "Others", 7);
            break;
        }

        //fprintf(fout, "%s, type%s in line %d \n", output, zonetype, linenum);

        strcat(zone, zonenumber);
        strcat(output, ", ");
        strcat(output, zone);
        strcat(output, ", type = ");
        strcat(output, zonetype);

        switch (type)
        {
        case 0:
            break;
        case 1:
            strcat(output, ", ");
            strcat(output, Movable);
            break;
        case 2:
            strcat(output, ", ");
            strcat(output, Unmovable);
            break;
        case 3:
            strcat(output, ", ");
            strcat(output, HighAtomic);
            break;
        case 4:
            strcat(output, ", ");
            strcat(output, Reclaimable);
            break;
        case 5:
            strcat(output, ", ");
            strcat(output, Isolate);
            break;
        case 6:
            strcat(output, ", ");
            strcat(output, CMA);
            break;

        default:
            break;
        }

        int *val = map_get(&m, output);
        if (val)
        {
            *val += 1;
            map_set(&m, output, *val);
        }
        else
        {
            map_set(&m, output, 1);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

int handleline(int linenum, char *text, FILE *fout) //handling each line grab from the file
{
    char buf[1024];
    memcpy(buf, text, 500);

    pid_sort(buf, linenum, fout);
    zone_sort(buf, linenum, fout);

    return 0;
}

int readtext(const char *filename, FILE *fout) //read the file line by line
{
    FILE *fp;

    int linenum;

    char *p, buf[1024];

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        return -1;
    }

    for (linenum = 1; fgets(buf, sizeof(buf), fp) != NULL; ++linenum)
    {
        if ((p = strchr(buf, '\n')) == NULL)
        {
            p = buf + strlen(buf);
        }

        if (p > buf && p[-1] == '\r')
        {
            --p;
        }

        *p = '\0';

        for (p = buf; *p != '\0' && isspace((int)*p); ++p)
        {
            ;
        }

        if (*p == '\0' || *p == '#')
        {
            continue;
        }

        if (handleline(linenum, p, fout) != 0)
        {
            printf("WARNING: cannot parse line[%d]=[%s], skipped\n", linenum, buf);
            continue;
        }
    }

    fclose(fp);

    return 0;
}

void assign_comm(FILE *fout)
{
    char path[1024];
    char copy[1024];
    FILE *fp;
    int checker, number;

    fp = popen("/bin/ps -o pid,comm", "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        exit(1);
    }
    while (fgets(path, sizeof(path), fp) != NULL) //comparing the pid number in both pid and map key
    {
        sscanf(path, "%*[^0-9]%d", &number);

        map_iter_t iter = map_iter(&m);
        const char *key;
        while ((key = map_next(&m, &iter)))
        {

            sscanf(key, "%*[^0-9]%d", &checker);

            if (number == checker)
            {

                strcpy(copy, path + 6);
                fprintf(fout, "%s -> %d ,      %s", key, *map_get(&m, key), copy);
            }
        }
    }
}

void out_put(FILE *fout)
{
    map_iter_t iter = map_iter(&m);
    const char *key;
    int number;
    char zonetype[1035];

    while ((key = map_next(&m, &iter)))
    {
        printf("%s\n", key);

        fprintf(fout, "%s -> %d\n", key, *map_get(&m, key));
    }
}

int main(int argc, char **argv)
{

    FILE *fout;
    map_init(&m);

    if (argc < 3)
    {
        printf("Usage: ./program <input>(sorted_page_owner.txt)<output>(result.txt)\n");
        perror("open: ");
        exit(1);
    }
    fout = fopen(argv[2], "w");
    readtext(argv[1], fout);

    fprintf(fout, "===========================================================\n");
    fprintf(fout, "   pid    node    zone      zonetype       type      number       process\n");

    assign_comm(fout);

    //out_put(fout);

    map_deinit(&m);

    return 0;
}
