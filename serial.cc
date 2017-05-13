#include <assert.h>
#include <iostream>
#include "ftd2xx.h"
#include "vector"
#include <thread>
#include <chrono>
 

//
// Some fancy MPSSE (Multi-Protocol Synchronous Serial Engine) Commands
// This is a subset of the entire set, since I'm only adding them as I use them
//
enum mpsse : unsigned char
{
    SET_D_BUS_DATA               = 0x80, // 0xVALUE, 0xDIRECTION (1 = output)
    GET_D_BUS_DATA               = 0x81,
    SET_C_BUS_DATA               = 0x82, // 0xVALUE, 0xDIRECTION (1 = output)
    GET_C_BUS_DATA               = 0x83,
    LOOPBACK_ENABLE              = 0x84,
    LOOPBACK_DISABLE             = 0x85,
    SET_TCK_DIVISOR              = 0x86, // VAL_L, VAL_H
    CLOCK_60_MHZ                 = 0x8A,
    THREE_PHASE_CLOCKING_ENABLE  = 0x8C,
    THREE_PHASE_CLOCKING_DISABLE = 0x8D,
    ADAPTIVE_CLOCKING_ENABLE     = 0x96,
    ADAPTIVE_CLOCKING_DISABLE    = 0x97,
    BAD_COMMANDS                 = 0xFA
};

//
// Connects to an FTDI serial connection and has some nice wrappers C++11 around the
// gross C
//
class SerialConnection
{
public: // constructor ////////////////////////////////////////////////////////
    SerialConnection(const unsigned int device_number = 0)
    {
        //
        // Let's do some basic set up of the port here, don't trust me - trust
        // http://www.ftdichip.com/Support/Documents/AppNotes/AN_135_MPSSE_Basics.pdf
        //


        //
        // First check if there is a valid device to use
        //
        DWORD number_of_devices;
        assert(FT_CreateDeviceInfoList(&number_of_devices) == FT_OK);
        std::cout << number_of_devices << " devices found!" << std::endl;
        if (number_of_devices < 1)
        {
            std::cout << "Need at least one device" << std::endl;
            return;
        }

        //
        // Try to open it
        //
        assert(FT_Open(device_number, &ft_handle) == FT_OK);
        std::cout << "Device opened successfully!" << std::endl;

        //
        // Now let's configure our device, to do that first we need to reset the receive buffer
        //
        FT_STATUS ft_status;
        DWORD num_bytes_to_read = 0;
        DWORD num_bytes_read = 0;
        ft_status |= FT_ResetDevice(ft_handle);
        ft_status |= FT_GetQueueStatus(ft_handle, &num_bytes_to_read);

        if ((ft_status == FT_OK) && (num_bytes_to_read > 0))
        {
            //
            // There are bytes to read, so perform a read just to clear that data
            //
            BYTE in_buffer[8];
            FT_Read(ft_handle, &in_buffer, num_bytes_to_read, &num_bytes_read);
        }

        //
        // Some needed configs
        //
        //Set USB request transfer sizes to 64K
        ft_status |= FT_SetUSBParameters(ft_handle, 65536, 65535);
        //Disable event and error characters
        ft_status |= FT_SetChars(ft_handle, false, 0, false, 0);
        //Sets the read and write timeouts in milliseconds
        ft_status |= FT_SetTimeouts(ft_handle, 0, 5000);
        //Set the latency timer to 1mS (default is 16mS) 
        ft_status |= FT_SetLatencyTimer(ft_handle, 1); 
        //Turn on flow control to synchronize IN requests
        ft_status |= FT_SetFlowControl(ft_handle, FT_FLOW_RTS_CTS, 0x00, 0x00);
        //Enable MPSSE mode
        ft_status |= FT_SetBitMode(ft_handle, 0x0, 0x02);

        assert(ft_status == FT_OK);
        std::cout << "MPSSE mode enabled successfully!" << std::endl;
    }

