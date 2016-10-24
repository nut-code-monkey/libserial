/**
 * @file LibSerialTest.cpp
 * @copyright LibSerial
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include <gtest/gtest.h>
#include <SerialPort.h>
#include <SerialStream.h>

// Default Serial Port and Baud Rate.
#define TEST_SERIAL_PORT   "/dev/ttyUSB0"
#define TEST_SERIAL_PORT_2 "/dev/ttyUSB1"

using namespace LibSerial;

class LibSerialTest
    : public ::testing::Test
{
public:
    LibSerialTest() : serialPort(TEST_SERIAL_PORT), serialPort2(TEST_SERIAL_PORT_2) {} 

protected:
    SerialPort serialPort ;
    SerialPort serialPort2 ;


    virtual void SetUp()
    {
        writeString = "Quidquid latine dictum sit, altum sonatur. (Whatever is said in Latin sounds profound.)";

        serialStreamBaudRate[0]   =  BaudRate::BAUD_50;
        serialStreamBaudRate[1]   =  BaudRate::BAUD_75;
        serialStreamBaudRate[2]   =  BaudRate::BAUD_110;
        serialStreamBaudRate[3]   =  BaudRate::BAUD_134;
        serialStreamBaudRate[4]   =  BaudRate::BAUD_150;
        serialStreamBaudRate[5]   =  BaudRate::BAUD_200;
        serialStreamBaudRate[6]   =  BaudRate::BAUD_300;
        serialStreamBaudRate[7]   =  BaudRate::BAUD_600;
        serialStreamBaudRate[8]   =  BaudRate::BAUD_1200;
        serialStreamBaudRate[9]   =  BaudRate::BAUD_1800;
        serialStreamBaudRate[10]  =  BaudRate::BAUD_2400;
        serialStreamBaudRate[11]  =  BaudRate::BAUD_4800;
        serialStreamBaudRate[12]  =  BaudRate::BAUD_9600;
        serialStreamBaudRate[13]  =  BaudRate::BAUD_19200;
        serialStreamBaudRate[14]  =  BaudRate::BAUD_38400;
        serialStreamBaudRate[15]  =  BaudRate::BAUD_57600;
        serialStreamBaudRate[16]  =  BaudRate::BAUD_115200;
        serialStreamBaudRate[17]  =  BaudRate::BAUD_230400;
        serialStreamBaudRate[18]  =  BaudRate::BAUD_460800;
        serialStreamBaudRate[19]  =  BaudRate::BAUD_500000;
        serialStreamBaudRate[20]  =  BaudRate::BAUD_576000;
        serialStreamBaudRate[21]  =  BaudRate::BAUD_921600;
        serialStreamBaudRate[22]  =  BaudRate::BAUD_1000000;
        serialStreamBaudRate[23]  =  BaudRate::BAUD_1152000;
        serialStreamBaudRate[24]  =  BaudRate::BAUD_1500000;
        serialStreamBaudRate[25]  =  BaudRate::BAUD_2000000;
        serialStreamBaudRate[26]  =  BaudRate::BAUD_2500000;
        serialStreamBaudRate[27]  =  BaudRate::BAUD_3000000;
        serialStreamBaudRate[28]  =  BaudRate::BAUD_3500000;
        serialStreamBaudRate[29]  =  BaudRate::BAUD_4000000;

        serialStreamCharSize[0] = CharSize::CHAR_SIZE_5;
        serialStreamCharSize[1] = CharSize::CHAR_SIZE_6;
        serialStreamCharSize[2] = CharSize::CHAR_SIZE_7;
        serialStreamCharSize[3] = CharSize::CHAR_SIZE_8;

        serialStreamParity[0] = SerialStreamBuf::PARITY_EVEN;
        serialStreamParity[1] = SerialStreamBuf::PARITY_ODD;
        serialStreamParity[2] = SerialStreamBuf::PARITY_NONE;

        serialStreamFlowControl[0] = SerialStreamBuf::FLOW_CONTROL_NONE;
        serialStreamFlowControl[1] = SerialStreamBuf::FLOW_CONTROL_HARD;
        serialStreamFlowControl[2] = SerialStreamBuf::FLOW_CONTROL_SOFT;

        serialPortBaudRate[0]   =  SerialPort::BAUD_50;
        serialPortBaudRate[1]   =  SerialPort::BAUD_75;
        serialPortBaudRate[2]   =  SerialPort::BAUD_110;
        serialPortBaudRate[3]   =  SerialPort::BAUD_134;
        serialPortBaudRate[4]   =  SerialPort::BAUD_150;
        serialPortBaudRate[5]   =  SerialPort::BAUD_200;
        serialPortBaudRate[6]   =  SerialPort::BAUD_300;
        serialPortBaudRate[7]   =  SerialPort::BAUD_600;
        serialPortBaudRate[8]   =  SerialPort::BAUD_1200;
        serialPortBaudRate[9]   =  SerialPort::BAUD_1800;
        serialPortBaudRate[10]  =  SerialPort::BAUD_2400;
        serialPortBaudRate[11]  =  SerialPort::BAUD_4800;
        serialPortBaudRate[12]  =  SerialPort::BAUD_9600;
        serialPortBaudRate[13]  =  SerialPort::BAUD_19200;
        serialPortBaudRate[14]  =  SerialPort::BAUD_38400;
        serialPortBaudRate[15]  =  SerialPort::BAUD_57600;
        serialPortBaudRate[16]  =  SerialPort::BAUD_115200;
        serialPortBaudRate[17]  =  SerialPort::BAUD_230400;
        serialPortBaudRate[18]  =  SerialPort::BAUD_460800;
        serialPortBaudRate[19]  =  SerialPort::BAUD_500000;
        serialPortBaudRate[20]  =  SerialPort::BAUD_576000;
        serialPortBaudRate[21]  =  SerialPort::BAUD_921600;
        serialPortBaudRate[22]  =  SerialPort::BAUD_1000000;
        serialPortBaudRate[23]  =  SerialPort::BAUD_1152000;
        serialPortBaudRate[24]  =  SerialPort::BAUD_1500000;
        serialPortBaudRate[25]  =  SerialPort::BAUD_2000000;
        serialPortBaudRate[26]  =  SerialPort::BAUD_2500000;
        serialPortBaudRate[27]  =  SerialPort::BAUD_3000000;
        serialPortBaudRate[28]  =  SerialPort::BAUD_3500000;
        serialPortBaudRate[29]  =  SerialPort::BAUD_4000000;

        serialPortCharacterSize[0] = SerialPort::CHAR_SIZE_5;
        serialPortCharacterSize[1] = SerialPort::CHAR_SIZE_6;
        serialPortCharacterSize[2] = SerialPort::CHAR_SIZE_7;
        serialPortCharacterSize[3] = SerialPort::CHAR_SIZE_8;

        serialPortParity[0] = SerialPort::PARITY_EVEN;
        serialPortParity[1] = SerialPort::PARITY_ODD;
        serialPortParity[2] = SerialPort::PARITY_NONE;

        serialPortFlowControl[0] = SerialPort::FLOW_CONTROL_NONE;
        serialPortFlowControl[1] = SerialPort::FLOW_CONTROL_HARD;
        serialPortFlowControl[2] = SerialPort::FLOW_CONTROL_SOFT;
    }


    //---------------------- Serial Stream Unit Tests -----------------------//

    void testSerialStreamOpenClose()
    {
        serialStream.Open(TEST_SERIAL_PORT);
        ASSERT_TRUE(serialStream.IsOpen());

        serialStream.Close();
        ASSERT_FALSE(serialStream.IsOpen());
    }

    void testSerialStreamReadWrite()
    {
        serialStream.Open(TEST_SERIAL_PORT);
        serialStream2.Open(TEST_SERIAL_PORT_2);

        ASSERT_TRUE(serialStream.IsOpen());
        ASSERT_TRUE(serialStream2.IsOpen());

        serialStream << writeString << std::endl;
        getline(serialStream2, readString);
        ASSERT_EQ(readString, writeString);

        serialStream.Close();
        serialStream2.Close();

        ASSERT_FALSE(serialStream.IsOpen());
        ASSERT_FALSE(serialStream2.IsOpen());


        serialStream.Open(TEST_SERIAL_PORT);
        serialStream2.Open(TEST_SERIAL_PORT_2);

        ASSERT_TRUE(serialStream.IsOpen());
        ASSERT_TRUE(serialStream2.IsOpen());        

        writeByte = 'a';
        serialStream.write(&writeByte, 1);
        serialStream << writeByte << std::endl;
        serialStream2.read(&readByte, 1);
        ASSERT_EQ(readByte, writeByte);

        serialStream.Close();
        serialStream2.Close();

        ASSERT_FALSE(serialStream.IsOpen());
        ASSERT_FALSE(serialStream2.IsOpen());


        serialStream.Open(TEST_SERIAL_PORT);
        serialStream2.Open(TEST_SERIAL_PORT_2);

        ASSERT_TRUE(serialStream.IsOpen());
        ASSERT_TRUE(serialStream2.IsOpen());        

        writeByte = 'A';
        serialStream.write(&writeByte, 1);
        serialStream << writeByte << std::endl;
        serialStream2.get(readByte);
        ASSERT_EQ(readByte, writeByte);

        serialStream.Close();
        serialStream2.Close();

        ASSERT_FALSE(serialStream.IsOpen());
        ASSERT_FALSE(serialStream2.IsOpen());
    }

    void testSerialStreamSetGetBaudRate()
    {
        serialStream.Open(TEST_SERIAL_PORT);
        ASSERT_TRUE(serialStream.IsOpen());

        size_t maxBaudIndex = 17;
        
        // // @TODO - Why don't the higher baud rates work?
        // #ifdef __linux__
        //     maxBaudIndex = 26;
            
        //     #if __MAX_BAUD > B2000000
        //         maxBaudIndex = 30;
        //     #endif
        
        // #endif

        for (size_t i = 0; i < maxBaudIndex; i++)
        {
            serialStream.SetBaudRate(serialStreamBaudRate[i]);
            BaudRate baudRate = serialStream.GetBaudRate();
            ASSERT_EQ(baudRate, serialStreamBaudRate[i]);
        }

        serialStream.Close();
        ASSERT_FALSE(serialStream.IsOpen());
    }

    void testSerialStreamSetGetCharSize()
    {
        serialStream.Open(TEST_SERIAL_PORT);
        ASSERT_TRUE(serialStream.IsOpen());

        // @TODO - Why don't the smaller Character Size values work?
        for (size_t i = 2; i < 4; i++)
        {
            serialStream.SetCharSize(serialStreamCharSize[i]);
            CharSize charSize = serialStream.GetCharSize();
            ASSERT_EQ(charSize, serialStreamCharSize[i]);
            usleep(10);
        }

        serialStream.Close();
        ASSERT_FALSE(serialStream.IsOpen());
    }

    void testSerialStreamSetGetParity()
    {
        serialStream.Open(TEST_SERIAL_PORT);
        ASSERT_TRUE(serialStream.IsOpen());

        for (size_t i = 0; i < 3; i++)
        {
            serialStream.SetParity(serialStreamParity[i]);
            SerialStreamBuf::ParityEnum parity = serialStream.GetParity();
            ASSERT_EQ(parity, serialStreamParity[i]);
            usleep(10);
        }

        serialStream.Close();
        ASSERT_FALSE(serialStream.IsOpen());
    }

    void testSerialStreamSetGetFlowControl()
    {
        serialStream.Open(TEST_SERIAL_PORT);
        ASSERT_TRUE(serialStream.IsOpen());

        for (size_t i = 0; i < 3; i++)
        {
            serialStream.SetFlowControl(serialStreamFlowControl[i]);
            SerialStreamBuf::FlowControlEnum flowControl = serialStream.GetFlowControl();
            ASSERT_EQ(flowControl, serialStreamFlowControl[i]);
            usleep(10);
        }

        serialStream.Close();
        ASSERT_FALSE(serialStream.IsOpen());
    }

    void testSerialStreamSetGetStopBits()
    {
        short numOfStopBits;

        serialStream.Open(TEST_SERIAL_PORT);
        ASSERT_TRUE(serialStream.IsOpen());

        serialStream.SetNumOfStopBits(1);
        numOfStopBits = serialStream.GetNumOfStopBits();
        ASSERT_EQ(numOfStopBits, 1);

        serialStream.SetNumOfStopBits(2);
        numOfStopBits = serialStream.GetNumOfStopBits();
        ASSERT_EQ(numOfStopBits, 2);

        serialStream.Close();
        ASSERT_FALSE(serialStream.IsOpen());
    }


    //----------------------- Serial Port Unit Tests ------------------------//

    void testSerialPortOpenClose()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortReadWrite(const int timeOutMilliseconds)
    {
        serialPort.Open();
        serialPort2.Open();
        
        ASSERT_TRUE(serialPort.IsOpen());
        ASSERT_TRUE(serialPort2.IsOpen());

        SerialPort::DataBuffer readDataBuffer;
        SerialPort::DataBuffer writeDataBuffer;

        // Test using ASCII characters.
        for (size_t i = 48; i < 122; i++)
        {
            writeDataBuffer.push_back(i);
        }

        serialPort.Write(writeDataBuffer);
        bytesRead = serialPort2.Read(readDataBuffer, 74, timeOutMilliseconds);
        ASSERT_EQ(readDataBuffer, writeDataBuffer);
        ASSERT_EQ(bytesRead, writeDataBuffer.size());
        usleep(10);

        serialPort.Close();
        serialPort2.Close();
        
        ASSERT_FALSE(serialPort.IsOpen());
        ASSERT_FALSE(serialPort2.IsOpen());
        
        serialPort.Open();
        serialPort2.Open();
        
        ASSERT_TRUE(serialPort.IsOpen());
        ASSERT_TRUE(serialPort2.IsOpen());

        unsigned char writeByte;
        unsigned char readByte;
        
        serialPort.WriteByte(writeByte);
        bytesRead = serialPort2.ReadByte(readByte, timeOutMilliseconds);
        ASSERT_EQ(readByte, writeByte);
        ASSERT_EQ(bytesRead, 1);
        usleep(10);
        
        serialPort.Close();
        serialPort2.Close();
        
        ASSERT_FALSE(serialPort.IsOpen());
        ASSERT_FALSE(serialPort2.IsOpen());
        
        serialPort.Open();
        serialPort2.Open();
        
        ASSERT_TRUE(serialPort.IsOpen());
        ASSERT_TRUE(serialPort2.IsOpen());

        serialPort.Write(writeString + '\n');
        bytesRead = serialPort2.ReadLine(readString, timeOutMilliseconds, '\n');
        ASSERT_EQ(readString, writeString + '\n');
        ASSERT_EQ(bytesRead, writeString.size() + 1);
        usleep(10);
        
        serialPort.Close();
        serialPort2.Close();
        
        ASSERT_FALSE(serialPort.IsOpen());
        ASSERT_FALSE(serialPort2.IsOpen());
    }

    void testSerialPortIsDataAvailableTest()
    {
        serialPort.Open();
        serialPort2.Open();

        ASSERT_TRUE(serialPort.IsOpen());
        ASSERT_TRUE(serialPort2.IsOpen());

        ASSERT_FALSE(serialPort.IsDataAvailable());

        unsigned char writeByte;
        unsigned char readByte;

        writeByte = 'A';

        serialPort.WriteByte(writeByte);
        usleep(1); // @TODO - This should be replaced to a call to tcdrain();
        ASSERT_TRUE(serialPort2.IsDataAvailable());

        bytesRead = serialPort2.ReadByte(readByte, 1);
        ASSERT_FALSE(serialPort2.IsDataAvailable());

        serialPort.Close();
        serialPort2.Close();
        
        ASSERT_FALSE(serialPort.IsOpen());
        ASSERT_FALSE(serialPort2.IsOpen());
    }

    void testSerialPortSetGetBaudRate()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        size_t maxBaudIndex = 17;
        
        #ifdef __linux__
            maxBaudIndex = 26;
            
            // // @TODO - Why don't higher baud rates work?
            // #if __MAX_BAUD > B2000000
            //     maxBaudIndex = 30;
            // #endif
        
        #endif

        for (size_t i = 0; i < maxBaudIndex; i++)
        {
            serialPort.SetBaudRate(serialPortBaudRate[i]);
            SerialPort::BaudRate baudRate = serialPort.GetBaudRate();
            ASSERT_EQ(baudRate, serialPortBaudRate[i]);
        }

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortSetGetCharSize()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        // @TODO - Why don't the smaller CharSize values work?
        for (size_t i = 2; i < 4; i++)
        {
            serialPort.SetCharSize(serialPortCharacterSize[i]);
            SerialPort::CharacterSize charSize = serialPort.GetCharSize();
            ASSERT_EQ(charSize, serialPortCharacterSize[i]);
        }

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortSetGetParity()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        for (size_t i = 0; i < 3; i++)
        {
            serialPort.SetParity(serialPortParity[i]);
            SerialPort::Parity parity = serialPort.GetParity();
            ASSERT_EQ(parity, serialPortParity[i]);
        }

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortSetGetFlowControl()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        // @TODO - FLOW_CONTROL_SOFT flow control aparrently is not valid.
        for (size_t i = 0; i < 2; i++)
        {
            serialPort.SetFlowControl(serialPortFlowControl[i]);
            SerialPort::FlowControl flowControl = serialPort.GetFlowControl();
            ASSERT_EQ(flowControl, serialPortFlowControl[i]);
        }

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortSetGetStopBits()
    {
        short numOfStopBits;

        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        serialPort.SetNumOfStopBits(SerialPort::STOP_BITS_1);
        numOfStopBits = serialPort.GetNumOfStopBits();
        ASSERT_EQ(numOfStopBits, SerialPort::STOP_BITS_1);

        serialPort.SetNumOfStopBits(SerialPort::STOP_BITS_2);
        numOfStopBits = serialPort.GetNumOfStopBits();
        ASSERT_EQ(numOfStopBits, SerialPort::STOP_BITS_2);

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortSetGetDTR()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        serialPort.SetDtr(true);
        bool dtrLine = serialPort.GetDtr();
        ASSERT_TRUE(dtrLine);

        serialPort.SetDtr(false);
        dtrLine = serialPort.GetDtr();
        ASSERT_FALSE(dtrLine);

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortSetGetRTS()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        serialPort.SetRts(true);
        bool rtsLine = serialPort.GetRts();
        ASSERT_TRUE(rtsLine);

        serialPort.SetRts(false);
        rtsLine = serialPort.GetRts();
        ASSERT_FALSE(rtsLine);

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortGetCTS()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        bool ctsLine = serialPort.GetCts();
        ASSERT_FALSE(ctsLine);

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    void testSerialPortGetDSR()
    {
        serialPort.Open();
        ASSERT_TRUE(serialPort.IsOpen());

        bool dsrLine = serialPort.GetDsr();
        ASSERT_FALSE(dsrLine);

        serialPort.Close();
        ASSERT_FALSE(serialPort.IsOpen());
    }

    SerialPort::BaudRate        serialPortBaudRate[30];
    SerialPort::CharacterSize   serialPortCharacterSize[4];
    SerialPort::Parity          serialPortParity[3];
    SerialPort::FlowControl     serialPortFlowControl[3];

    LibSerial::BaudRate         serialStreamBaudRate[30];
    LibSerial::CharSize         serialStreamCharSize[4];

    SerialStreamBuf::ParityEnum         serialStreamParity[3];
    SerialStreamBuf::FlowControlEnum    serialStreamFlowControl[3];

    SerialStream serialStream;
    SerialStream serialStream2;

    std::string readString;
    std::string writeString;

    char writeByte;
    char readByte;

    int bytesRead = 0;
};



//------------------------ Serial Stream Unit Tests -------------------------//

TEST_F(LibSerialTest, testSerialStreamOpenClose)
{
    SCOPED_TRACE("Serial Stream Open and Close Test");
    testSerialStreamOpenClose();
}

TEST_F(LibSerialTest, testSerialStreamReadWrite)
{
    SCOPED_TRACE("Serial Stream Read and Write Test");
    
    for (size_t i = 0; i < 1000; i++)
    {
        testSerialStreamReadWrite();
    }
}

TEST_F(LibSerialTest, testSerialStreamSetGetBaudRate)
{
    SCOPED_TRACE("Serial Stream Set and Get Baud Rate Test");
    testSerialStreamSetGetBaudRate();
}

TEST_F(LibSerialTest, testSerialStreamSetGetCharSize)
{
    SCOPED_TRACE("Serial Stream Set and Get Char Size Test");
    testSerialStreamSetGetCharSize();
}

TEST_F(LibSerialTest, testSerialStreamSetGetParity)
{
    SCOPED_TRACE("Serial Stream Set and Get Parity Type Test");
    testSerialStreamSetGetParity();
}

TEST_F(LibSerialTest, testSerialStreamSetGetFlowControl)
{
    SCOPED_TRACE("Serial Stream Set and Get Flow Control Test");
    testSerialStreamSetGetFlowControl();
}

TEST_F(LibSerialTest, testSerialStreamSetGetStopBits)
{
    SCOPED_TRACE("Serial Stream Set and Get Stop Bits Test");
    testSerialStreamSetGetStopBits();
}



//------------------------- Serial Port Unit Tests --------------------------//

TEST_F(LibSerialTest, testSerialPortOpenClose)
{
    SCOPED_TRACE("Serial Port Open and Close Test");

    for (size_t i = 0; i < 100; i++)
    {
        testSerialPortOpenClose();
    }
}

TEST_F(LibSerialTest, testSerialPortReadWrite)
{
    int timeOutMilliseconds = 25;

    SCOPED_TRACE("Serial Port Read and Write Test");

    for (size_t i = 0; i < 1000; i++)
    {
        testSerialPortReadWrite(timeOutMilliseconds);
    }
}

TEST_F(LibSerialTest, testSerialPortIsDataAvailableTest)
{
    SCOPED_TRACE("Serial Port Is Data Available Test");
    testSerialPortIsDataAvailableTest();
}

TEST_F(LibSerialTest, testSerialPortSetGetBaudRate)
{
    SCOPED_TRACE("Serial Port Set and Get Baud Rate Test");
    testSerialPortSetGetBaudRate();
}

TEST_F(LibSerialTest, testSerialPortSetGetCharSize)
{
    SCOPED_TRACE("Serial Port Set and Get Char Size Test");
    testSerialPortSetGetCharSize();
}

TEST_F(LibSerialTest, testSerialPortSetGetParity)
{
    SCOPED_TRACE("Serial Port Set and Get Parity Type Test");
    testSerialPortSetGetParity();
}

TEST_F(LibSerialTest, testSerialPortSetGetFlowControl)
{
    SCOPED_TRACE("Serial Port Set and Get Flow Control Test");
    testSerialPortSetGetFlowControl();
}

TEST_F(LibSerialTest, testSerialPortSetGetStopBits)
{
    SCOPED_TRACE("Serial Port Set and Get Stop Bits Test");
    testSerialPortSetGetStopBits();
}
TEST_F(LibSerialTest, testSerialPortSetGetDTR)
{
    SCOPED_TRACE("Serial Port Set and Get DTR Test");
    testSerialPortSetGetDTR();
}

TEST_F(LibSerialTest, testSerialPortSetGetRTS)
{
    SCOPED_TRACE("Serial Port Set and Get RTS Test");
    testSerialPortSetGetRTS();
}

TEST_F(LibSerialTest, testSerialPortGetCTS)
{
    SCOPED_TRACE("Serial Port Get CTS Test");
    testSerialPortGetCTS();
}

TEST_F(LibSerialTest, testSerialPortGetDSR)
{
    SCOPED_TRACE("Serial Port Get DSR Test");
    testSerialPortGetDSR();
}