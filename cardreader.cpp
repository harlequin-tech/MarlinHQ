#include "Marlin.h"
#include "cardreader.h"
#include "ultralcd.h"
#include "stepper.h"
#include "temperature.h"
#include "language.h"

#define FILE_EXT	'G'		// .gcode

/* Work around a bug with PROGMEM and PSTR where the compiler always
 * generates warnings.
 */

#ifdef SDSUPPORT
#include <SD.h>
#include <SPI.h>

#undef DEBUG

CardReader::CardReader()
{
   filesize = 0;
   sdpos = 0;
   sdprinting = false;
   cardOK = false;
   saving = false;
   autostart_atmillis=0;
   startTime = 0;

   workDirName[0] = '/';
   workDirName[1] = 0;
   pathDepth = 0;	// root
   pathStack[0] = 0;

   autostart_stilltocheck=true; //the sd start is delayed, because otherwise the serial cannot answer fast enought to make contact with the hostsoftware.
   lastnr=0;
  //power to SD reader
  #if SDPOWER > -1
    SET_OUTPUT(SDPOWER); 
    WRITE(SDPOWER,HIGH);
  #endif //SDPOWER
  
  autostart_atmillis=millis()+5000;
}

char *createFilename(char *buffer,const dir_t &dir) //buffer>12characters
{
    for (uint8_t ind = 0; ind < 11; ind++) {
	if (dir.name[ind] == ' ') continue;
	if (ind == 8) {
	    *buffer++ = '.';
	}
	*buffer++ = dir.name[ind];
    }
    *buffer = 0;
    return buffer;
}

uint16_t CardReader::countFiles(SdFile &parent)
{
    dir_t dir;
    uint16_t count = 0;

    parent.rewind();
    while (parent.readDir(dir, longFilename) > 0) {
	if (dir.name[0] == DIR_NAME_FREE) break;
	if (dir.name[0] == DIR_NAME_DELETED || dir.name[0] == '.'|| dir.name[0] == '_') continue;
	if (dir.name[0] == '.') {
	    if (dir.name[1] != '.')
		continue;
	}

	if (!DIR_IS_FILE_OR_SUBDIR(&dir)) continue;

	if (!DIR_IS_SUBDIR(&dir)) {
	    if(dir.name[8]!=FILE_EXT) continue;
	    if(dir.name[9]=='~') continue;
	}

	count++;
    }

    return count;
}


char *CardReader::getFilenameFromIndex(SdFile &parent, uint16_t index)
{
    dir_t dir;
    uint16_t count = 0;

    parent.rewind();

    while (parent.readDir(dir, longFilename) > 0) {

	if (dir.name[0] == DIR_NAME_FREE) break;
	if (dir.name[0] == DIR_NAME_DELETED || dir.name[0] == '.'|| dir.name[0] == '_') continue;
	if (dir.name[0] == '.') {
	    if (dir.name[1] != '.')
		continue;
	}

	if (!DIR_IS_FILE_OR_SUBDIR(&dir)) continue;

	filenameIsDir = DIR_IS_SUBDIR(&dir);

	if (!filenameIsDir) {
	    if (dir.name[8]!=FILE_EXT) continue;
	    if (dir.name[9]=='~') continue;
	}

	if (count == index) {
	    createFilename(filename,dir);
	    break;
	}
	count++;
    }

    return longFilename;
}

