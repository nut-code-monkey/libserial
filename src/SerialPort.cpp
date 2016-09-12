/******************************************************************************
 *   @file SerialPort.cpp                                                     *
 *   Copyright (C) 2004 by Manish Pagey                                       *
 *   crayzeewulf@users.sourceforge.net                                        *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                          *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                *
 *****************************************************************************/

#include "SerialPort.h"
#include "PosixSignalDispatcher.h"
#include "PosixSignalHandler.h"
#include <queue>
#include <map>
#include <cerrno>
#include <cassert>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <strings.h>
#include <cstring>
#include <cstdlib>
#include <sstream>

namespace
{
    //
    // Various error messages used in this file while throwing
    // exceptions.
    //
    const std::string ERR_MSG_PORT_NOT_OPEN     = "Serial port not open.";
    const std::string ERR_MSG_PORT_ALREADY_OPEN = "Serial port already open.";
    const std::string ERR_MSG_UNSUPPORTED_BAUD  = "Unsupported baud rate.";
    const std::string ERR_MSG_UNKNOWN_BAUD      = "Unknown baud rate.";
    const std::string ERR_MSG_INVALID_PARITY    = "Invalid parity setting.";
    const std::string ERR_MSG_INVALID_STOP_BITS = "Invalid number of stop bits.";
    const std::string ERR_MSG_INVALID_FLOW_CONTROL = "Invalid flow control.";

    /*
     * Return the difference between the two specified timeval values.
     * This method subtracts secondOperand from firstOperand and returns
     * the result as a timeval. The time represented by firstOperand must
     * be later than the time represented by secondOperand. Otherwise,
     * the result of this operator may be undefined.
     */
    const struct timeval
    operator-( const struct timeval& firstOperand,
               const struct timeval& secondOperand );
}

namespace LibSerial 
{
    class SerialPort::Implementation : public PosixSignalHandler
    {
    public:
        /**
         * Constructor.
         */
        Implementation( const std::string& serialPortName );

        /**
         * Destructor.
         */
        ~Implementation();

        /**
         * Open the serial port.
         */
        void Open();

        /**
         * Check if the serial port is currently open.
         */
        bool IsOpen() const;

        /**
         * Close the serial port.
         */
        void Close();

        /**
         * Set the baud rate of the serial port.
         */
        void
        SetBaudRate(SerialPort::BaudRate baudRate);

        /**
         * Get the current baud rate.
         */
        SerialPort::BaudRate GetBaudRate() const;

        /**
         * Set the character size.
         */
        void SetCharSize(SerialPort::CharacterSize charSize);

        /**
         * Get the current character size.
         */
        SerialPort::CharacterSize GetCharSize() const;

        void SetParity(const SerialPort::Parity parityType);

        SerialPort::Parity GetParity() const;

        void SetNumOfStopBits(SerialPort::StopBits numOfStopBits);

        SerialPort::StopBits GetNumOfStopBits() const;

        void SetFlowControl(SerialPort::FlowControl flowControl);

        SerialPort::FlowControl GetFlowControl() const;

        bool IsDataAvailable() const;

        int Read(SerialPort::DataBuffer& dataBuffer,
                 const unsigned int      numOfBytes,
                 const unsigned int      msTimeout);

        int ReadByte(unsigned char&     charBuffer, 
                     const unsigned int msTimeout = 0);

        int ReadLine(std::string&       dataString,
                     const unsigned int msTimeout = 0,
                     const char         lineTerminator = '\n');

        void WriteByte(unsigned char dataByte);

        void Write(const SerialPort::DataBuffer& dataBuffer);

        void Write(const unsigned char* dataBuffer,
                   const unsigned int   bufferSize );

        void SetDtr(const bool dtrState);

        bool GetDtr() const;

        void SetRts( const bool rtsState );

        bool GetRts() const;

        bool GetCts() const;

        bool GetDsr() const;

        /*
         * This method must be defined by all subclasses of
         * PosixSignalHandler.
         */
        void HandlePosixSignal(int signalNumber) override;
    private:
        /**
         * Name of the serial port. On POSIX systems this is the name of
         * the device file.
         */
        std::string mSerialPortName {};

        /**
         * Flag that indicates whether the serial port is currently open.
         */
        bool mIsOpen {false};

