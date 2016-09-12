/******************************************************************************
 *   @file SerialStreamBuf.cc                                                 *
 *   @copyright                                                               *
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

#include "SerialStreamBuf.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <fstream>
#include <limits.h>
#include <strings.h>


using namespace std;

namespace LibSerial
{
    class SerialStreamBuf::Implementation
    {
    public:
        Implementation();

        ~Implementation() { /* empty */ }

        BaudRate
        SetBaudRate(const BaudRate baudRate);

        BaudRate
        GetBaudRate() const;

        CharSize
        SetCharSize(const CharSize charSize);

        CharSize
        GetCharSize() const;

        short SetNumOfStopBits(short numOfStopBits);
        short GetNumOfStopBits() const;

        SerialStreamBuf::ParityEnum 
        SetParity(const SerialStreamBuf::ParityEnum parityType);

        SerialStreamBuf::ParityEnum
        GetParity() const;

        SerialStreamBuf::FlowControlEnum 
        SetFlowControl(const SerialStreamBuf::FlowControlEnum flowControlType);

        SerialStreamBuf::FlowControlEnum 
        GetFlowControl() const;

        short SetVMin(short vtime);
        short GetVMin() const;

        short SetVTime(short vtime);
        short GetVTime() const;

        streamsize
        xsgetn(char_type *s, streamsize n);

        streamsize 
        showmanyc();

        streambuf::int_type
        underflow();

        streambuf::int_type
        pbackfail(int_type c);

        streamsize
        xsputn(const char_type *s, streamsize n);

        streambuf::int_type
        overflow(int_type c);

        /** 
         * We use unbuffered I/O for the serial port. However, we
         * need to provide the putback of atleast one
         * character. This character contains the putback character.
         */
        char mPutbackChar;

        /** 
         * True if a putback value is available in mPutbackChar. 
         */
        bool mPutbackAvailable;

        /** 
         * The file descriptor associated with the serial port. 
         */
        int mFileDescriptor;

        /** 
         * This routine is called by open() in order to
         * initialize some parameters of the serial port and
         * setting its parameters to default values.
         * 
         * @return -1 on failure and some other value on success. 
         */
        int InitializeSerialPort();

        int SetParametersToDefault();
    };

    SerialStreamBuf::SerialStreamBuf()
        : mImpl(new Implementation)
    {
        setbuf(0, 0);
        return;
    }


    SerialStreamBuf::~SerialStreamBuf() 
    {
        if (this->is_open()) 
        {
            this->close();
        }
        return;
    }

    bool
    SerialStreamBuf::is_open() const 
    {
        return (-1 != mImpl->mFileDescriptor);
    }


    std::streambuf* 
    SerialStreamBuf::setbuf(char_type *, std::streamsize) 
    {
        return std::streambuf::setbuf(0, 0);
    }


    SerialStreamBuf*
    SerialStreamBuf::close() 
    {

        // Return a null pointer if the serial port is not currently open. 
        if (this->is_open() == false) 
        {
            return 0;
        }

        // Otherwise, close the serial port and set the file descriptor
        // to an invalid value.
        if (-1 == ::close(mImpl->mFileDescriptor)) 
        {
            // If the close failed then return a null pointer. 
            return 0;
        } 
        else
        {
            // Set the file descriptor to an invalid value, -1. 
            mImpl->mFileDescriptor = -1;

            // On success, return "this" as required by the C++ standard.
            return this;
        }
    }

    std::streambuf::int_type
    SerialStreamBuf::uflow() 
    {
        int_type next_ch = underflow();
        mImpl->mPutbackAvailable = false;
        return next_ch;
    }

    SerialStreamBuf*
    SerialStreamBuf::open(const string filename,
                          ios_base::openmode mode) 
    {
        // If the buffer is alreay open then we should not allow a call to
        // another open().
        if ( is_open() != false ) 
        {
            return 0;
        }

        // We only allow three different combinations of ios_base::openmode
        // so we can use a switch here to construct the flags to be used
        // with the open() system call.
        int flags;
        
        if ( mode == (ios_base::in|ios_base::out) )
        {
            flags = O_RDWR;
        } 
        else if ( mode == ios_base::in )
        {
            flags = O_RDONLY;
        } 
        else if ( mode == ios_base::out )
        {
            flags = O_WRONLY;
        } 
        else 
        {
            return 0;
        }

        /**
            switch( mode )
            {
            case ios_base::in:
                flags = O_RDONLY;
                break;
            case ios_base::out:
                flags = O_WRONLY;
                break;
            case (ios_base::in|ios_base::out):
                flags = O_RDWR;
                break;
            default:
                return 0;
                break;
            }
        */

        // Since we are dealing with the serial port we need to use the
        // O_NOCTTY option.
        flags |= O_NOCTTY;

        // Try to open the serial port. 
        mImpl->mFileDescriptor = ::open(filename.c_str(), flags);
        
        if ( -1 == mImpl->mFileDescriptor ) 
        {
            return 0;
        }

        // Initialize the serial port. 

        if ( -1 == mImpl->InitializeSerialPort() )
        {
            return 0;
        }
        return this;
    }

    int
    SerialStreamBuf::SetParametersToDefault()
    {
        return mImpl->SetParametersToDefault();
    }

    BaudRate
    SerialStreamBuf::SetBaudRate(const BaudRate baud_rate)
    {
        return mImpl->SetBaudRate( baud_rate );
    }

    BaudRate
    SerialStreamBuf::GetBaudRate() const 
    {
        return mImpl->GetBaudRate();
    }


    CharSize
    SerialStreamBuf::SetCharSize(const CharSize char_size)
    {
        return mImpl->SetCharSize( char_size );
    }


    CharSize
    SerialStreamBuf::GetCharSize() const 
    {
        return mImpl->GetCharSize();
    }


    short
    SerialStreamBuf::SetNumOfStopBits(const short stop_bits)
    {
        return mImpl->SetNumOfStopBits( stop_bits );
    }


    short 
    SerialStreamBuf::GetNumOfStopBits() const 
    {
        return mImpl->GetNumOfStopBits();
    }


    SerialStreamBuf::ParityEnum
    SerialStreamBuf::SetParity(const ParityEnum parity)
    {
        return mImpl->SetParity( parity );
    }

    SerialStreamBuf::ParityEnum
    SerialStreamBuf::GetParity() const 
    {
        return mImpl->GetParity();
    }

    SerialStreamBuf::FlowControlEnum
    SerialStreamBuf::SetFlowControl(const FlowControlEnum flow_c)
    {
        return mImpl->SetFlowControl( flow_c );
    }

    SerialStreamBuf::FlowControlEnum
    SerialStreamBuf::GetFlowControl() const
    {
        return mImpl->GetFlowControl();
    }


    short 
    SerialStreamBuf::SetVMin(const short vmin)
    {
        return mImpl->SetVMin( vmin );
    }


    short 
    SerialStreamBuf::GetVMin() const
    {
        return mImpl->GetVMin();
    }

    short 
    SerialStreamBuf::SetVTime(const short vtime)
    {
        return mImpl->SetVTime( vtime );
    }

    short 
    SerialStreamBuf::GetVTime() const 
    {
        return mImpl->GetVTime();
    }

    streamsize
    SerialStreamBuf::xsgetn(char_type *s, streamsize n)
    {
        return mImpl->xsgetn( s, n );
    }

    streamsize
    SerialStreamBuf::xsputn(const char_type *s, streamsize n)
    {
        return mImpl->xsputn( s, n );
    }

    std::streamsize
    SerialStreamBuf::showmanyc()
    {
        return mImpl->showmanyc();
    }

    streambuf::int_type
    SerialStreamBuf::overflow(const int_type c)
    {
        return mImpl->overflow(c);
    }

    streambuf::int_type
    SerialStreamBuf::underflow()
    {
        return mImpl->underflow();
    }


    streambuf::int_type
    SerialStreamBuf::pbackfail(const int_type c)
    {
        return mImpl->pbackfail(c);
    }

    inline
    SerialStreamBuf::Implementation::Implementation()
        : mPutbackChar(0)
        , mPutbackAvailable(false)
        , mFileDescriptor(-1)
    {
        /* empty */
    }

    inline
    int 
    SerialStreamBuf::Implementation::SetParametersToDefault()
    {
        if ( -1 == mFileDescriptor )
        {
            return -1;
        }

        // Set all values (also the ones, which are not covered by the
        // parameter-functions of this library).
        struct termios tio;
        if ( -1 == tcgetattr(mFileDescriptor, &tio) )
        {
            return -1;
        }

        tio.c_iflag = IGNBRK;
        tio.c_oflag = 0;
        tio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
        tio.c_lflag = 0;

        // :TRICKY:
        // termios.c_line is not a standard element of the termios structure (as 
        // per the Single Unix Specification 2. This is only present under Linux.
    #ifdef __linux__
        tio.c_line = '\0';
    #endif
        bzero( &tio.c_cc, sizeof(tio.c_cc) );
        tio.c_cc[VTIME] = 0;
        tio.c_cc[VMIN]  = 1;
        
        if ( -1 == tcsetattr(mFileDescriptor,TCSANOW,&tio) )
        {
            return -1;
        }

        // Baud rate
        if ( BaudRate::BAUD_INVALID == 
            SetBaudRate(SerialStreamBuf::DEFAULT_BAUD) ) 
        {
            return -1;
        };

        // Character size. 
        if ( CharSize::CHAR_SIZE_INVALID == SetCharSize(DEFAULT_CHAR_SIZE) )
        {
            return -1;
        }

        // Number of stop bits. 
        if ( -1 == SetNumOfStopBits(DEFAULT_NO_OF_STOP_BITS) )
        {
            return -1;
        }

        // Parity
        if ( -1 == SetParity(DEFAULT_PARITY) )
        {
            return -1;
        }

        // Flow control
        if ( -1 == SetFlowControl(DEFAULT_FLOW_CONTROL) )
        {
            return -1;
        }

        // VMin
        if ( -1 == SetVMin(DEFAULT_VMIN) )
        {
            return -1;
        }

        // VTime
        if ( -1 == SetVTime(DEFAULT_VTIME) )
        {
            return -1;
        }

        // All done. Return a value other than -1. 
        return 0;
    }

    inline
    int
    SerialStreamBuf::Implementation::InitializeSerialPort()
    {
        // If we do not have a valid file descriptor then return with
        // failure.
        if ( -1 == this->mFileDescriptor )
        {
            return -1;
        }

        // Use non-blocking mode while configuring the serial port. 
        int flags = fcntl(this->mFileDescriptor, F_GETFL, 0);
        
        if ( -1 == fcntl( this->mFileDescriptor, 
                         F_SETFL, 
                         flags | O_NONBLOCK ) )
        {
            return -1;
        }

        // Flush out any garbage left behind in the buffers associated
        // with the port from any previous operations. 
        if ( -1 == tcflush(this->mFileDescriptor, TCIOFLUSH) )
        {
            return -1;
        }

        // Set up the default configuration for the serial port.
        if ( -1 == this->SetParametersToDefault() )
        {
            return -1;
        }

        // Allow all further communications to happen in blocking mode.
        flags = fcntl(this->mFileDescriptor, F_GETFL, 0);
        
        if ( -1 == fcntl( this->mFileDescriptor, 
                         F_SETFL, 
                         flags & ~O_NONBLOCK ) )
        {
            return -1;
        }

        // If we get here without problems then we are good; return a value
        // different from -1.
        return 0;
    }

    inline
    BaudRate
    SerialStreamBuf::Implementation::SetBaudRate(const BaudRate baud_rate)
    {
        if ( -1 == mFileDescriptor )
        {
            return BaudRate::BAUD_INVALID;
        }

        switch (baud_rate)
        {
        case BaudRate::BAUD_50:
        case BaudRate::BAUD_75:
        case BaudRate::BAUD_110:
        case BaudRate::BAUD_134:
        case BaudRate::BAUD_150:
        case BaudRate::BAUD_200:
        case BaudRate::BAUD_300:
        case BaudRate::BAUD_600:
        case BaudRate::BAUD_1200:
        case BaudRate::BAUD_1800:
        case BaudRate::BAUD_2400:
        case BaudRate::BAUD_4800:
        case BaudRate::BAUD_9600:
        case BaudRate::BAUD_19200:
        case BaudRate::BAUD_38400:
        case BaudRate::BAUD_57600:
        case BaudRate::BAUD_115200:
            //
            // Get the current terminal settings. 
            //
            struct termios term_setting;
            if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
            {
                return BaudRate::BAUD_INVALID;
            }

            // Modify the baud rate in the term_setting structure.
            cfsetispeed( &term_setting, static_cast<speed_t>(baud_rate) );
            cfsetospeed( &term_setting, static_cast<speed_t>(baud_rate) );

            // Apply the modified termios structure to the serial 
            // port. 
            if ( -1 == tcsetattr(mFileDescriptor, TCSANOW, &term_setting) )
            {
                return BaudRate::BAUD_INVALID;
            }
            break;
        default:

            // :TODO: Thu Jul 13 16:30:14 2000 Pagey
            //
            // There is obviously a problem if we reach here. The method
            // must have been called with an invalid value of the baud
            // rate. We should probably throw an exception here. I will
            // print something on cerr for the time being but leave the
            // stream in "good" state. 
            return BaudRate::BAUD_INVALID;
            break;
        }


        // If we succeeded in setting the baud rate then we need to return
        // the baud rate.
        return GetBaudRate();
    }

    inline
    BaudRate
    SerialStreamBuf::Implementation::GetBaudRate() const
    {
        if ( -1 == mFileDescriptor )
        {
            return BaudRate::BAUD_INVALID;
        }

        // Get the current terminal settings. 
        struct termios term_setting;
        
        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return BaudRate::BAUD_INVALID;
        }

        // Read the input and output baud rates.
        speed_t input_baud = cfgetispeed( &term_setting );
        speed_t output_baud = cfgetospeed( &term_setting );

        // Make sure that the input and output baud rates are
        // equal. Otherwise, we do not know which one to return.
        if ( input_baud != output_baud )
        {
            return BaudRate::BAUD_INVALID;
        }

        switch( input_baud )
        {
        case B50: 
            return BaudRate::BAUD_50; break;
        case B75:
            return BaudRate::BAUD_75; break;
        case B110: 
            return BaudRate::BAUD_110; break;
        case B134: 
            return BaudRate::BAUD_134; break;
        case B150:
            return BaudRate::BAUD_150; break;
        case B200: 
            return BaudRate::BAUD_200; break;
        case B300:
            return BaudRate::BAUD_300; break;
        case B600:
            return BaudRate::BAUD_600; break;
        case B1200:
            return BaudRate::BAUD_1200; break;
        case B1800:
            return BaudRate::BAUD_1800; break;
        case B2400: 
            return BaudRate::BAUD_2400; break;
        case B4800:
            return BaudRate::BAUD_4800; break;
        case B9600:
            return BaudRate::BAUD_9600; break;
        case B19200:
            return BaudRate::BAUD_19200; break;
        case B38400:
            return BaudRate::BAUD_38400; break;
        case B57600:
            return BaudRate::BAUD_57600; break;
        case B115200:
            return BaudRate::BAUD_115200; break;
        default:
            return BaudRate::BAUD_INVALID; // we return an invalid value in this case. 
            break;
        }

        // The code should never reach here due to the fact that the default
        // section of the above switch statement returns. So we force an
        // abort here using an assertion which will always fail.
        assert( false );
        return BaudRate::BAUD_INVALID;
    }

    inline
    CharSize
    SerialStreamBuf::Implementation::SetCharSize(const CharSize char_size)
    {
        if ( -1 == mFileDescriptor )
        {
            return CharSize::CHAR_SIZE_INVALID;
        }

        switch(char_size)
        {
        case CharSize::CHAR_SIZE_5:
        case CharSize::CHAR_SIZE_6:
        case CharSize::CHAR_SIZE_7:
        case CharSize::CHAR_SIZE_8:

            // Get the current terminal settings.
            struct termios term_setting;
            if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
            {
                return CharSize::CHAR_SIZE_INVALID;
            }

            // Set the character size to the specified value. If the character
            // size is not 8 then it is also important to set ISTRIP. Setting
            // ISTRIP causes all but the 7 low-order bits to be set to
            // zero. Otherwise they are set to unspecified values and may
            // cause problems. At the same time, we should clear the ISTRIP
            // flag when the character size is 8 otherwise the MSB will always
            // be set to zero (ISTRIP does not check the character size
            // setting; it just sets every bit above the low 7 bits to zero).
            if ( char_size == CharSize::CHAR_SIZE_8 )
            {
                term_setting.c_iflag &= ~ISTRIP; // clear the ISTRIP flag.
            }
            else
            {
                term_setting.c_iflag |= ISTRIP;  // set the ISTRIP flag.
            }

            term_setting.c_cflag &= ~CSIZE;                             // clear all the CSIZE bits.
            term_setting.c_cflag |= static_cast<tcflag_t>(char_size);   // set the character size. 

            // Set the new settings for the serial port. 
            if ( -1 == tcsetattr(mFileDescriptor, TCSANOW, &term_setting) )
            {
                return CharSize::CHAR_SIZE_INVALID;
            }

            break;
        default:
            return CharSize::CHAR_SIZE_INVALID;
            break;
        }

        return this->GetCharSize();
    }

    inline
    CharSize
    SerialStreamBuf::Implementation::GetCharSize() const
    {
        if ( -1 == mFileDescriptor )
        {
            return CharSize::CHAR_SIZE_INVALID;
        }

        // Get the current terminal settings.
        struct termios term_setting;
        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return CharSize::CHAR_SIZE_INVALID;
        }

        // Extract the character size from the terminal settings.
        int char_size = (term_setting.c_cflag & CSIZE);
        switch( char_size )
        {
        case CS5:
            return CharSize::CHAR_SIZE_5; break;
        case CS6:
            return CharSize::CHAR_SIZE_6; break;
        case CS7: 
            return CharSize::CHAR_SIZE_7; break;
        case CS8:
            return CharSize::CHAR_SIZE_8; break;
        default:
            // If we get an invalid character, we set the badbit for the
            // stream associated with the serial port.
            return CharSize::CHAR_SIZE_INVALID;
            break;
        }

        return CharSize::CHAR_SIZE_INVALID;
    }

    inline
    short
    SerialStreamBuf::Implementation::SetNumOfStopBits(const short stop_bits)
    {
        if ( -1 == mFileDescriptor )
        {
            return 0;
        }

        // Get the current terminal settings.
        struct termios term_setting;

        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return 0;
        }

        switch( stop_bits )
        {
        case 1:
            term_setting.c_cflag &= ~CSTOPB;
            break;
        case 2:
            term_setting.c_cflag |= CSTOPB;
            break;
        default: 
            return 0;
            break;
        }

        // Set the new settings for the serial port. 
        if ( -1 == tcsetattr(mFileDescriptor, TCSANOW, &term_setting) )
        {
            return 0;
        }

        return this->GetNumOfStopBits();
    }

    inline
    short 
    SerialStreamBuf::Implementation::GetNumOfStopBits() const
    {
        if ( -1 == mFileDescriptor )
        {
            return 0;
        }

        // Get the current terminal settings.
        struct termios term_setting;

        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return 0;
        }

        // If CSTOPB is set then the number of stop bits is 2 otherwise it
        // is 1.
        if ( term_setting.c_cflag & CSTOPB )
        {
            return 2;
        }
        else
        {
            return 1;
        }
    }

    inline
    SerialStreamBuf::ParityEnum
    SerialStreamBuf::Implementation::SetParity(const SerialStreamBuf::ParityEnum parity) 
    {
        if ( -1 == mFileDescriptor )
        {
            return PARITY_INVALID;
        }

        // Get the current terminal settings. 
        struct termios term_setting;
        
        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return PARITY_INVALID;
        }

        // Set the parity in the termios structure. 
        switch( parity )
        {
        case PARITY_EVEN:
            term_setting.c_cflag |= PARENB;
            term_setting.c_cflag &= ~PARODD;
            break;
        case PARITY_ODD:
            term_setting.c_cflag |= PARENB;
            term_setting.c_cflag |= PARODD;
            break;
        case PARITY_NONE:
            term_setting.c_cflag &= ~PARENB;
            break;
        default:
            return PARITY_INVALID;
        }

        // Write the settings back to the serial port. 
        if ( -1 == tcsetattr(mFileDescriptor, TCSANOW, &term_setting) )
        {
            return PARITY_INVALID;
        }

        return GetParity();
    }


    inline
    SerialStreamBuf::ParityEnum
    SerialStreamBuf::Implementation::GetParity() const 
    {
        if ( -1 == mFileDescriptor )
        {
            return PARITY_INVALID;
        }

        // Get the current terminal settings.
        struct termios term_setting;
        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return PARITY_INVALID;
        }

        // Get the parity setting from the termios structure. 

        if ( term_setting.c_cflag & PARENB )
        {   // parity is enabled.
            if (term_setting.c_cflag & PARODD)  // odd parity
            {
                return PARITY_ODD; 
            }
            else                                // even parity
            {
                return PARITY_EVEN;
            }
        }
        else                                    // no parity.
        {
            return PARITY_NONE;
        }

        return PARITY_INVALID; // execution should never reach here. 
    }

    SerialStreamBuf::FlowControlEnum
    SerialStreamBuf::Implementation::SetFlowControl(const SerialStreamBuf::FlowControlEnum flow_c)
    {
        if ( -1 == mFileDescriptor )
        {
            return FLOW_CONTROL_INVALID;
        }

        // Flush any unwritten, unread data from the serial port.
        if ( -1 == tcflush(mFileDescriptor, TCIOFLUSH) ) 
        {
            return FLOW_CONTROL_INVALID;
        }

        // Get the current terminal settings.
        struct termios tset;
        int retval = tcgetattr(mFileDescriptor, &tset);
        
        if (-1 == retval)
        {
            return FLOW_CONTROL_INVALID;
        }

        // Set the flow control. Hardware flow control uses the RTS (Ready
        // To Send) and CTS (clear to Send) lines. Software flow control
        // uses IXON|IXOFF
        if ( FLOW_CONTROL_HARD == flow_c )
        {
            tset.c_iflag &= ~ (IXON|IXOFF);
            tset.c_cflag |= CRTSCTS;
            tset.c_cc[VSTART] = _POSIX_VDISABLE;
            tset.c_cc[VSTOP] = _POSIX_VDISABLE;
        }
        else if ( FLOW_CONTROL_SOFT == flow_c )
        {
            tset.c_iflag |= IXON|IXOFF;
            tset.c_cflag &= ~CRTSCTS;
            tset.c_cc[VSTART] = CTRL_Q; // 0x11 (021) ^q
            tset.c_cc[VSTOP]  = CTRL_S; // 0x13 (023) ^s
        }
        else
        {
            tset.c_iflag &= ~(IXON|IXOFF);
            tset.c_cflag &= ~CRTSCTS;
        }
        
        retval = tcsetattr(mFileDescriptor, TCSANOW, &tset);
        
        if (-1 == retval) {
            return FLOW_CONTROL_INVALID;
        }

        return GetFlowControl();
    }

    inline
    SerialStreamBuf::FlowControlEnum
    SerialStreamBuf::Implementation::GetFlowControl() const 
    {
        if ( -1 == mFileDescriptor )
        {
            return FLOW_CONTROL_INVALID;
        }

        // Get the current terminal settings.
        struct termios tset;

        if ( -1 == tcgetattr(mFileDescriptor, &tset) )
        {
            return FLOW_CONTROL_INVALID;
        }

        // Check if IXON and IXOFF are set in c_iflag. If both are set and
        // VSTART and VSTOP are set to 0x11 (^Q) and 0x13 (^S) respectively,
        // then we are using software flow control.
        if ( (tset.c_iflag & IXON)         &&
             (tset.c_iflag & IXOFF)        &&
             (CTRL_Q == tset.c_cc[VSTART]) &&
             (CTRL_S == tset.c_cc[VSTOP] ) )
        {
            return FLOW_CONTROL_SOFT;
        }
        else if ( ! ( (tset.c_iflag & IXON) ||
                      (tset.c_iflag & IXOFF) ) )
        {
            if ( tset.c_cflag & CRTSCTS )
            {
                //
                // If neither IXON or IXOFF is set then we must have hardware flow
                // control.
                //
                return FLOW_CONTROL_HARD;
            }
            else
            {
                return FLOW_CONTROL_NONE;
            }
        }

        // If none of the above conditions are satisfied then the serial
        // port is using a flow control setup which we do not support at
        // present.
        return FLOW_CONTROL_INVALID;
    }

    inline
    short 
    SerialStreamBuf::Implementation::SetVMin( short vmin )
    {
        if ( -1 == mFileDescriptor )
        {
            return -1;
        }

        if ( vmin < 0 || vmin > 255 )
        {
            return -1;
        }

        // Get the current terminal settings. 
        struct termios term_setting;

        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return -1;
        }

        term_setting.c_cc[VMIN] = (cc_t)vmin;

        // Set the new settings for the serial port. 
        if ( -1 == tcsetattr(mFileDescriptor, TCSANOW, &term_setting) )
        {
            return -1;
        } 

        return vmin;
    }

    short 
    SerialStreamBuf::Implementation::GetVMin() const
    {
        if ( -1 == mFileDescriptor )
        {
            return -1;
        }

        // Get the current terminal settings. 
        struct termios term_setting;
        
        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return -1;
        }

        return term_setting.c_cc[VMIN];
    }

    inline
    short 
    SerialStreamBuf::Implementation::SetVTime(const short vtime)
    {
        if ( -1 == mFileDescriptor )
        {
            return -1;
        }

        if ( vtime < 0 || vtime > 255 )
        {
            return -1;
        }

        //
        // Get the current terminal settings. 
        //
        struct termios term_setting;

        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return -1;
        }

        term_setting.c_cc[VTIME] = (cc_t)vtime;

        // Set the new settings for the serial port. 
        if ( -1 == tcsetattr(mFileDescriptor, TCSANOW, &term_setting) )
        {
            return -1;
        }

        return vtime;
    }

    inline
    short 
    SerialStreamBuf::Implementation::GetVTime() const 
    {
        if ( -1 == mFileDescriptor )
        {
            return -1;
        }

        // Get the current terminal settings. 
        struct termios term_setting;

        if ( -1 == tcgetattr(mFileDescriptor, &term_setting) )
        {
            return -1;
        }

        return term_setting.c_cc[VTIME];
    }

    inline
    streamsize
    SerialStreamBuf::Implementation::xsgetn(char_type *s, streamsize n) 
    {
        // If mFileDescriptor is -1 then we do not have a valid serial port
        // associated with this buffer. Hence, we cannot read any characters
        // from the serial port. Similarly, if the parameter n is less than
        // or equal to 0, then we do not need to do anything here.
        if ( (-1 == mFileDescriptor) ||
            (n <= 0) )
        {
            return 0;
        }

        // Try to read upto n characters in the array s.
        ssize_t retval {0};

        // If a putback character is available, then we need to read only
        // n-1 character.
        if ( mPutbackAvailable )
        {
            // Put the mPutbackChar at the beginning of the array,
            // s. Increment retval to indicate that a character has been
            // placed in s.
            // 
            // (Corrected Bug#2364846 by incrementing retval below)
            s[0] = mPutbackChar; 
            ++retval;

            // The putback character is no longer available. 
            mPutbackAvailable = false;
            //
            // If we need to read more than one character, then call read()
            // and try to read n-1 more characters and put them at location
            // starting from &s[1].
            //
            if ( n > 1 )
            {
                retval = read(mFileDescriptor, &s[1], n-1);

                // If read was successful, then we need to increment retval by
                // one to indicate that the putback character was prepended to
                // the array, s. If read failed then leave retval at -1.
                if ( retval != -1 )
                {
                    retval ++;
                }
            }
        }
        else
        {

            // If no putback character is available then we try to read n
            // characters.
            retval = read(mFileDescriptor, s, n);
        }

        // If retval == -1 then the read call had an error, otherwise, if
        // retval == 0 then we could not read the characters. In either
        // case, we return 0 to indicate that no characters could be read
        // from the serial port.
        if ( ( -1 == retval ) ||
             (  0 == retval ) )
        {
            return 0;
        }

        // Return the number of characters actually read from the serial
        // port.
        return retval;
    }

    inline
    streamsize
    SerialStreamBuf::Implementation::xsputn(const char_type *s, streamsize n) 
    {
        // If we do not have a valid file descriptor, then we cannot do much
        // here. Similarly if n is non-positive then we have nothing to do
        // here.
        if ( (-1 == mFileDescriptor) ||
            (n <= 0) )
        {
            return 0;
        }

        // Write the n characters to the serial port. 
        ssize_t retval = write(mFileDescriptor, s, n);

        // If the write failed then return 0. 
        if ( (-1 == retval) ||
            ( 0 == retval) )
        {
            return 0;
        }

        // Otherwise, return the number of bytes actually written.
        return retval;
    }

    inline
    std::streamsize
    SerialStreamBuf::Implementation::showmanyc()
    {

        int retval = -1;

        if ( -1 == mFileDescriptor )
        {
            return -1;
        };

        if ( mPutbackAvailable )
        {

            // We still have a character left in the buffer.
            retval = 1;

        }
        else
        {
            // Switch to non-blocking read.
            int flags = fcntl(this->mFileDescriptor, F_GETFL, 0);
            if ( -1 == fcntl( this->mFileDescriptor, 
                              F_SETFL, 
                              flags | O_NONBLOCK ) )
            {
                return -1;
            }

            // Try to read a character.
            retval = read(mFileDescriptor, &mPutbackChar, 1);

            if ( retval == 1 )
            {
                mPutbackAvailable = true;
            }
            else
            {
                retval = 0;
            }

            // Switch back to blocking read.
            if ( -1 == fcntl( this->mFileDescriptor,
                              F_SETFL, 
                              flags ) )
            {
                return -1;
            }
        }

        return retval;    
    }

    inline
    streambuf::int_type
    SerialStreamBuf::Implementation::overflow(int_type c) 
    {
        // If we do not have a valid file descriptor then we cannot do much here.
        if ( -1 == mFileDescriptor )
        {
            return traits_type::eof();
        }

        // Try to write the specified character to the serial port. 
        if ( traits_type::eq_int_type( c, traits_type::eof()) )
        {
            // If c is the eof character then we do nothing. 
            return traits_type::eof();
        }
        else
        {
            // Otherwise we write the character to the serial port. 
            char out_ch = traits_type::to_char_type(c);
            ssize_t retval = write(mFileDescriptor, &out_ch, 1);

            // If the write failed then return eof. 
            if ( (-1 == retval) ||
                 ( 0 == retval) )
            {
                return traits_type::eof();
            }

            // Otherwise, return something other than eof().
            return traits_type::not_eof(c);
        }

        assert( 0 == "The code should never reach here." );
    }

    inline
    streambuf::int_type
    SerialStreamBuf::Implementation::underflow() 
    {

        // If we do not have a valid file handler for the serial port, we
        // cannot do much.
        if ( -1 == mFileDescriptor )
        {
            return traits_type::eof();
        }

        // Read the next character from the serial port. 
        char next_ch;
        ssize_t retval;

        // If a putback character is available then we return that
        // character. However, we are not supposed to change the value of
        // gptr() in this routine so we leave mPutbackAvailable set to true.

        if ( mPutbackAvailable )
        {
            next_ch = mPutbackChar;
        }
        else
        {
            // If no putback character is available then we need to read one
            // character from the serial port.
            retval = read(mFileDescriptor, &next_ch, 1);

            // Make the next character the putback character. This has the
            // effect of returning the next character without changing gptr()
            // as required by the C++ standard.
            if ( retval == 1 )
            {
                mPutbackChar = next_ch;
                mPutbackAvailable = true;
            }
            else if ( ( -1 == retval ) ||
                      (  0 == retval ) )
            {
                // If we had a problem reading the character, we return
                // traits::eof().
                return traits_type::eof();
            }
        }

        // :NOTE: Wed Aug  9 21:26:51 2000 Pagey
        // The value of mPutbackAvailable is always true when the code
        // reaches here.

        // Return the character as an int value as required by the C++
        // standard.
        return traits_type::to_int_type(next_ch);
    }

    inline
    streambuf::int_type
    SerialStreamBuf::Implementation::pbackfail(int_type c) 
    {
        // If we do not have a valid file descriptor, then we return eof. 
        if ( -1 == mFileDescriptor )
        {
            return traits_type::eof();
        }

        // If a putback character is already available, then we cannot
        // do any more putback and hence need to return eof.
        if ( mPutbackAvailable )
        {
            return traits_type::eof();
        }
        else if ( traits_type::eq_int_type(c, traits_type::eof()) )
        {
            // If an eof character is passed in, then we are required to
            // backup one character. However, we cannot do this for a serial
            // port. Hence we return eof to signal an error.
            return traits_type::eof();
        }
        else
        {
            // If no putback character is available at present, then make
            // c the putback character and return it.
            mPutbackChar = traits_type::to_char_type(c);
            mPutbackAvailable = true;
            return traits_type::not_eof(c);
        }
    }
}