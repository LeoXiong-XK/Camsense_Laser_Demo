#include <iostream>
#include <fstream>
#include <thread>
#include <stack>
#include <time.h>
#include <GL\freeglut.h>
#include <numeric>
#include <vector>
#include "base\lidar.h"
#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\filereadstream.h"
#include "rapidjson\filewritestream.h"
#include "rapidjson\prettyWriter.h"

#include "base\CClient_Wins.h"
#include "base\CServer_Wins.h"


#pragma comment(lib, "ws2_32.lib")

using namespace std;
char m_client_buff[2048] = {0};

#define PI 3.141592653589793
#define DEGREETORAD (PI/180.)
#define psln(x) {std::cout << #x " = " << std::endl << (x) << std::endl;}
#define PAUSE {system("pause");}
#define SERIAL_READ_TIMEOUT	100

Dev device;
std::string json_allConfig = "M:\\WorkSpace\\PrivateDoc\\PraviteCode\\Camsense_Laser_Demo\\Camsense_SDK_v1.5\\bin\\lib\\Release\\config\\all_config.json";
int databuff = 600;
rapidjson::Document config_doc;
rangedata dataPack[8];
std::vector<rangedata> dataset;
int fps = 0;
bool LEFTBUTTON = false, RIGHTBUTTON = false, SHIFTBTN = false, MIDDLEBTN = false;
bool dataEnable = false, is_display = false, is_record_data = false;
bool is_data_available = false, is_serial_available = false;
int MOUSEPOS[2];
double PHI = 0, THETA = 90, DEPTH = -6, X_SHIFT = 0.0f, Y_SHIFT = 0.0f;
double DIST_MEAN = 0.0, DIST_STD = 0.0;
std::vector<double> dist_set;
double SCALE = 2;
int WIDTH = 640 * SCALE, HEIGHT = 480 * SCALE;
int POINT_SIZE = 3.0*SCALE;
int NUM_POINT = 3000, DATA_RANGE = 2, angle_offset = 0;
int rtn;
char buff[256];
std::ofstream outFile_rangeData;
HWND hWnd_main, hWnd_rangeView;


//----------------------------------------------------------
//服务器和客户端
CServer_Wins	m_sServer;//服务器端实例化的时候就已经初始化
CClient_Wins	m_sClient;
#define				SVR_IP	"127.0.0.1"
#define				SVR_PORT	6000  //CServer_Wins和CClient_Wins两个类已经默认端口号是6000
#define				BUFF_SIZE	2048 
char					m_revData[BUFF_SIZE] = {0};

bool m_bSendFlag;
// 临界区结构对象
CRITICAL_SECTION g_cs;
//----------------------------------------------------------


// Calculate mean & precision of distance
void CalcMeanStd(const std::vector<double>& in_data, double& dist_mean, double& dist_std)
{
    double sum = 0.0;
    size_t data_len = in_data.size();
    if (data_len == 0)
    {
        dist_mean = 0;
        dist_std = 0;
        return;
    }

    for (size_t i = 0; i < data_len; ++i)
    {
        sum += in_data[i];
    }
    dist_mean = sum / data_len;

    sum = 0.0;
    for (size_t i = 0; i < data_len; ++i)
    {
        sum += std::abs(in_data[i] - dist_mean);
    }
    double tmp = sum / data_len;
    dist_std = (tmp / dist_mean) * 100.0;
}

int LoadConfigFile(char *configfile, rapidjson::Document &config_doc)
{
    FILE * fp = fopen(configfile, "r");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    config_doc.Clear();
    if (config_doc.ParseStream(is).HasParseError())
    {
        fclose(fp);
        printf("Unable to load json file!\n");
        PAUSE;
        return -1;
    }

    printf("Load json file successfully!\n");
    fclose(fp);
    return 0;
}

