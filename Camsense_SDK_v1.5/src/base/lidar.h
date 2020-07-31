#include "rOc_serial.h"
#include "TimeOut.h"

typedef unsigned char BYTE;
extern unsigned char buffer_rangedata[2048];
struct rangedata
{
    bool flag;
    float angle;
    unsigned int dist;
    unsigned int speed;
    rangedata() :
        flag(false),
        angle(0.),
        dist(0),
        speed(0) {}
};

class Dev
{
public:
    Dev();
    ~Dev();

    int ReadData_serial(unsigned int TimeOut_ms = 60);
    void ParseData_serial(rangedata *dataPack, int &fps);
    bool CheckData(BYTE *buffer);
    int readByte(char *out_byte, unsigned int TimeOut_ms);
    int openSerial(char *port, unsigned int baud);
    void closeSerial();
    void parse(BYTE *in_buffer, const int in_numData, rangedata *out_dataPack);
	void Parse_hex_serial_rawdata( char send_buff[], int &fps);
private:
    int data_num_per_pack_;
    int data_num_total_;
    int data_rate_hz_;
    TimeOut data_timer_;
    rOc_serial serial_;
    BYTE rcvbuffer_[2048];
};

