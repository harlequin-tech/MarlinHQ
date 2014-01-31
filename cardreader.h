#ifndef CARDREADER_H
#define CARDREADER_H

#ifdef SDSUPPORT

#include <SD.h>
#define MAX_VFAT_ENTRIES (2)
#define LONG_FILENAME_LENGTH (13*MAX_VFAT_ENTRIES+1)

class CardReader
{
public:
    CardReader();

    bool initsd();
    void write_command(char *buf);
    //files auto[0-9].g on the sd card are performed in a row
    //this is to delay autostart and hence the initialisaiton of the sd card to some seconds after the normal init, so the device is available quick after a reset

    void checkautostart(bool x); 
    void openFile(char* name,bool read);
    void removeFile(char* name);
    void closefile();
    void release();
    void startFileprint();
    void pauseSDPrint();
    void getStatus();
    void printingHasFinished();

    char *getfilename(const uint16_t index);
    char *getfilename(SdFile &dir, const uint16_t index);
    uint16_t getnrfilenames(void);

    void ls();
    void setroot();
    void pushDir(const char *subdirname, uint16_t findex);
    bool popDir(void);
    bool chdir(uint16_t *path, uint8_t depth);
    bool recover(void);
    uint32_t timeLeft(void);

    FORCE_INLINE bool eof() 
	{
	    return sdpos>=filesize;
	};
    FORCE_INLINE int16_t get() 
	{
	    sdpos = file.curPosition();
	    return (int16_t)file.read();
	};
    FORCE_INLINE void setIndex(long index)
	{
	    sdpos = index;file.seekSet(index);
	};
    FORCE_INLINE uint8_t percentDone()
        {
	    if(!sdprinting) return 0;
	    if(filesize) return
		sdpos*100/filesize;
	    else
		return 0;
	};
    FORCE_INLINE char* getWorkDirName()
	{
	    return workDirName;
	};

    bool saving;
    bool sdprinting ;  
    bool cardOK ;
    char filename[13];
    char longFilename[LONG_FILENAME_LENGTH];
    bool filenameIsDir;
    int lastnr; //last number of the autostart;
    SdFile file;

private:
    uint16_t pathStack[32];	// Directory path list
    uint8_t pathDepth;		// current depth in path list
    char workDirName[LONG_FILENAME_LENGTH];
    SdFile root,*curDir,workDir,workDirParent,workDirParentParent;
    Sd2Card card;
    SdVolume volume;
    char curFilename[13];
    uint16_t filePathStack[32];	// Directory path list for current file
    uint8_t filePathDepth;	// Directory path depth for current file
    uint32_t filesize;
    unsigned long autostart_atmillis;
    uint32_t sdpos ;
    uint32_t startTime;

    bool autostart_stilltocheck; //the sd start is delayed, because otherwise the serial cannot answer fast enought to make contact with the hostsoftware.

    int16_t nrFiles; //counter for the files in the current directory
    char* diveDirName;
    void lsDive(char *path, uint16_t limit, SdFile &parent);
    char *getFilenameFromIndex(SdFile &parent, uint16_t index);
    uint16_t countFiles(SdFile &parent);
};

#define IS_SD_PRINTING (card.sdprinting)

#else

#define IS_SD_PRINTING (false)

#endif //SDSUPPORT
#endif