    ~SerialConnection()
    {
        FT_SetBitMode(ft_handle, 0x0, 0x00);
        FT_Close(ft_handle);
    }

public: // types //////////////////////////////////////////////////////////////
    //
    // Public type used by others when writing data to the board
    //
    using ByteVector_t = std::vector<BYTE>;

public: // publicest methods //////////////////////////////////////////////////
    //
    // Simple c++11 wrapper to write some data and check that it went through.
    // Give it a byte vector that you don't care about since it'll eat it
    //
    bool write_data(ByteVector_t data) const
    {
        const unsigned int bytes_to_send = data.size();
        unsigned int bytes_sent = 0;
        FT_STATUS ft_status = FT_Write(ft_handle, data.data(), bytes_to_send, &bytes_sent);
        if (status_okay(ft_status) == false)
        {
            return false;
        };

        return bytes_sent == bytes_to_send;
    };

    //
    // Wait until we have some number of bytes in the receive buffer,
    // read them, clear them, and return the data
    // TODO: Timeouts
    //
    ByteVector_t block_and_read(const unsigned int num_bytes_to_read) const
    {
        unsigned int bytes_ready = 0;
        FT_STATUS ft_status = FT_OK;

        //
        // Wait until we get enough data
        //
        while (bytes_ready < num_bytes_to_read)
        {
            ft_status = FT_GetQueueStatus(ft_handle, &bytes_ready);
            if (status_okay(ft_status) == false)
            {
                // return nothing is there was a problem
                return {};
            }
        }

        //
        // Actually read the data
        //
        ByteVector_t recv_buffer(num_bytes_to_read);
        unsigned int bytes_read = 0;
        ft_status = FT_Read(ft_handle, recv_buffer.data(), num_bytes_to_read, &bytes_read);
        check_bad_response(recv_buffer);
        if (status_okay(ft_status) == false)
        {
            return {};
        }

        //
        // Give the user the recv buffer
        //
        assert(bytes_read == num_bytes_to_read);
        return recv_buffer;
    }

    //
    // Request the data that is on a pin. According to the documentation, there are two
    // pin sets: D and C. D is the lower byte and C is the upper byte. The Pin number
    // here will start at 0 for D0 and end at 15 for C7. C8 and C9 are reserved
    //
    // TODO: There could be a more efficient implementation that could check multiple
    //       pins at once, but I don't plan to use this for much
    //
    bool get_pin(const size_t pin_number) const
    {
        //
        // Check if we need to query the upper pins or lower pins
        // `pin_number_offset` is the bit number in the set, ie `pin_number`
        // may be 8 meaning the caller wants C0, but for the bit shift we want
        // an offset in the byte
        //
        size_t pin_number_offset = pin_number;
        ByteVector_t request = {mpsse::GET_D_BUS_DATA};
        if(pin_number > 7)
        {
            pin_number_offset -= 8;
            request[0] = mpsse::GET_C_BUS_DATA;
        }

        write_data(request);

        ByteVector_t response = block_and_read(1);
        std::cout << static_cast<uint16_t>(response[0]) << std::endl;
        return (response[0] >> pin_number_offset) & 1;
    }

    //
    // Set a pin with the same rules as above
    //
    void set_pin(const size_t pin_number, bool high) const
    {
        ByteVector_t request = {mpsse::SET_D_BUS_DATA, 0x00, 0xFB};
        request[1] = high ? 0xFF : 0x00;
        write_data(request);
    }

public: // more public methods ////////////////////////////////////////////////
    //
    // Sets default values for pins and what not - maybe this should go in the
    // constructor?
    //
    void configure_mpsse_defaults()
    {
        //
        // Hardware parameters that should be set to default.
        //
        ByteVector_t hardware_config = {mpsse::CLOCK_60_MHZ,
                                        mpsse::ADAPTIVE_CLOCKING_DISABLE,
                                        mpsse::THREE_PHASE_CLOCKING_DISABLE};
        write_data(hardware_config);

        //
        // Clock divisor should be 0x05DB
        //
        write_data({mpsse::SET_TCK_DIVISOR, 0xDB, 0x05});

        //
        // Time to configure the defaults for all the pins. We need to configure
        // the default value and direction for both D and C pins
        //
        write_data({mpsse::SET_D_BUS_DATA, 0xC9, 0xFB});
        write_data({mpsse::SET_C_BUS_DATA, 0x00, 0x00});

        std::cout << "MPSSE Configuration Successful!" << std::endl;
    }

