#include "lidar.h"
#include <iostream>
unsigned char buffer_rangedata[2048]={0};
Dev::Dev()
{
}

Dev::~Dev()
{
}
void Dev::ParseData_serial(rangedata *dataPack, int &fps)
{
    parse(rcvbuffer_, data_num_per_pack_, dataPack);

    fps = data_rate_hz_;
}
void Dev::Parse_hex_serial_rawdata(char send_buff[],int &fps)
{
	//printf("0x%x %x %x %x\n",rcvbuffer_[0],rcvbuffer_[1],rcvbuffer_[2],rcvbuffer_[3]);

	//rev_buff = rcvbuffer_;
	memcpy(send_buff,rcvbuffer_,data_rate_hz_);
	fps = data_rate_hz_;
}


void Dev::parse(unsigned char *in_buffer, const int in_numData,rangedata *out_dataPack)
{
    // The size of each data is 3 bytes
    const int data_size = 3;
    // in_numData: sampling number in every data package
    int id_start = 2;
    float FA = (in_buffer[id_start+1] - 0xA0 + in_buffer[id_start]/256.0) * 4;

    int id_LA_start = id_start + in_numData * data_size + 2;
    float LA = (in_buffer[id_LA_start+1] - 0xA0 + in_buffer[id_LA_start]/256.0) * 4;

    if (LA < FA) { LA += 360; }

    float dAngle = (LA - FA) / (in_numData - 1);        // angle info for each sampling

    unsigned char data[3];
    int pre_bytes = 4;          // 4 bytes before sampling data in each data package
    // calc speed (rpm)
    unsigned int speed = (in_buffer[1] << 8 | in_buffer[0]) / 64;

    for (int i=0; i<in_numData; ++i)
    {
        double angle_cur = FA + dAngle * i;
        if (angle_cur > 360)
        {
            angle_cur -= 360;
        }

        out_dataPack[i].angle = angle_cur;

        memcpy(data, in_buffer + pre_bytes + i * data_size, sizeof(unsigned char) * data_size);
        out_dataPack[i].flag = (data[1] >> 7) & 0x01;
        out_dataPack[i].dist = ((data[1] & 0x3F) << 8) | data[0];
        out_dataPack[i].speed = speed;
    }
}

int Dev::ReadData_serial(unsigned int TimeOut_ms)
{
    if (data_timer_.Duation_ms() > 1000)
    {
        data_rate_hz_ = data_num_total_;
        data_num_total_ = 0;
        data_timer_.InitTimer();
    }

    char rtn;                                                // Returned value from Read
    bool found= false;
    int find_times = 0;
    int numDataByte;
    BYTE curByte = NULL, preByte = NULL;

    while(! found)
    {
        rtn=serial_.readChar(&curByte, TimeOut_ms);
        if (rtn==1)                                                 // If a byte has been read
        {

            found = (preByte == BYTE(0x55)) && (curByte == BYTE(0xaa));
            numDataByte = 2;
            preByte = curByte;
        }else
        {
            return rtn;
        }

        ++find_times;
        if (find_times >= 100)
        {
            // Find data header time out!
            return -1;
        }
    }

    BYTE buff[128] = {0};
    int buff_length_target = 34;
    const int data_num_target = 8;
    rtn = serial_.readChars(buff, buff_length_target, TimeOut_ms);
    if (rtn != buff_length_target)
    {
        return rtn;
    }

    bool is_equal = CheckData(buff);
	printf("is_equal:%d buf[1]:%d\n",is_equal,buff[1]);
    if (!is_equal || data_num_target != buff[1])
    {
		printf("Serial Check data failed!\n");
        return -1;
    }

    unsigned int counts = data_num_target * 3 + 8;
    memset(rcvbuffer_, 0, 2048);
    memcpy_s(rcvbuffer_, 2048, buff + numDataByte, counts);
    data_num_per_pack_ = data_num_target;
    data_num_total_ += data_num_per_pack_;
    return data_num_target;
}

bool Dev::CheckData(BYTE *buffer)
{
    int temp[17] = {0};
    temp[0] = 0x55 + (0xAA << 8);
    for (int i = 1; i < 17; i++)
    {
        temp[i] = buffer[2 * (i-1)] + (buffer[2 * (i-1) + 1] << 8);
    }

    int chk32 = 0;
    for (int i = 0; i < 17; i++)
    {
        chk32 = (chk32 << 1) + temp[i];
    }

    int checksum_target = (chk32 & 0x7FFF) + (chk32 >> 15);
    checksum_target = checksum_target & 0x7FFF;
    int checksum_cur = buffer[32] + (buffer[33] << 8);
    bool is_equal = (checksum_target == checksum_cur);
    return is_equal;
}

int Dev::readByte(char *out_byte, unsigned int TimeOut_ms)
{
    int rtn = serial_.readChar(out_byte, TimeOut_ms);
    return rtn;
}

int Dev::openSerial(char *port, unsigned int bauds)
{
    return serial_.openDevice(port, bauds);
}

void Dev::closeSerial()
{
    serial_.closeDevice();
}