        /**
         * The file descriptor corresponding to the serial port.
         */
        int mFileDescriptor {-1};

        /**
         * Serial port settings are saved into this variable immediately
         * after the port is opened. These settings are restored when the
         * serial port is closed.
         */
        termios mOldPortSettings {};

        /**
         * Circular buffer used to store the received data. This is done
         * asynchronously and helps prevent overflow of the corresponding 
         * tty's input buffer.
         * 
         * :TODO: The size of this buffer is allowed to increase indefinitely. If 
         * data keeps arriving at the serial port and is never read then this 
         * buffer will continue occupying more and more memory. We need to put a 
         * cap on the size of this buffer. It might even be worth providing a 
         * method to set the size of this buffer.  
         */
        std::queue<unsigned char> mInputBuffer {};

        /*
         * In order to be thread save, a second queue is implemented to store
         * byte which are received while the queue is locked by the
         * ReadByte method. The content must be transfered to mInputBuffer
         * before further bytes are stored into it.
         */
        std::queue<unsigned char> mShadowInputBuffer {};

        /*
         * Mutex to control threaded access to mInputBuffer
         */
        pthread_mutex_t mQueueMutex {};

        /*
         * Indicates if unread bytes are available within the queue
         */
        volatile bool mIsQueueDataAvailable {false};

        /**
         * Set the specified modem control line to the specified value. 
         *
         * @param modemLine One of the following four values: TIOCM_DTR,
         * TIOCM_RTS, TIOCM_CTS, or TIOCM_DSR.
         *
         * @param lineState State of the modem line after successful
         * call to this method.
         */
        void
        SetModemControlLine( const int modemLine,
                             const bool lineState );

        /**
         * Get the current state of the specified modem control line.
         * 
         * @param modemLine One of the following four values: TIOCM_DTR,
         * TIOCM_RTS, TIOCM_CTS, or TIOCM_DSR.
         *
         * @return True if the specified line is currently set and false
         * otherwise.
         */
        bool
        GetModemControlLine( const int modemLine ) const;
    };

    SerialPort::SerialPort( const std::string& serialPortName ) :
        mImpl(new Implementation(serialPortName))
    {
        /* empty */
    }

    SerialPort::~SerialPort()
    throw()
    {
        /*
         * Close the serial port if it is open.
         */
        if ( this->IsOpen() )
        {
            this->Close();
        }
        return;
    }

    void
    SerialPort::Open( const BaudRate      baudRate,
                      const CharacterSize charSize,
                      const Parity        parityType,
                      const StopBits      stopBits,
                      const FlowControl   flowControl )
    {
        //
        // Open the serial port.
        mImpl->Open();
        //
        // Set the various parameters of the serial port if it is open.
        //
        this->SetBaudRate(baudRate);
        this->SetCharSize(charSize);
        this->SetParity(parityType);
        this->SetNumOfStopBits(stopBits);
        this->SetFlowControl(flowControl);

        //
        // All done.
        //
        return;
    }

    bool
    SerialPort::IsOpen() const
    {
        return mImpl->IsOpen();
    }

    void SerialPort::Close()
    {
        mImpl->Close();
        return;
    }

    void
    SerialPort::SetBaudRate( const BaudRate baudRate )
    {
        mImpl->SetBaudRate( baudRate );
        return;
    }

    SerialPort::BaudRate
    SerialPort::GetBaudRate() const
    {
        return mImpl->GetBaudRate();
    }


    void
    SerialPort::SetCharSize( const CharacterSize charSize )
    {
        mImpl->SetCharSize(charSize);
    }

    SerialPort::CharacterSize
    SerialPort::GetCharSize() const
    {
        return mImpl->GetCharSize();
    }

    void
    SerialPort::SetParity( const Parity parityType )
    {
        mImpl->SetParity( parityType );
        return;
    }

    SerialPort::Parity
    SerialPort::GetParity() const
    {
        return mImpl->GetParity();
    }

    void
    SerialPort::SetNumOfStopBits( const StopBits numOfStopBits )
    {
        mImpl->SetNumOfStopBits(numOfStopBits);
        return;
    }

    SerialPort::StopBits
    SerialPort::GetNumOfStopBits() const
    {
        return mImpl->GetNumOfStopBits();
    }


    void
    SerialPort::SetFlowControl( const FlowControl   flowControl )
    {
        mImpl->SetFlowControl( flowControl );
        return;
    }