    //
    // Basic test script - sends some bad data and ensures it gets an error back
    // Returns true if everything is working, false otherwise
    //
    void run_comms_check()
    {
        std::cout << "\nRunning comms check... =============" << std::endl;

        ByteVector_t data = {mpsse::LOOPBACK_ENABLE};
        assert(write_data(data));
        std::cout << "Loopback enabled!" << std::endl;

        //
        // Now to write the bad data
        //
        data = {0xAB};
        assert(write_data(data));
        std::cout << "Bad data sent..." << std::endl;
        ByteVector_t recv = block_and_read(2);
        assert(recv[1] == data[0]);
        std::cout << "Response matches expected response!" << std::endl;

        std::cout << "Passed! ============================\n" << std::endl;
    };

private: // methods ///////////////////////////////////////////////////////////
    //
    // Makes sure the status return FT_OK
    //
    inline bool status_okay(const FT_STATUS ft_status) const
    {
        if (ft_status != FT_OK)
        {
            std::cout << "ERROR: ft_status = " << ft_status << std::endl;
            return false;
        }
        return true;
    }

    //
    // Checks for the bad response code and alerts the user
    //
    inline void check_bad_response(const ByteVector_t &recv_buffer) const
    {
        if (recv_buffer.size() == 0)
            return;

        if (recv_buffer[0] == mpsse::BAD_COMMANDS)
        {
            std::cout << "BAD COMMAND(S) DETECTED: ";
            for (const BYTE &byte : recv_buffer)
            {
                std::cout << static_cast<uint16_t>(byte) << " ";
            }
            std::cout << std::endl;
        }
    }

private: // members ///////////////////////////////////////////////////////////

    //
    // Handle to the device
    //
    FT_HANDLE ft_handle;

};

class NeoPixelCommunitron
{
public: // types //////////////////////////////////////////////////////////////
    using ms = std::chrono::milliseconds;
    using us = std::chrono::microseconds;
    using ns = std::chrono::nanoseconds;
    using generator = std::function<void()>;

public: // constructor ////////////////////////////////////////////////////////
    //
    // Gives an easy interface for converting bytes to neopixel command packets
    // This doesn't build color packets, it just does the translation
    // https://learn.adafruit.com/adafruit-neopixel-uberguide/advanced-coding
    //
    NeoPixelCommunitron(SerialConnection &s_, const size_t data_pin_) :
        s(s_), data_pin(data_pin_)
    { };

public: // methods ///////////////////////////////////////////////////////////
    //
    // Given a byte, send it over the wire to the display
    //
    inline void sent_byte(unsigned char byte)
    {
        unsigned char mask = 0x80;
        for (size_t i = 0; i < 8; ++i)
        {
            byte * mask == 0 ? zero() : one();
            mask = mask >> 1;
        }
    }

    //
    // Call between groups of LED color data
    //
    inline void done()
    {
        s.set_pin(data_pin, 0);
        std::this_thread::sleep_for(us(50));
    }

private: // methods ///////////////////////////////////////////////////////////

    inline void one() const
    {
        s.set_pin(data_pin, 1);
        std::this_thread::sleep_for(ns(850));
        s.set_pin(data_pin, 0);
        std::this_thread::sleep_for(ns(450));
    }
    inline void zero() const
    {
        s.set_pin(data_pin, 0);
        std::this_thread::sleep_for(ns(400));
        s.set_pin(data_pin, 1);
        std::this_thread::sleep_for(ns(800));
    }

private: // members ///////////////////////////////////////////////////////////
    SerialConnection s;
    size_t data_pin;
};

int main()
{
    SerialConnection s;
    s.run_comms_check();
    s.configure_mpsse_defaults();
    NeoPixelCommunitron comms(s, 5);

    while(true)
    {
        comms.sent_byte(0xFF);
        comms.sent_byte(0xFF);
        comms.sent_byte(0xFF);
        comms.done();
    }
}