/* Recursively list all files in directory file */
void CardReader::lsDive(char *path, uint16_t limit, SdFile &parent)
{
    dir_t dir;
    char lfilename[13];

    parent.rewind();

    while (parent.readDir(dir, longFilename) > 0) {
	if (DIR_IS_SUBDIR(&dir)) {
	    createFilename(lfilename, dir);

	    if (*path == 0) {
		strcat(path,"/");
	    }

	    strncat(path, path, limit - strlen(path) - 1);
	    strncat(path, lfilename, limit - strlen(path) - 1);
	    strncat(path, "/", limit - strlen(path) - 1);

	    //Serial.print(path);

	    SdFile dfile;
	    if (!dfile.open(parent,lfilename, O_READ)) {
		SERIAL_ECHO_START;
		SERIAL_ECHOLNPGM(MSG_SD_CANT_OPEN_SUBDIR);
		SERIAL_ECHOLN(lfilename);
	    }
	    lsDive(path, limit, dfile);
	    //close done automatically by destructor of SdFile

	} else {
	    if (dir.name[0] == DIR_NAME_FREE) break;
	    if (dir.name[0] == DIR_NAME_DELETED || dir.name[0] == '.'|| dir.name[0] == '_') continue;
	    if (dir.name[0] == '.') {
		if (dir.name[1] != '.')
		    continue;
	    }

	    if (!DIR_IS_FILE_OR_SUBDIR(&dir)) continue;

	    filenameIsDir=DIR_IS_SUBDIR(&dir);

	    if (!filenameIsDir) {
		if(dir.name[8]!=FILE_EXT) continue;
		if(dir.name[9]=='~') continue;
	    }

	    createFilename(filename,dir);

	    SERIAL_PROTOCOL(path);
	    SERIAL_PROTOCOLLN(filename);
	}
    }
}

void CardReader::ls() 
{
    char path[13*5];
    path[0] = 0;
    lsDive(path, sizeof(path), root);
}

bool CardReader::initsd()
    {
#ifdef ULTRA_LCD
    uint8_t cardSpeed = SPI_NO_INIT;	// already setup via SPI lib
    // need slowest speed for card setup
    SPI.setClockDivider(SPI_CLOCK_DIV128);
#else
    uint8_t cardSpeed = SPI_HALF_SPEED;
#endif
    cardOK = false;
    if(root.isOpen())
    root.close();
    if (!card.init(cardSpeed,SDSS)) {
	SERIAL_ECHO_START;
	SERIAL_ECHOLNPGM(MSG_SD_INIT_FAIL);
#ifdef ULTRA_LCD
	// back to full speed
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	lcd_error(F("SD Init Failed"));
#endif
	return false;
    }

#ifdef ULTRA_LCD
    // back to full speed
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    lcd_clearError();
#endif

    if (!volume.init(&card)) {
	SERIAL_ERROR_START;
	SERIAL_ERRORLNPGM(MSG_SD_VOL_INIT_FAIL);
    } else if (!root.openRoot(&volume)) {
	SERIAL_ERROR_START;
	SERIAL_ERRORLNPGM(MSG_SD_OPENROOT_FAIL);
    } else {
	cardOK = true;
	SERIAL_ECHO_START;
	SERIAL_ECHOLNPGM(MSG_SD_CARD_OK);
    }
    workDir=root;
    curDir=&root;

    workDirName[0] = '/';
    workDirName[1] = 0;
    pathDepth = 0;	// root
    pathStack[0] = 0;

    return true;
}

void CardReader::setroot()
{
  workDir=root;
  
  curDir=&workDir;
}
void CardReader::release()
{
  sdprinting = false;
  cardOK = false;
}

void CardReader::startFileprint()
{
  if(cardOK)
  {
    sdprinting = true;
    startTime = millis();
    
  }
}

void CardReader::pauseSDPrint()
{
  if(sdprinting)
  {
    sdprinting = false;
  }
}