    SerialPort::FlowControl
    SerialPort::GetFlowControl() const
    {
        return mImpl->GetFlowControl();
    }

    bool
    SerialPort::IsDataAvailable() const
    {
        return mImpl->IsDataAvailable();
    }



    int
    SerialPort::Read(SerialPort::DataBuffer& dataBuffer,
                     const unsigned int      numOfBytes,
                     const unsigned int      msTimeout)
    {
        return mImpl->Read( dataBuffer,
                            numOfBytes,
                            msTimeout );
    }

    int
    SerialPort::ReadByte(unsigned char&     charBuffer,
                         const unsigned int msTimeout)
    {
        return mImpl->ReadByte(charBuffer,
                               msTimeout);
    }

    int
    SerialPort::ReadLine(std::string&       dataString,
                         const unsigned int msTimeout,
                         const char         lineTerminator)
    {
        return mImpl->ReadLine( dataString,
                                msTimeout,
                                lineTerminator );
    }

    void
    SerialPort::WriteByte(const unsigned char dataByte)
    {
        mImpl->WriteByte( dataByte );
        return;
    }

    void
    SerialPort::Write(const DataBuffer& dataBuffer)
    {
        mImpl->Write( dataBuffer );
        return;
    }

    void
    SerialPort::Write(const std::string& dataString)
    {
        mImpl->Write( reinterpret_cast<const unsigned char*>(dataString.c_str()),
                                dataString.length() );
        return;
    }

    void
    SerialPort::SetDtr( const bool dtrState )
    {
        mImpl->SetDtr( dtrState );
        return;
    }

    bool
    SerialPort::GetDtr() const 
    {
        return mImpl->GetDtr();
    }

    void
    SerialPort::SetRts( const bool rtsState )
    {
        mImpl->SetRts( rtsState );
        return;
    }

    bool
    SerialPort::GetRts() const 
    {
        return mImpl->GetRts();
    }


    bool
    SerialPort::GetCts() const 
    {
        return mImpl->GetCts();
    }

    bool
    SerialPort::GetDsr() const 
    {
        return mImpl->GetDsr();
    }

    /* ------------------------------------------------------------ */
    inline
    SerialPort::Implementation::Implementation(const std::string& serialPortName) :
        mSerialPortName(serialPortName)
    {
        //Initializing the mutex
        if (pthread_mutex_init(&mQueueMutex, NULL) != 0) {
            std::ostringstream err_msg; 
            err_msg << "SerialPort.cpp: Could not initialize mutex!";
            throw std::runtime_error(err_msg.str());
        }
    }

    inline
    SerialPort::Implementation::~Implementation()
    {
        //
        // Close the serial port if it is open.
        //
        if ( this->IsOpen() )
        {
            this->Close();
        }
        return;
    }