void initScene(void)
{
    float bg_color = 0.0;
    glClearColor(bg_color, bg_color, bg_color, 0.0);
    glEnable(GL_DEPTH_TEST | GL_COLOR_MATERIAL);// need enable color material to draw colors for obj
    glShadeModel(GL_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(POINT_SIZE);
}

void drawCircle(float radius)
{
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 360; i++)
    {
        float degInRad = i * DEGREETORAD;
        glVertex3f(cos(degInRad)*radius, 0, sin(degInRad)*radius);
    }

    glEnd();
}

void MapWorld()
{
    float maxRange = 8;
    float interval = 0.1; //m
    int numLine = maxRange / interval;
    float color_base = 0.0;

    for (int i = 0; i <= numLine; i++)
    {
        if (i % 10 != 0)
        {
            float color_m = std::abs(color_base - 0.1);
            glColor3f(color_m, color_m, color_m);
        } else if (i % 60 == 0)
        {
            glColor3f(0.0f, 0.5f, 0.5f);
        } else
        {
            float color_dm = std::abs(color_base - 0.4);
            glColor3f(color_dm, color_dm, color_dm);
        }

        drawCircle(i*interval);
    }

    float a = 3;

    glBegin(GL_LINES);
    // x axis
    glColor3f(0.8f, 0.0f, 0.0f);
    glVertex3f(a, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    // x axis(negative)
    glColor3f(0.4f, 0.4f, 0.4f);
    glVertex3f(-a, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);

    // z axis
    glColor3f(0.0f, 0.0f, 0.8f);
    glVertex3f(0.0f, 0.0f, a);
    glVertex3f(0.0f, 0.0f, 0.0f);
    // z axis(negative)
    glColor3f(0.4f, 0.4f, 0.4f);
    glVertex3f(0.0f, 0.0f, -a);
    glVertex3f(0.0f, 0.0f, 0.0f);

    // y axis
    glColor3f(0.0f, 0.8f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, a, 0.0f);

    glEnd();

    // show line at dist_mean
    if (is_display)
    {
        glBegin(GL_LINES);
        float line_len = 1.0;
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f(-DIST_MEAN, 0.0f, -line_len);
        glVertex3f(-DIST_MEAN, 0.0f, line_len);
        glEnd();
    }
}

// color_mode: 1->green, 0->red
void showTxt(char *buff, int color_mode = 1)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 320, 0, 60);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (1 == color_mode)
    {
        glColor3f(0, 1, 0);//needs to be called before RasterPos
    } else
    {
        glColor3f(1, 0, 0);
    }

    glRasterPos2i(0, 40);
    void * font = GLUT_BITMAP_HELVETICA_18;
    std::string txt = buff;

    for (std::string::iterator i = txt.begin(); i != txt.end(); ++i)
    {
        char c = *i;
        glutBitmapCharacter(font, c);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

void drawRangeData()
{

    glBegin(GL_POINTS);
    glPointSize(POINT_SIZE);
/*	
	 bool bRtn = m_sServer.Accept();

	 if (m_bSendFlag)
	 {

		 //服务器端收到数据
		 memset(m_revData, 0, BUFF_SIZE);
		 printf("Tip:SVR prepare to receive data--->\n");
		 m_sServer.RecMsg(m_revData);
		 //数据判断
		 if (strlen(m_revData) > 0)
		 {
			 //----------------------------------------------------------------------------------
			 //解析收到的数据
			 rangedata data[8];
			 memset(&data, 0, sizeof(data));
			 memcpy(&data, m_revData, sizeof(data));
			 //数据打包
			 for (int i = 0; i < 8;i++) {
				 dataset.push_back(data[i]);
				 if (dataset.size() > databuff) {
					 dataset.erase(dataset.begin());
				 }
			 }
			 printf("dataset:%d\n",dataset.size());
			 //----------------------------------------------------------------------------------
			 m_bSendFlag = false;
		 }
		 else
		 {
			 std::cout << "Error：Null recv data error!\n" << std::endl;
			 //return;
		 }


	 }
	*/ 
		//---------------------------------------------------------------------------------- 
		//装满集合后
		if (dataset.size() == databuff)
		{
			for (int i = 0; i < databuff; i++)
			{
				rangedata range_cur = dataset[i];
				double dist = (range_cur.dist);         // -0.001)/cos(yaw);
				double angle_cur = fmod(range_cur.angle + angle_offset, 360.0);
				double x = dist * std::cos(angle_cur * PI / 180);
				double z = dist * std::sin(angle_cur * PI / 180);
				double x_abs = std::abs(x / 1000.);
				double z_abs = std::abs(z / 1000.);

            if (dataset[i].flag)
            {
                //glColor3f(1.0f, 0.0f, 0.0f);          // invalid points
            } else
            {
                glColor3f(1.0f, 1.0f, 1.0f);

                //calc mean & std distance
                if (is_display)
                {
                    if (angle_cur >= (180 - DATA_RANGE)
                        && angle_cur <= (180 + DATA_RANGE))
                    {
                        glColor3f(1.0f, 0.0f, 0.0f);            // marked as red
                        if (is_record_data)
                        {
                            outFile_rangeData << dist << " "
                                << x_abs << " "
                                << z_abs << endl;
                        }

                        dist_set.push_back(x_abs);
                        if (dist_set.size() > NUM_POINT)
                        {
                            CalcMeanStd(dist_set, DIST_MEAN, DIST_STD);
                            dist_set.erase(dist_set.begin());
                        }
                    }
                }
            }

            glVertex3f(x / 1000., 0.0, z / 1000.);
        }


			//-------------------------------------------------------
			//打包清空
			//dataset.erase(dataset.begin());
			//-------------------------------------------------------

		//}

    }

    glEnd();

    // Display mean & std distance
    if (is_display)
    {
        glViewport(10, HEIGHT - 30, 320, 10);
        sprintf_s(buff, "Mean = %.2f, Precision = %.2f%% (for %d points)", DIST_MEAN, DIST_STD, NUM_POINT);
        showTxt(buff);
    }

    // Display spin velocity
    if (dataset.size() > 0)
    {
        glViewport(0, 0, 320, 10);
        sprintf_s(buff, "FPS: %d, speed: %d/rpm",
            fps, dataset[0].speed);
        if (angle_offset > 0)
        {
            sprintf_s(buff, "FPS: %d, speed: %d/rpm, angle_offset: %.2f°",
                fps, dataset[0].speed, angle_offset);
        }

        showTxt(buff);
    }

	//Sleep(10);
	//// 离开临界区
	//LeaveCriticalSection(&g_cs);

}


void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear info
    glEnable(GL_DEPTH_TEST); //depth test should be enabled
    glLoadIdentity();// reset the drawing perspeive

    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0,                //camera view angle;
        (double)WIDTH / (double)HEIGHT,// width-to-height ratio
        0.1,                 // near 0.1 unit will not be shown
        200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(X_SHIFT, Y_SHIFT, DEPTH);//translate the whole scene

    glRotatef(THETA, 1.0f, 0.0f, 0.0f);
    glRotatef(PHI, 0.0f, 1.0f, 0.0f);

    MapWorld();
    if (is_data_available)
    {
        drawRangeData();//数据显示
    }

    glutSwapBuffers();
}

