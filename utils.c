#include "utils.h"

//Credit: http://stackoverflow.com/questions/3769405/determining-cpu-utilization
//Needs to be tested on hardware
double get_local_usage() {
  long double a[4], b[4], loadavg;
  FILE *fp;

  fp = fopen("/proc/stat","r");
  if (!fp) {
    return -1;
  }
  fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
  fclose(fp);
  sleep(1);

  fp = fopen("/proc/stat","r");
  if (!fp) {
    return -1;
  }
  fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
  fclose(fp);

  loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));

  return loadavg;
}

//Another contender for this function
//Needs to be tested on hardware
//Credit: https://www.raspberrypi.org/forums/viewtopic.php?t=64835&p=479657 : should work on raspberrypi
// double get_local_usage() {
//    int FileHandler;
//    char FileBuffer[1024];
//    double load;
//
//    FileHandler = open("/proc/loadavg", O_RDONLY);
//    if(FileHandler < 0) {
//       return -1; }
//    read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
//    sscanf(FileBuffer, "%f", &load);
//    close(FileHandler);
//    return load;
// }