    inline
    void
    SerialPort::Implementation::Open()
    {
        /*
         * Throw an exception if the port is already open.
         */
        if ( this->IsOpen() )
        {
            throw SerialPort::AlreadyOpen( ERR_MSG_PORT_ALREADY_OPEN );
        }
        /*
         * Try to open the serial port and throw an exception if we are
         * not able to open it.
         *
         * :FIXME: Exception thrown by this method after opening the
         * serial port might leave the port open even though mIsOpen
         * is false. We need to close the port before throwing an
         * exception or close it next time this method is called before
         * calling open() again.
         */
        mFileDescriptor = open( mSerialPortName.c_str(),
                                O_RDWR | O_NOCTTY | O_NONBLOCK );
        if ( mFileDescriptor < 0 )
        {
            throw SerialPort::OpenFailed( strerror(errno) ) ;
        }


        PosixSignalDispatcher& signal_dispatcher = PosixSignalDispatcher::Instance();
        signal_dispatcher.AttachHandler( SIGIO,
                                         *this );

        /*
         * Direct all SIGIO and SIGURG signals for the port to the current
         * process.
         */
        if ( fcntl( mFileDescriptor,
                    F_SETOWN,
                    getpid() ) < 0 )
        {
            throw SerialPort::OpenFailed( strerror(errno) );
        }

        /*
         * Enable asynchronous I/O with the serial port.
         */
        if ( fcntl( mFileDescriptor,
                    F_SETFL,
                    FASYNC ) < 0 )
        {
            throw SerialPort::OpenFailed( strerror(errno) );
        }

        /*
         * Save the current settings of the serial port so they can be
         * restored when the serial port is closed.
         */
        if ( tcgetattr( mFileDescriptor,
                        &mOldPortSettings ) < 0 )
        {
            throw SerialPort::OpenFailed( strerror(errno) );
        }

        //
        // Start assembling the new port settings.
        //
        termios port_settings;
        bzero( &port_settings,
               sizeof( port_settings ) );

        //
        // Enable the receiver (CREAD) and ignore modem control lines
        // (CLOCAL).
        //
        port_settings.c_cflag |= CREAD | CLOCAL;

        //
        // Set the VMIN and VTIME parameters to zero by default. VMIN is
        // the minimum number of characters for non-canonical read and
        // VTIME is the timeout in deciseconds for non-canonical
        // read. Setting both of these parameters to zero implies that a
        // read will return immediately only giving the currently
        // available characters.
        //
        port_settings.c_cc[ VMIN  ] = 0;
        port_settings.c_cc[ VTIME ] = 0;
        /*
         * Flush the input buffer associated with the port.
         */
        if ( tcflush( mFileDescriptor,
                      TCIFLUSH ) < 0 )
        {
            throw SerialPort::OpenFailed( strerror(errno) );
        }
        /*
         * Write the new settings to the port.
         */
        if ( tcsetattr( mFileDescriptor,
                        TCSANOW,
                        &port_settings ) < 0 )
        {
            throw SerialPort::OpenFailed( strerror(errno) );
        }

        /*
         * The serial port is open at this point.
         */
        mIsOpen = true;

        //Reset flag
        mIsQueueDataAvailable = false;

        return;
    }

    inline
    bool
    SerialPort::Implementation::IsOpen() const
    {
        return mIsOpen;
    }

    inline
    void
    SerialPort::Implementation::Close()
    {
        //
        // Throw an exception if the serial port is not open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        PosixSignalDispatcher& signal_dispatcher = PosixSignalDispatcher::Instance();
        signal_dispatcher.DetachHandler( SIGIO,
                                         *this );
        //
        // Restore the old settings of the port.
        //
        tcsetattr( mFileDescriptor,
                   TCSANOW,
                   &mOldPortSettings );
        //
        // Close the serial port file descriptor.
        //
        close(mFileDescriptor);
        //
        // The port is not open anymore.
        //
        mIsOpen = false;
        //
        return;
    }

    inline
    void
    SerialPort::Implementation::SetBaudRate( const SerialPort::BaudRate baudRate )
    {
        //
        // Throw an exception if the serial port is not open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current settings of the serial port.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Set the baud rate for both input and output.
        //
        if ( ( cfsetispeed( &port_settings,
                            baudRate ) < 0 ) ||
             ( cfsetospeed( &port_settings,
                            baudRate ) < 0 ) )
        {
            //
            // If any of the settings fail, we abandon this method.
            //
            throw SerialPort::UnsupportedBaudRate( ERR_MSG_UNSUPPORTED_BAUD );
        }
        //
        // Set the new attributes of the serial port.
        //
        if ( tcsetattr( mFileDescriptor,
                        TCSANOW,
                        &port_settings ) < 0 )
        {
            throw SerialPort::UnsupportedBaudRate( strerror(errno) );
        }
        return;
    }