void glutTimer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(3, glutTimer, 1);
}

void Mouse(int button, int state, int mx, int my)
{
    int specialkey = glutGetModifiers();
    MOUSEPOS[0] = mx;
    MOUSEPOS[1] = my;
    LEFTBUTTON = ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN));
    RIGHTBUTTON = ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN));
    MIDDLEBTN = ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN));
    //SHIFTBTN = ((specialkey==GLUT_ACTIVE_SHIFT)&& (state == GLUT_DOWN));

    if (state == GLUT_UP && button == 3)
    {
        DEPTH += 0.1;
        glutPostRedisplay();
    }
    if (state == GLUT_UP && button == 4)
    {
        DEPTH -= 0.1;
        glutPostRedisplay();
    }
}

void MouseMotion(int x, int y)
{

    if (LEFTBUTTON)
    {
        X_SHIFT += (float)(x - MOUSEPOS[0])*(abs(DEPTH) + 1) / 650;
        Y_SHIFT += (float)(MOUSEPOS[1] - y)*(abs(DEPTH) + 1) / 650;
    } else if (RIGHTBUTTON)
    {
        PHI -= (float)(x - MOUSEPOS[0]) / 4.0;
        THETA -= (float)(MOUSEPOS[1] - y) / 4.0;
    } else if (MIDDLEBTN)
    {
        PHI = 0;
        THETA = 90;
    }

    MOUSEPOS[0] = x;
    MOUSEPOS[1] = y;
    glutPostRedisplay();
}

