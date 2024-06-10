#include "picture.h"
#include "ImageData.h"
#include "run_File.h"
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
// #include <string.h>

#include "f_util.h"
// const char *fileList = "fileList.txt";          // Picture names store files
// char pathName[fileLen];                         // The name of the picture to display
// int scanFileNum = 0;                            // The number of images scanned
char photoFramePath[] = "default.bmp";

typedef struct picture_s
{
    UBYTE * image;


    const char *fileList;
    const char *fileIndex;
    char pathName[100];
    int scanFileNum;
    uint32_t indexCurrent;

}picture_t;

picture_t gstPicture = {
    .image = NULL,
    .fileList = "fileList.txt",
    .fileIndex = "index.txt",
    .scanFileNum = 0,
    .indexCurrent = 1,
};


static void ls2file(const char *dir, const char *path) {
    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr; /* Return value */
    char const *p_dir;
    if (dir[0]) {
        p_dir = dir;
    } else {
        fr = f_getcwd(cwdbuf, sizeof cwdbuf);
        if (FR_OK != fr) {
            printf("f_getcwd error: %s (%d)\n", FRESULT_str(fr), fr);
            return;
        }
        p_dir = cwdbuf;
    }
    printf("Directory Listing: %s\n", p_dir);
    DIR dj;      /* Directory object */
    FILINFO fno; /* File information */
    memset(&dj, 0, sizeof dj);
    memset(&fno, 0, sizeof fno);
    fr = f_findfirst(&dj, &fno, p_dir, "*");
    if (FR_OK != fr) {
        printf("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    int filNum=0;
    FIL fil;
    fr =  f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE);
    if(FR_OK != fr && FR_EXIST != fr)
        panic("f_open(%s) error: %s (%d) \n", path, FRESULT_str(fr), fr);
    // f_printf(&fil, "{");
    while (fr == FR_OK && fno.fname[0]) { /* Repeat while an item is found */
        /* Create a string that includes the file name, the file size and the
         attributes string. */
        const char *pcWritableFile = "writable file",
                   *pcReadOnlyFile = "read only file",
                   *pcDirectory = "directory";
        const char *pcAttrib;
        /* Point pcAttrib to a string that describes the file. */
        if (fno.fattrib & AM_DIR) {
            pcAttrib = pcDirectory;
        } else if (fno.fattrib & AM_RDO) {
            pcAttrib = pcReadOnlyFile;
        } else {
            pcAttrib = pcWritableFile;
        }
        /* Create a string that includes the file name, the file size and the
         attributes string. */
        if(fno.fname) {
            // f_printf(&fil, "%d %s\r\n", filNum, fno.fname);
            f_printf(&fil, "pic/%s\r\n", fno.fname);
            filNum++;
        }
        fr = f_findnext(&dj, &fno); /* Search for next item */
    }
    // f_printf(&fil, "}");
    // printf("The number of file names written is: %d\n" ,filNum);
    // scanFileNum = filNum;
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    f_closedir(&dj);
}

static void run_cat(const char *path) {
    // char *arg1 = strtok(NULL, " ");
    if (!path) 
    {
        printf("Missing argument\n");
        return;
    }
    FIL fil;
    FRESULT fr = f_open(&fil, path, FA_READ);
    if (FR_OK != fr) 
    {
        printf("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    char buf[256];
    int i=0;
    while (f_gets(buf, sizeof buf, &fil)) 
    {
        printf("%5d,%s", ++i,buf);
    }

    printf("The number of file names read is %d",i);
    gstPicture.scanFileNum = i;

    fr = f_close(&fil);
    if (FR_OK != fr) 
        printf("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
}

static void setIndexFile(uint32_t idx)
{
    FRESULT fr; /* Return value */
    FIL fil;

    fr =  f_open(&fil, gstPicture.fileIndex, FA_OPEN_ALWAYS | FA_WRITE);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("setIndexFile open error\r\n");
        return;
    }
    f_printf(&fil, "%d\r\n", idx);
    printf("set index is %d\r\n", idx);
    f_close(&fil);
}

static void getCurrentIndex(void)
{
    FRESULT fr; /* Return value */
    FIL fil;
    char strbuf[10];
    int index = 1;

    if (FS_isFileExist(gstPicture.fileIndex) == true) {
        fr =  f_open(&fil, gstPicture.fileIndex, FA_READ);
        if(FR_OK == fr) {
            f_gets(strbuf, 10, &fil);
            sscanf(strbuf, "%d", &index);   // char to int

            if(index > gstPicture.scanFileNum) 
            {
                index = 1;
                printf("get index over scanFileNum\r\n");    
            }
            if(index < 1)
            {
                index = 1;
                printf("get index over one\r\n");  
            }
            printf("get index is %d\r\n", index);
        }
        f_close(&fil);
    }
    else {
        setIndexFile(index);
    }
    gstPicture.indexCurrent = index;

}

static void getCurrentFileName()
{
    FRESULT fr; /* Return value */
    FIL fil;

    fr =  f_open(&fil, gstPicture.fileList, FA_READ);
    if(FR_OK != fr && FR_EXIST != fr) {
        printf("fil2array open error\r\n");
        return;
    }

    // printf("ls array path\r\n");
    for(int i=0; i<gstPicture.indexCurrent; i++) {
        if(f_gets(gstPicture.pathName, 200, &fil) == NULL) {
            break;
        }
        // printf("%s", pathName[i]);
    }

    f_close(&fil);
}

static void updatePathIndexas()
{
    uint32_t index = gstPicture.indexCurrent;

    index++;
    if(index > gstPicture.scanFileNum)
        index = 1;
    setIndexFile(index);
    printf("updatePathIndex index is %d\r\n", index);
}

void PICTURE_Draw()
{
    if (FS_isSdCardMounted() == true) {
        ls2file("0:/pic", gstPicture.fileList);
        run_cat(gstPicture.fileList);
        getCurrentIndex();
        getCurrentFileName();

        GUI_ReadBmp_RGB_7Color(gstPicture.pathName, 0, 0);
        if (FS_isFileExist(photoFramePath) == true) {
            // GUI_ReadBmp_RGB_7Color_Photo_Frame(photoFramePath, 0, 0);
        }
        updatePathIndexas();
    }
}