    inline
    SerialPort::BaudRate
    SerialPort::Implementation::GetBaudRate() const
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Read the current serial port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Obtain the input baud rate from the current settings.
        //
        return SerialPort::BaudRate(cfgetispeed( &port_settings ));
    }

    inline
    void
    SerialPort::Implementation::SetCharSize( const SerialPort::CharacterSize charSize )
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current settings of the serial port.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Set the character size.
        //
        port_settings.c_cflag &= ~CSIZE;
        port_settings.c_cflag |= charSize;
        //
        // Apply the modified settings.
        //
        if ( tcsetattr( mFileDescriptor,
                        TCSANOW,
                        &port_settings ) < 0 )
        {
            throw std::invalid_argument( strerror(errno) );
        }
        return;
    }

    inline
    SerialPort::CharacterSize
    SerialPort::Implementation::GetCharSize() const
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Read the character size from the setttings.
        //
        return SerialPort::CharacterSize( port_settings.c_cflag & CSIZE );
    }

    inline
    void
    SerialPort::Implementation::SetParity( const SerialPort::Parity parityType )
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Set the parity type depending on the specified parameter.
        //
        switch( parityType )
        {
        case SerialPort::PARITY_EVEN:
            port_settings.c_cflag |= PARENB;
            port_settings.c_cflag &= ~PARODD;
            port_settings.c_iflag |= INPCK;
            break;
        case SerialPort::PARITY_ODD:
            port_settings.c_cflag |= ( PARENB | PARODD );
            port_settings.c_iflag |= INPCK;
            break;
        case SerialPort::PARITY_NONE:
            port_settings.c_cflag &= ~(PARENB);
            port_settings.c_iflag |= IGNPAR;
            break;
        default:
            throw std::invalid_argument( ERR_MSG_INVALID_PARITY );
            break;
        }
        //
        // Apply the modified port settings.
        //
        if ( tcsetattr( mFileDescriptor,
                        TCSANOW,
                        &port_settings ) < 0 )
        {
            throw std::invalid_argument( strerror(errno) );
        }
        return;
    }

    inline
    SerialPort::Parity
    SerialPort::Implementation::GetParity() const
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Get the parity type from the current settings.
        //
        if ( port_settings.c_cflag & PARENB )
        {
            //
            // Parity is enabled. Lets check if it is odd or even.
            //
            if ( port_settings.c_cflag & PARODD )
            {
                return SerialPort::PARITY_ODD;
            }
            else
            {
                return SerialPort::PARITY_EVEN;
            }
        }
        //
        // Parity is disabled.
        //
        return SerialPort::PARITY_NONE;
    }

    inline
    void
    SerialPort::Implementation::SetNumOfStopBits( const SerialPort::StopBits numOfStopBits )
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Set the number of stop bits.
        //
        switch( numOfStopBits )
        {
        case SerialPort::STOP_BITS_1:
            port_settings.c_cflag &= ~(CSTOPB);
            break;
        case SerialPort::STOP_BITS_2:
            port_settings.c_cflag |= CSTOPB;
            break;
        default:
            throw std::invalid_argument( ERR_MSG_INVALID_STOP_BITS );
            break;
        }
        //
        // Apply the modified settings.
        //
        if ( tcsetattr( mFileDescriptor,
                        TCSANOW,
                        &port_settings ) < 0 )
        {
            throw std::invalid_argument( strerror(errno) );
        }
        return;
    }

    inline
    SerialPort::StopBits
    SerialPort::Implementation::GetNumOfStopBits() const
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // If CSTOPB is set then we are using two stop bits, otherwise we
        // are using 1 stop bit.
        //
        if ( port_settings.c_cflag & CSTOPB )
        {
            return SerialPort::STOP_BITS_2;
        }
        return SerialPort::STOP_BITS_1;
    }

    inline
    void
    SerialPort::Implementation::SetFlowControl( const SerialPort::FlowControl   flowControl )
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // Set the flow control.
        //
        switch( flowControl )
        {
        case SerialPort::FLOW_CONTROL_HARD:
            port_settings.c_cflag |= CRTSCTS;
            break;
        case SerialPort::FLOW_CONTROL_NONE:
            port_settings.c_cflag &= ~(CRTSCTS);
            break;
        default:
            throw std::invalid_argument( ERR_MSG_INVALID_FLOW_CONTROL );
            break;
        }
        //
        // Apply the modified settings.
        //
        if ( tcsetattr( mFileDescriptor,
                        TCSANOW,
                        &port_settings ) < 0 )
        {
            throw std::invalid_argument( strerror(errno) );
        }
        return;
    }

    inline
    SerialPort::FlowControl
    SerialPort::Implementation::GetFlowControl() const
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Get the current port settings.
        //
        termios port_settings;
        if ( tcgetattr( mFileDescriptor,
                        &port_settings ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // If CRTSCTS is set then we are using hardware flow
        // control. Otherwise, we are not using any flow control.
        //
        if ( port_settings.c_cflag & CRTSCTS )
        {
            return SerialPort::FLOW_CONTROL_HARD;
        }
        return SerialPort::FLOW_CONTROL_NONE;
    }

    inline
    bool
    SerialPort::Implementation::IsDataAvailable() const
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Check if any data is available in the input buffer.
        //
        //return ( mInputBuffer.size() > 0 ? true : false );
        //Here comes an (almost) thread safe alternative
        return mIsQueueDataAvailable;
    }

    inline
    int
    SerialPort::Implementation::Read(SerialPort::DataBuffer& dataBuffer,
                                     const unsigned int      numOfBytes,
                                     const unsigned int      msTimeout)
    {
        // Make sure that the serial port is open.
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }

        // Empty the data buffer.
        dataBuffer.resize(0);
        unsigned char nextChar = 0;
        size_t bytesRead = 0;

        if ( 0 == numOfBytes )
        {
            // Read all available data if numOfBytes is zero.
            while( this->IsDataAvailable() )
            {
                bytesRead += ReadByte( nextChar,
                                       msTimeout);
                dataBuffer.push_back( nextChar );
            }
        }
        else
        {
            // Reserve enough space in the buffer to store the incoming data.
            dataBuffer.reserve( numOfBytes );

            for(unsigned int i=0; i<numOfBytes; ++i)
            {
                bytesRead += ReadByte( nextChar,
                                       msTimeout);
                dataBuffer.push_back( nextChar );
            }
        }
        
        return dataBuffer.size();
    }

    inline
    int
    SerialPort::Implementation::ReadByte(unsigned char&     charBuffer,
                                         const unsigned int msTimeout)
    {
        pthread_mutex_lock(&mQueueMutex);
        int queueSize = mInputBuffer.size();
        pthread_mutex_unlock(&mQueueMutex);

        // Make sure that the serial port is open.
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }

        // Get the current time. Throw an exception if we are unable
        // to read the current time.
        struct timeval entry_time;
        if ( gettimeofday( &entry_time,
                           NULL ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }

        // Wait for data to be available.
        const int MICROSECONDS_PER_MS  = 1000;
        const int MILLISECONDS_PER_SEC = 1000;

        while(queueSize == 0 )
        {
            // Read the current time.
            struct timeval curr_time;
            if ( gettimeofday( &curr_time,
                               NULL ) < 0 )
            {
                throw std::runtime_error( strerror(errno) );
            }

            // Obtain the elapsed time.
            struct timeval elapsed_time = curr_time - entry_time;

            // Calculate the elapsed number of milliseconds.
            int elapsed_ms = ( elapsed_time.tv_sec  * MILLISECONDS_PER_SEC +
                               elapsed_time.tv_usec / MICROSECONDS_PER_MS );

            // If more than msTimeout milliseconds have elapsed while
            // waiting for data, then we throw a ReadTimeout exception.
            if ( ( msTimeout > 0 ) &&
                 ( elapsed_ms > (int)msTimeout ) )
            {
                throw SerialPort::ReadTimeout();
            }

            // Wait for 1ms (1000us) for data to arrive.
            usleep( MICROSECONDS_PER_MS );
            pthread_mutex_lock(&mQueueMutex);
            queueSize = mInputBuffer.size();
            pthread_mutex_unlock(&mQueueMutex);
        }

        // Return the first byte and remove it from the queue.
        pthread_mutex_lock(&mQueueMutex);
        charBuffer = mInputBuffer.front();
        mInputBuffer.pop();

        //Updating flag if queue is empty by now
        if ( mInputBuffer.size() == 0)
        {
            mIsQueueDataAvailable = false;
        }
        pthread_mutex_unlock( &mQueueMutex );

        return 1;
    }

    inline
    int
    SerialPort::Implementation::ReadLine(std::string&       dataString,
                                         const unsigned int msTimeout,
                                         const char         lineTerminator)
    {
        // Clear the data string for new data to be read into.
        dataString.clear();

        unsigned char nextChar = 0;
        size_t bytesRead = 0;

        // Get the current time. Throw an exception if we are unable
        // to read the current time.
        struct timeval entry_time;
        if ( gettimeofday( &entry_time,
                           NULL ) < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }

        const int MICROSECONDS_PER_MS  = 1000;
        const int MILLISECONDS_PER_SEC = 1000;

        do
        {
            // Read the current time.
            struct timeval curr_time;
            if ( gettimeofday( &curr_time,
                               NULL ) < 0 )
            {
                throw std::runtime_error( strerror(errno) );
            }

            // Obtain the elapsed time.
            struct timeval elapsed_time = curr_time - entry_time;

            // Calculate the elapsed number of milliseconds.
            int elapsed_ms = ( elapsed_time.tv_sec  * MILLISECONDS_PER_SEC +
                               elapsed_time.tv_usec / MICROSECONDS_PER_MS );

            // If more than msTimeout milliseconds have elapsed while
            // waiting for data, then we throw a ReadTimeout exception.
            if ( ( msTimeout > 0 ) &&
                 ( elapsed_ms > (int)msTimeout ) )
            {
                throw SerialPort::ReadTimeout();
            }

            bytesRead += this->ReadByte(nextChar,
                                        msTimeout);
            dataString += nextChar;
        } while ( nextChar != lineTerminator );
        
        return bytesRead;
    }

    inline
    void
    SerialPort::Implementation::WriteByte( const unsigned char dataByte )
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Write the byte to the serial port.
        //
        this->Write( &dataByte,
                     1 );
        return;
    }

    inline
    void
    SerialPort::Implementation::Write(const SerialPort::DataBuffer& dataBuffer)
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Nothing needs to be done if there is no data in the buffer.
        //
        if ( 0 == dataBuffer.size() )
        {
            return;
        }
        //
        // Allocate memory for storing the contents of the
        // dataBuffer. This allows us to write all the data using a single
        // call to write() instead of writing one byte at a time.
        //
        unsigned char* local_buffer = new unsigned char[dataBuffer.size()];
        if ( 0 == local_buffer )
        {
            throw std::runtime_error( std::string(__FUNCTION__) +
                                      ": Cannot allocate memory while writing"
                                      "data to the serial port." );
        }
        //
        // Copy the data into local_buffer.
        //
        std::copy( dataBuffer.begin(),
                   dataBuffer.end(),
                   local_buffer );
        //
        // Write data to the serial port.
        //
        try
        {
            this->Write( local_buffer,
                         dataBuffer.size() );
        }
        catch( ... )
        {
            //
            // Free the allocated memory.
            //
            delete [] local_buffer;
            throw;
        }
        //
        // Free the allocated memory.
        //
        delete [] local_buffer;
        return;
    }

    inline
    void
    SerialPort::Implementation::Write( const unsigned char* dataBuffer,
                                       const unsigned int   bufferSize )
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Write the data to the serial port. Keep retrying if EAGAIN
        // error is received.
        //
        int num_of_bytes_written = -1;
        do
        {
            num_of_bytes_written = write( mFileDescriptor,
                                          dataBuffer,
                                          bufferSize );
        }
        while ( ( num_of_bytes_written < 0 ) &&
                ( EAGAIN == errno ) );
        //
        if ( num_of_bytes_written < 0 )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // :FIXME: What happens if num_of_bytes_written < bufferSize ?
        //
        return;
    }

    inline
    void
    SerialPort::Implementation::SetDtr( const bool dtrState )
    {
        this->SetModemControlLine( TIOCM_DTR, 
                                   dtrState );
        return;
    }

    inline
    bool
    SerialPort::Implementation::GetDtr() const
    {
        return this->GetModemControlLine( TIOCM_DTR );
    }    

    inline
    void
    SerialPort::Implementation::SetRts( const bool rtsState )
    {
        this->SetModemControlLine( TIOCM_RTS, 
                                   rtsState );
        return;
    }

    inline
    bool
    SerialPort::Implementation::GetRts() const
    {
        return this->GetModemControlLine( TIOCM_RTS );
    }    


    inline
    bool
    SerialPort::Implementation::GetCts() const
    {
        return this->GetModemControlLine( TIOCM_CTS );
    }    


    inline
    bool
    SerialPort::Implementation::GetDsr() const
    {
        return this->GetModemControlLine( TIOCM_DSR );
    }    

    inline
    void
    SerialPort::Implementation::HandlePosixSignal( int signalNumber )
    {
        //
        // We only want to deal with SIGIO signals here.
        //
        if ( SIGIO != signalNumber )
        {
            return;
        }
        //
        // Check if any data is available at the specified file
        // descriptor.
        //
        int num_of_bytes_available = 0;
        if ( ioctl( mFileDescriptor,
                    FIONREAD,
                    &num_of_bytes_available ) < 0 )
        {
            /*
             * Ignore any errors and return immediately.
             */
            return;
        }

        //Try to get the mutex
        if (pthread_mutex_trylock(&mQueueMutex) == 0){
            // First of all, any pending data within the mShadowInputBuffer
            // must be transfered into the regular buffer.
            while(!mShadowInputBuffer.empty())
            {
                //Transfering to actual input buffer
                mInputBuffer.push(mShadowInputBuffer.front());
                //Removing from shadow buffer
                mShadowInputBuffer.pop();
            }

            //
            // If data is available, read all available data and shove
            // it into the corresponding input buffer.
            //
            for(int i=0; i<num_of_bytes_available; ++i)
            {
                unsigned char next_byte;
                if ( read( mFileDescriptor,
                           &next_byte,
                           1 ) > 0 )
                {
                    mInputBuffer.push( next_byte );
                }
                else
                {
                    //Updating flag
                    pthread_mutex_unlock(&mQueueMutex);
                    break;
                }
            }

            //Updating flag
            mIsQueueDataAvailable = true;

            //Release the mutex
            pthread_mutex_unlock(&mQueueMutex);
        } else {
            //Mutex is locked - using shadowQueue to avoid a deadlock!
            //
            // If data is available, read all available data and shove
            // it temporarily into the shadow buffer.
            //
            for(int i=0; i<num_of_bytes_available; ++i)
            {
                unsigned char next_byte;
                if ( read( mFileDescriptor,
                           &next_byte,
                           1 ) > 0 )
                {
                    mShadowInputBuffer.push( next_byte );
                }
                else
                {
                    break;
                }
            }
        }
        return;
    }

    inline
    void
    SerialPort::Implementation::SetModemControlLine( const int  modemLine,
                                                     const bool lineState )
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // :TODO: Check to make sure that modemLine is a valid value.
        // 
        // Set or unset the specified bit according to the value of
        // lineState.
        //
        int ioctl_result = -1;
        if ( true == lineState )
        {
            int set_line_mask = modemLine;
            ioctl_result = ioctl( mFileDescriptor, 
                                  TIOCMBIS,
                                  &set_line_mask );
        }
        else
        {
            int reset_line_mask = modemLine;
            ioctl_result = ioctl( mFileDescriptor, 
                                  TIOCMBIC,
                                  &reset_line_mask );
        }
        //
        // Check for errors. 
        //
        if ( -1 == ioctl_result )
        {
            throw std::runtime_error( strerror(errno) );
        }
        return;
    }

    inline
    bool
    SerialPort::Implementation::GetModemControlLine( const int modemLine ) const
    {
        //
        // Make sure that the serial port is open.
        //
        if ( ! this->IsOpen() )
        {
            throw SerialPort::NotOpen( ERR_MSG_PORT_NOT_OPEN );
        }
        //
        // Use an ioctl() call to get the state of the line.
        //
        int serial_port_state = 0;
        if ( -1 == ioctl( mFileDescriptor,
                          TIOCMGET,
                          &serial_port_state ) )
        {
            throw std::runtime_error( strerror(errno) );
        }
        //
        // :TODO: Verify that modemLine is a valid value.
        //
        return ( serial_port_state & modemLine );
    }
}

namespace
{
    const struct timeval
    operator-( const struct timeval& firstOperand,
               const struct timeval& secondOperand )
    {
        /*
         * This implementation may result in undefined behavior if the
         * platform uses unsigned values for storing tv_sec and tv_usec
         * members of struct timeval.
         */
        //
        // Number of microseconds in a second.
        //
        const int MICROSECONDS_PER_SECOND = 1000000;
        struct timeval result;
        //
        // Take the difference of individual members of the two operands.
        //
        result.tv_sec  = firstOperand.tv_sec - secondOperand.tv_sec;
        result.tv_usec = firstOperand.tv_usec - secondOperand.tv_usec;
        //
        // If abs(result.tv_usec) is larger than MICROSECONDS_PER_SECOND,
        // then increment/decrement result.tv_sec accordingly.
        //
        if ( std::abs( result.tv_usec ) > MICROSECONDS_PER_SECOND )
        {
            int num_of_seconds = (result.tv_usec / MICROSECONDS_PER_SECOND );
            result.tv_sec  += num_of_seconds;
            result.tv_usec -= ( MICROSECONDS_PER_SECOND * num_of_seconds );
        }
        return result;
    }
}