void CheckSerial()
{
    int count = 0;
    while (true)
    {
        char byte_cur = NULL;
        int rtn = device.readByte(&byte_cur, SERIAL_READ_TIMEOUT/*60*/);
        if (rtn > 0)
        {
            is_serial_available = true;
            return;
        } else
        {
            SwitchToThisWindow(hWnd_main, 1);
            is_data_available = false;
            is_serial_available = false;
            printf("Warning: Invalid serial data!\n");
            Sleep(500);
            count++;
            if (count > 10)
            {
                printf("\n\n");
                return;
            }
        }
    }
}

void SerialDataThread()
{
    int rtn;
	static int nNr = 0;

	//初始化创建客户端，客户端连接服务器
	if ( m_sClient.Sendbefore(SVR_IP) )
	{
		printf("Tip: Init client OK and connect with server successfully!\n");
	}
	else
	{
		printf("Error: Init client or connect with server fail!\n");
		dataEnable = false;
	}

	while (dataEnable)
	{
		//if (!m_bSendFlag)
		{
			//if (nNr == 0)
			{
			
			rtn = device.ReadData_serial(SERIAL_READ_TIMEOUT/*60*/);

			if (rtn <= 0)
			{
				if (rtn == -3) {
					printf("ROC Serial buffer is full!\n");
				}
				is_data_available = false;
				SwitchToThisWindow(hWnd_main, 1);
				CheckSerial();
				ShowWindow(hWnd_main, SW_SHOWNOACTIVATE);
				SwitchToThisWindow(hWnd_rangeView, 1);
				std::cout << "is_serial_available:" << is_serial_available << std::endl;
				if (is_serial_available)
				{
					printf("Read serial data successfully!\n");
					Sleep(100);
				}
				else
				{
					printf("Unable to read serial data!\n");
					printf("Try to open serial port...\n");
					device.closeSerial();
					rtn = LoadConfigFile(const_cast<char*>(json_allConfig.c_str()), config_doc);
					printf("Error: Unable to load json file!\n");
					PAUSE;

					rapidjson::Value &config = config_doc["rangeView"];
					int port = config["port"].GetInt();
					int baud = config["baudRate"].GetInt();
					char com[8];
					sprintf_s(com, "com%d", port);
					printf("rangeView port=%s, baudRate=%d\n", com, baud);

					rtn = device.openSerial(com, baud);
					if (rtn < 0)
					{
						sprintf_s(buff, "Error: Open serial port (%s)！", com);
						printf("%s\n", buff);
					}
					else
					{
						printf("Open serial port successfully!\n");
						is_serial_available = true;
					}
				}
				continue;
			}
			else
			{
				is_data_available = true;
			}
			
			//device.ParseData_serial(dataPack, fps);
			
			//DWORD start_ts = GetTickCount();
			/*
			for (int i = 0; i < 8; i++)
			{
				dataPack[i].flag = true;
				dataPack[i].angle = (68.89f+10*i);
				dataPack[i].dist = (1500+100*i);
				dataPack[i].speed = 312;// (290 + 2 * i);
			}*/

			
			if (m_sServer.m_bLinkFlag)
			{

				//数据打包和发送
				printf("Tip:Client prepare to send data--->\n");
				device.ParseData_serial(dataPack, fps);
				memcpy(m_client_buff,dataPack,8*sizeof(dataPack[0]));
				m_sClient.SendMsg(SVR_IP, m_client_buff, 8 * sizeof(dataPack[0]));
				m_bSendFlag = true;
			}
			
			//DWORD end_ts = GetTickCount();
			//DWORD cur_ts = start_ts - end_ts;
			//std::cout << "The run time is:" << cur_ts << "ms." << std::endl;
			//Sleep(10);
		}
		}
    }
}
void tcp_ReceiveThread() {

	bool bRtn = m_sServer.Accept();
	while (true) {
		//服务器端收到数据
		memset(m_revData, 0, BUFF_SIZE);
		printf("Tip:SVR prepare to receive data--->\n");
		m_sServer.RecMsg(m_revData);
		//数据判断
		if (strlen(m_revData) > 0)
		{
			//----------------------------------------------------------------------------------
			//解析收到的数据
			rangedata data[8];
			memset(&data, 0, sizeof(data));
			memcpy(&data, m_revData, sizeof(data));
			//数据打包
			for (int i = 0; i < 8; i++) {
				dataset.push_back(data[i]);
				if (dataset.size() > databuff) {
					dataset.erase(dataset.begin());
				}
			}
			printf("dataset:%d\n", dataset.size());
			//----------------------------------------------------------------------------------
			m_bSendFlag = false;
		}
		else
		{
			std::cout << "Error：Null recv data error!\n" << std::endl;
			//return;
		}
		Sleep(10);
	}
}
void GL_RangeViewThread()
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(500/*10*/, 20/*10*/);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("World coordinate");
    hWnd_rangeView = FindWindow(NULL, "World coordinate");
    initScene();
    glutDisplayFunc(drawScene);//关联显示函数
    glutMouseFunc(Mouse);
    glutMotionFunc(MouseMotion);
    glutTimerFunc(3, glutTimer, 1);
    glutMainLoop();//keep going
}

