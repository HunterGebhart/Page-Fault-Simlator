#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct PTE
{
    int valid;
    int frameNum;
}PTE;

int findFreeFrame();
int findVictimFrame();
int powerOfTwo(int bytes);
int twoToThePowerOf(int exp);
long int getBitMask(int d);

int* freeFrames;
PTE* PT;
int clock = 0;
int* LRUcount;
int* revMap;

int main(int argc, char* argv[])
{
    FILE* inFile, *outFile;
    int i, full = 0, j = 0, k = 0, pageFaults = 0, p, d, f, bytes, pageCount, frameCount;
    unsigned int pnum, dnum, fnum;
    unsigned long LA, PA;
    long int bitMask;

    if(argc < 6 || argc > 6)
    {
        printf("ERROR incorrect number of arguments (expected 5)\n");
        return 1;
    }

    sscanf(argv[1], "%d", &bytes);
    d = powerOfTwo(bytes);
    sscanf(argv[2], "%d", &bytes);
    p = powerOfTwo(bytes) - d;
    sscanf(argv[3], "%d", &bytes);
    f = powerOfTwo(bytes) - d;

    pageCount = twoToThePowerOf(p);
    frameCount = twoToThePowerOf(f);

    PT = (PTE*)malloc(sizeof(PTE) * pageCount);
    freeFrames = (int*)malloc(sizeof(int) * frameCount);
    LRUcount = (int*)malloc(sizeof(int) * frameCount);
    revMap = (int*)malloc(sizeof(int) * frameCount);

    bitMask = getBitMask(d);

    if(PT == NULL || freeFrames == NULL || LRUcount == NULL || revMap == NULL)
    {
        printf("ERROR with malloc\n");
        return 1;
    }

    for(i = 0; i < pageCount; i++)
    {
        PT[i].valid = 0;
    }

    freeFrames[0] = 0;

    for(i = 1; i < frameCount; i++)
    {
        freeFrames[i] = 1;
    }

    inFile = fopen(argv[4], "rb");

    if(inFile == NULL)
    {
        printf("ERROR could not open input file\n");
    }
    outFile = fopen(argv[5], "wb+");

    while(fread(&LA, sizeof(unsigned long), 1, inFile) == 1)
    {
        clock++;
        pnum = LA >> d;
        dnum = LA & bitMask;

        if(PT[pnum].valid == 1)
        {
            fnum = PT[pnum].frameNum;
            PA = (fnum << d) + dnum;
            fwrite(&PA, sizeof(PA), 1, outFile);
            LRUcount[fnum] = clock;
        }
        else
        {
            int frame = findFreeFrame();

            if(frame > 0)
            {
                PT[pnum].frameNum = frame;
                PT[pnum].valid = 1; 
                fnum = PT[pnum].frameNum;
                PA = (fnum << d) + dnum;
                fwrite(&PA, sizeof(PA), 1, outFile);
                revMap[frame] = pnum;
                LRUcount[fnum] = clock;
                pageFaults++;
            }
            else /*perform LRU policy*/
            {
                pageFaults++;
                int victim = findVictimFrame();
                PT[revMap[victim]].valid = 0;
                PT[pnum].frameNum = victim;
                PT[pnum].valid = 1;
                fnum = PT[pnum].frameNum;
                PA = (fnum << d) + dnum;
                fwrite(&PA, sizeof(PA), 1, outFile);
                LRUcount[fnum] = clock;
                revMap[fnum] = pnum;
            }
        }
    }
    printf("pageFaults: %d\n", pageFaults);

    free(PT);
    free(freeFrames);
    free(LRUcount);

    fclose(inFile);
    fclose(outFile);
}

int findFreeFrame()
{
    int i;
    for (i = 1; i < 8; i++)
    {
        if(freeFrames[i] == 1)
        {
            freeFrames[i] = 0;
            return i;
        }
    }
    return -1;
}

int findVictimFrame()
{
    int i, min = INT_MAX, frame;
    for (i = 1; i < 8; i++)
    {
        if(LRUcount[i] < min)
        {
            min = LRUcount[i];
            frame = i;
        }
    }
    return frame;
}

int powerOfTwo(int bytes)
{
    int i, count = 0;
    for(; bytes != 0; bytes = bytes/2 )
    {
        count++;
    }
    return count - 1;
}

int twoToThePowerOf(int exp)
{
    int i, val = 2;
    for(i = 1; i < exp; i++)
    {
        val = val * 2;
    }
    return val;
}

long int getBitMask(int d) 
{
    char* maskStr = (char*)malloc(sizeof(char) * d);
    int i, remainder;
    //unsigned int bitMask;
    long int hex, bitMask;
    for(i = 0; i < d; i++)
    {
        maskStr[i] = '1';
    }
    sscanf(maskStr, "%ld", &bitMask);
    free(maskStr);
    printf("%ld\n", bitMask);

    i = 1;
    while (bitMask != 0){
        remainder = bitMask % 10;
        hex = hex + remainder * i;
        i = i * 2;
        bitMask = bitMask / 10;
    }
    printf("Equivalent hexadecimal value: %lX\n", hex);

    return hex;
}