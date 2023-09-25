#define FILENAME "sweds.ino"
#define MYVERSION "4.18"
#define BUILD_DATE "9/15/23"
#define CONFIGADDR 10
#define MODEADDR 20
#define LEDBASEADDR 30
#define MYFIRMWARE "argon_4.10"

struct MyConfig
{
    int initialized;
    bool motionEnabled;
    bool gestureArmed;
    int mode;
    int awayHoldTMR;
};

struct MyLEDConfig
{
    int stripid;
    int red;
    int green;
    int blue;
    int white;
};

// Prototypes for local build, ok to leave in for Build IDE
// int ledConfig(String command);
// void juiceLeds(int stripId, int ured, int ugreen,int ublue, int uwhite);
// void myHandler(const char *event, const char *data);
// void ledhandler(const char *event, const char *data);