int LoadConfig()
{
    rtn = LoadConfigFile(const_cast<char*>(json_allConfig.c_str()), config_doc);
    rapidjson::Value &config = config_doc["rangeView"];
	int port = config["port"].GetInt();
	int baud = config["baudRate"].GetInt();
    char com[8];
    printf("ConfigFile Try to open serial port...\n");
    sprintf_s(com, "com%d", port);
    printf("rangeView port=%s, baudRate=%d\n", com, baud);

    rtn = device.openSerial(com, baud);
    if (rtn<0)
    {
        printf("Error: Open serial port fail!\n");
        PAUSE;
        exit(0);
    }

    databuff = config["data_buff"].GetInt();
    angle_offset = config["angle_offset"].GetInt();
    DATA_RANGE = config["data_range"].GetInt();
    NUM_POINT = config["num_point"].GetInt();
    is_display = config["is_display"].GetInt() > 0;
    is_record_data = config["is_record_data"].GetInt() > 0;
    SCALE = config["map_scale"].GetDouble();
    WIDTH = 640 * SCALE, HEIGHT = 480 * SCALE;
    POINT_SIZE = 3.0*SCALE;

    if (is_record_data)
    {
        char cmd[128];
        sprintf_s(cmd, "rangeViewData.txt");
        outFile_rangeData.open(cmd);
    }
    return 0;
}

int main(int argc, char ** argv)
{
    hWnd_main = GetForegroundWindow();
	char IP[] = "127.0.0.1";
    rtn = LoadConfig();

	//初始化服务器
	//m_sServer;//注意：服务器端实例化的时候就已经初始化

    dataEnable = true;
	std::thread recThread(tcp_ReceiveThread);//Sercer端接收数据
    std::thread sThread(SerialDataThread);//串口数据转Client端发送
    glutInit(&argc, argv);
    std::thread gThread(GL_RangeViewThread);//服务器端接收数据后显示
    sThread.join();
	recThread.join();
    return 0;
}

