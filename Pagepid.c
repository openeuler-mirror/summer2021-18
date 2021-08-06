#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.c"
#include <ctype.h>

#define ENABLE_LOCATION 0 //enable to show the pid in which line
#define HIDE_PID1 1       //enable to show information about pid 1

map_int_t m; //define the map as global

void pid_sort(char *s, int linenum, FILE *fout) //matching the pid number, do the counting
{

    char *pt;
    char pidnumber[5];
    char pid[5] = "pid ";
    char pid1[5] = "pid 1";
    int number;

    pt = strstr(s, pid);
    sscanf(pt, "%*[^0-9]%d", &number);
    sprintf(pidnumber, "%d", number);
    strcat(pid, pidnumber);

    int *val = map_get(&m, pid);
    if (val)
    {
        *val += 1;
        map_set(&m, pid, *val);
    }
    else
    {
        map_set(&m, pid, 1);
    }

    if (ENABLE_LOCATION) //using the flag for control the output for line
    {
        if (strncmp(pid, pid1, sizeof(pid)) != 0 && HIDE_PID1)//addition flag to hide the pid 1 init process
        {
            fprintf(fout, "%s in line %d \n", pid, linenum);
        }
        else
        {
            fprintf(fout, "%s in line %d \n", pid, linenum);
        }
    }
}

int handleline(int linenum, char *text, FILE *fout) //handling each line grab from the file
{
    char buf[1024];
    memcpy(buf, text, strlen(text));
    if (strlen(text) > 45) //A naive way to avoid multiline output
    {
        pid_sort(buf, linenum, fout);
    }

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
    char path[1035];
    char copy[1035];
    FILE *fp;
    int checker, number;

    fp = popen("/bin/ps -o pid,comm", "r");
    //fp = fopen("pid.txt", "r");
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

                memcpy(copy, path, strlen(path) - 1);
                fprintf(fout, "%s -> %d\n", copy, *map_get(&m, key));
            }
        }
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

    fprintf(fout, "=========================\n");
    fprintf(fout, "   pid  comm       number\n");

    assign_comm(fout);

    map_deinit(&m);

    return 0;
}