void CardReader::openFile(char* name,bool read)
{
  if(!cardOK)
    return;
  file.close();
  sdprinting = false;
  
  
  SdFile myDir;
  curDir=&root;
  char *fname=name;
  
  char *dirname_start,*dirname_end;
  if(name[0]=='/')
  {
    dirname_start=strchr(name,'/')+1;
    while(dirname_start>0)
    {
      dirname_end=strchr(dirname_start,'/');
      //SERIAL_ECHO("start:");SERIAL_ECHOLN((int)(dirname_start-name));
      //SERIAL_ECHO("end  :");SERIAL_ECHOLN((int)(dirname_end-name));
      if(dirname_end>0 && dirname_end>dirname_start)
      {
        char subdirname[13];
        strncpy(subdirname, dirname_start, dirname_end-dirname_start);
        subdirname[dirname_end-dirname_start]=0;
        SERIAL_ECHOLN(subdirname);
        if(!myDir.open(curDir,subdirname,O_READ))
        {
          SERIAL_PROTOCOLPGM(MSG_SD_OPEN_FILE_FAIL);
          SERIAL_PROTOCOL(subdirname);
          SERIAL_PROTOCOLLNPGM(".");
          return;
        }
        else
          ;//SERIAL_ECHOLN("dive ok");
          
        curDir=&myDir; 
        dirname_start=dirname_end+1;
      }
      else // the reminder after all /fsa/fdsa/ is the filename
      {
        fname=dirname_start;
        //SERIAL_ECHOLN("remaider");
        //SERIAL_ECHOLN(fname);
        break;
      }
      
    }
  }
  else //relative path
  {
    curDir=&workDir;
  }
  if(read)
  {
    if (file.open(curDir, fname, O_READ)) 
    {
      filesize = file.fileSize();
      SERIAL_PROTOCOLPGM(MSG_SD_FILE_OPENED);
      SERIAL_PROTOCOL(fname);
      SERIAL_PROTOCOLPGM(MSG_SD_SIZE);
      SERIAL_PROTOCOLLN(filesize);
      sdpos = 0;
      
      SERIAL_PROTOCOLLNPGM(MSG_SD_FILE_SELECTED);
      LCD_MESSAGE(fname);

      // record path stack for file and file name (needed for recovery)
      memcpy(filePathStack, pathStack, sizeof(pathStack));
      strncpy(curFilename, fname, sizeof(curFilename));
      curFilename[sizeof(curFilename)-1] = 0;
      filePathDepth = pathDepth;
      MYSERIAL.print(F("echo:pathDepth "));
      MYSERIAL.println(pathDepth);
    }
    else
    {
      SERIAL_PROTOCOLPGM(MSG_SD_OPEN_FILE_FAIL);
      SERIAL_PROTOCOL(fname);
      SERIAL_PROTOCOLLNPGM(".");
    }
  }
  else 
  { //write
    if (!file.open(curDir, fname, O_CREAT | O_APPEND | O_WRITE | O_TRUNC))
    {
      SERIAL_PROTOCOLPGM(MSG_SD_OPEN_FILE_FAIL);
      SERIAL_PROTOCOL(fname);
      SERIAL_PROTOCOLLNPGM(".");
    }
    else
    {
      saving = true;
      SERIAL_PROTOCOLPGM(MSG_SD_WRITE_TO_FILE);
      SERIAL_PROTOCOLLN(name);
      LCD_MESSAGE(fname);
    }
  }
  
}

void CardReader::removeFile(char* name)
{
  if(!cardOK)
    return;
  file.close();
  sdprinting = false;
  
  
  SdFile myDir;
  curDir=&root;
  char *fname=name;
  
  char *dirname_start,*dirname_end;
  if(name[0]=='/')
  {
    dirname_start=strchr(name,'/')+1;
    while(dirname_start>0)
    {
      dirname_end=strchr(dirname_start,'/');
      //SERIAL_ECHO("start:");SERIAL_ECHOLN((int)(dirname_start-name));
      //SERIAL_ECHO("end  :");SERIAL_ECHOLN((int)(dirname_end-name));
      if(dirname_end>0 && dirname_end>dirname_start)
      {
        char subdirname[13];
        strncpy(subdirname, dirname_start, dirname_end-dirname_start);
        subdirname[dirname_end-dirname_start]=0;
        SERIAL_ECHOLN(subdirname);
        if(!myDir.open(curDir,subdirname,O_READ))
        {
          SERIAL_PROTOCOLPGM("open failed, File: ");
          SERIAL_PROTOCOL(subdirname);
          SERIAL_PROTOCOLLNPGM(".");
          return;
        }
        else
          ;//SERIAL_ECHOLN("dive ok");
          
        curDir=&myDir; 
        dirname_start=dirname_end+1;
      }
      else // the reminder after all /fsa/fdsa/ is the filename
      {
        fname=dirname_start;
        //SERIAL_ECHOLN("remaider");
        //SERIAL_ECHOLN(fname);
        break;
      }
      
    }
  }
  else //relative path
  {
    curDir=&workDir;
  }
    if (file.remove(curDir, fname)) 
    {
      SERIAL_PROTOCOLPGM("File deleted:");
      SERIAL_PROTOCOL(fname);
      sdpos = 0;
    }
    else
    {
      SERIAL_PROTOCOLPGM("Deletion failed, File: ");
      SERIAL_PROTOCOL(fname);
      SERIAL_PROTOCOLLNPGM(".");
    }
  
}

void CardReader::getStatus()
{
  if(cardOK){
    SERIAL_PROTOCOLPGM(MSG_SD_PRINTING_BYTE);
    SERIAL_PROTOCOL(sdpos);
    SERIAL_PROTOCOLPGM("/");
    SERIAL_PROTOCOLLN(filesize);
  }
  else{
    SERIAL_PROTOCOLLNPGM(MSG_SD_NOT_PRINTING);
  }
}

void CardReader::write_command(char *buf)
{
  char* begin = buf;
  char* npos = 0;
  char* end = buf + strlen(buf) - 1;

  if((npos = strchr(buf, 'N')) != NULL) {
    begin = strchr(npos, ' ') + 1;
    end = strchr(npos, '*') - 1;
  }
  end[1] = '\r';
  end[2] = '\n';
  end[3] = '\0';
  if (file.write(begin) < 0) {
    SERIAL_ERROR_START;
    SERIAL_ERRORLNPGM(MSG_SD_ERR_WRITE_TO_FILE);
  }
}


void CardReader::checkautostart(bool force)
{
  if(!force)
  {
    if(!autostart_stilltocheck)
      return;
    if(autostart_atmillis<millis())
      return;
  }
  autostart_stilltocheck=false;
  if(!cardOK)
  {
    initsd();
    if(!cardOK) //fail
      return;
  }
  
  char autoname[30];
  snprintf(autoname,sizeof(autoname),"auto%i.g",lastnr);
  for(int8_t i=0;i<(int8_t)strlen(autoname);i++)
    autoname[i]=tolower(autoname[i]);
  dir_t p;

  root.rewind();
  
  bool found=false;
  while (root.readDir(p, NULL) > 0) 
  {
    for(int8_t i=0;i<(int8_t)strlen((char*)p.name);i++)
    p.name[i]=tolower(p.name[i]);
    //Serial.print((char*)p.name);
    //Serial.print(" ");
    //Serial.println(autoname);
    if(p.name[9]!='~') //skip safety copies
    if(strncmp((char*)p.name,autoname,5)==0)
    {
      char cmd[30];

      snprintf(cmd,sizeof(cmd),"M23 %s",autoname);
      enquecommand(cmd);
      enquecommand("M24");
      found=true;
    }
  }
  if(!found)
    lastnr=-1;
  else
    lastnr++;
}

void CardReader::closefile()
{
  file.sync();
  file.close();
  saving = false; 
}

char *CardReader::getfilename(const uint16_t index)
{
#ifdef DEBUG
    char *filename = getFilenameFromIndex(workDir, index);
    MYSERIAL.print("filename: ");
    MYSERIAL.println(filename);
    return filename;
#else
    return getFilenameFromIndex(workDir, index);
#endif
}

char *CardReader::getfilename(SdFile &dir, const uint16_t index)
{
#ifdef DEBUG
    char *filename = getFilenameFromIndex(dir, index);
    MYSERIAL.print("filename: ");
    MYSERIAL.println(filename);
    return filename;
#else
    return getFilenameFromIndex(dir, index);
#endif
}

uint16_t CardReader::getnrfilenames()
{
  nrFiles = countFiles(workDir);
  return nrFiles;
}

bool CardReader::chdir(uint16_t *path, uint8_t depth)
{
    uint8_t ind;
    SdFile dir, next;

    MYSERIAL.print(F("echo:chdir depth="));
    MYSERIAL.println(depth);

    if (depth == 0) {
	workDir = root;
	workDirName[0] = '/';
	workDirName[1] = 0;
	return true;
    }

    getfilename(root, path[0]);

    if (dir.open(root, filename, O_READ) == 0) {
	MYSERIAL.print(F("Can't open directory[0]="));
	MYSERIAL.println(path[0]);
	return false;
    }
    MYSERIAL.print(F("echo:chdir "));
    MYSERIAL.println(longFilename);

    for (ind=1; ind<depth; ind++) {
	// Get the directory name
	getfilename(dir, path[ind]);
	MYSERIAL.print(F("echo:chdir "));
	MYSERIAL.println(longFilename);
	if (next.open(dir, filename, O_READ) == 0) {
	    MYSERIAL.print(F("Can't open directory["));
	    MYSERIAL.print(ind);
	    MYSERIAL.print(F("]="));
	    MYSERIAL.println(path[ind]);
	    return false;
	}
	dir.close();
	dir = next;
    }

    workDir = dir;
    curDir = &workDir;
    strncpy(workDirName, longFilename, sizeof(workDirName));

    return true;
}

/*
 * Change directories to the specified subdirectory,
 * push the directory onto the path stack
 * pathStack[0] is the first directory from the root
 * pathDepth == 0 when in the root
 */
void CardReader::pushDir(const char *subdirname, uint16_t index)
{
    SdFile dir;
    SdFile *parent=&root;

    if (pathDepth >= (sizeof(pathStack) / sizeof(pathStack[0]))) {
	MYSERIAL.print(F("Can't open directory, at max depth."));
	return;
    }
  
    if(workDir.isOpen()) {
	parent=&workDir;
    }

    if(!dir.open(*parent, subdirname, O_READ)) {
	SERIAL_ECHO_START;
	SERIAL_ECHOPGM(MSG_SD_CANT_ENTER_SUBDIR);
	SERIAL_ECHOLN(subdirname);
    } else {
	getfilename(*parent, index);
	strncpy(workDirName, longFilename, sizeof(workDirName));
	workDir=dir;
	pathStack[pathDepth++] = index;
    }
}

/*
 * Pop directory up one level
 */
bool CardReader::popDir(void)
{
    SdFile dir, next;

    if (pathDepth > 0) {
	pathDepth--;
    }

    return chdir(pathStack, pathDepth);
}

void CardReader::printingHasFinished()
{
 st_synchronize();
 quickStop();
 sdprinting = false;
 if(SD_FINISHED_STEPPERRELEASE)
 {
   //finishAndDisableSteppers();
   enquecommand(SD_FINISHED_RELEASECOMMAND);
 }
 autotempShutdown();
}

bool CardReader::recover(void)
{
    uint8_t retry=0;
    bool recoverFile = false;

    MYSERIAL.println(F("echo:attempting SD card recovery"));

    if (file.isOpen()) {
	/* recover file also */
	file.close();
	recoverFile = true;
    }

    while (!initsd()) {
	retry++;
	if (retry >= 50) {
	    return false;
	}
	MYSERIAL.print(F("echo: retry "));
	MYSERIAL.println(retry);
	delay(500);
    }

    if (recoverFile) {
	if (!chdir(filePathStack, filePathDepth)) {
	    MYSERIAL.println(F("echo:recovery failed to chdir"));
	    return false;
	}

	if (file.open(&workDir, curFilename, O_READ)) {
	    filesize = file.fileSize();
	    if (!file.seekSet(sdpos)) {
		MYSERIAL.print(F("echo:recovery failed to seek "));
		MYSERIAL.println(sdpos);
		return false;
	    }
	} else {
	    MYSERIAL.print(F("echo:recovery failed to open "));
	    MYSERIAL.println(curFilename);
	    return false;
	}
    }

    MYSERIAL.println(F("echo:recovery suceeded"));
    return true;
}

#define ROUGH_BYTES_PER_SECOND	320L

// return number of seconds left in print
uint32_t CardReader::timeLeft()
{
    if (filesize < 1) return 0;

    uint32_t duration = millis() - startTime;

    if (sdpos < (120*ROUGH_BYTES_PER_SECOND)) {
	return (filesize - sdpos) / ROUGH_BYTES_PER_SECOND;
    } else {
	if (duration == 0) return 0;
	// estimate based on time taken so far
	return (filesize - sdpos) / ((sdpos * 1000) / duration);
    }
}

#endif //SDSUPPORT
