#include "serial.hh"
#include <assert.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <algorithm>



namespace serial
{

//
// ### constructor ############################################################
//

SerialConnection::SerialConnection(const unsigned int device_number)
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
    FT_STATUS ft_status = FT_OK;
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
    // Set USB request transfer sizes to 64K
    ft_status |= FT_SetUSBParameters(ft_handle, 65536, 65535);
    // Disable event and error characters
    ft_status |= FT_SetChars(ft_handle, false, 0, false, 0);
    // Sets the read and write timeouts in milliseconds
    ft_status |= FT_SetTimeouts(ft_handle, 0, 5000);
    // Set the latency timer to 1mS (default is 16mS) 
    ft_status |= FT_SetLatencyTimer(ft_handle, 1); 
    // Turn on flow control to synchronize IN requests
    ft_status |= FT_SetFlowControl(ft_handle, FT_FLOW_RTS_CTS, 0x00, 0x00);
    // Reset controller
    ft_status |= FT_SetBitMode(ft_handle, 0x0, 0x00);
    // Enable MPSSE mode
    ft_status |= FT_SetBitMode(ft_handle, 0x0, 0x02);

    assert(ft_status == FT_OK);
    std::cout << "MPSSE mode enabled successfully!" << std::endl;
}

//
// ############################################################################
//

SerialConnection::~SerialConnection()
{
    FT_SetBitMode(ft_handle, 0x0, 0x00);
    FT_Close(ft_handle);
}

//
// ### public methods #########################################################
//

bool SerialConnection::write_data(ByteVector_t data) const
{
    const unsigned int bytes_to_send = data.size();
    unsigned int bytes_sent = 0;
    FT_STATUS ft_status = FT_Write(ft_handle, data.data(), bytes_to_send, &bytes_sent);

    // for (BYTE &b : data)
    // {
    //     std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (uint32_t) b << " ";
    // }
    // std::cout << "(" << ft_status << ")" << " sent: " << bytes_sent << std::endl;

    if (status_okay(ft_status) == false)
    {
        return false;
    };

    return bytes_sent == bytes_to_send;
}

//
// ############################################################################
//

SerialConnection::ByteVector_t SerialConnection::block_and_read(const unsigned int num_bytes_to_read) const
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
// ############################################################################
//

bool SerialConnection::spi_write_data(ByteVector_t data) const
{
    const uint16_t data_length = data.size() - 1;
    const BYTE length_L = data_length & 0xFF;
    const BYTE length_H = (data_length >> 8);

    //
    // Append header data
    //
    ByteVector_t header_data = {mpsse::MSB_R_EDGE_OUT_BYTE, length_L, length_H};
    data.insert(data.begin(), header_data.begin(), header_data.end());

    return write_data(std::move(data));
}

//
// ############################################################################
//

bool SerialConnection::get_pin(const size_t pin_number) const
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
// ############################################################################
//

void SerialConnection::set_pin(const size_t pin_number, bool high) const
{
    // TODO: Fix this so it doesn't set the whole row
    // ByteVector_t request = {mpsse::SET_D_BUS_DATA, 0x00, 0xFB};
    // request[1] = high ? 0xFF : 0x00;
    // write_data(request);
}

//
// ############################################################################
//

void SerialConnection::configure_spi_defaults() const
{
    //
    // Hardware parameters that should be set to default.
    //
    ByteVector_t hardware_config = {mpsse::CLOCK_60_MHZ,
                                    mpsse::ADAPTIVE_CLOCKING_DISABLE,
                                    mpsse::THREE_PHASE_CLOCKING_DISABLE};
    write_data(hardware_config);

    //
    // We want a 6mhz clock, this divisor does that according to documentation
    //
    write_data({mpsse::SET_TCK_DIVISOR, 0x05, 0x00});

    //
    // We need to configure the default value and direction for both D and C pins
    //
    write_data({mpsse::SET_D_BUS_DATA, 0xC9, 0xFB});
    write_data({mpsse::SET_C_BUS_DATA, 0x00, 0x00});

    write_data({mpsse::LOOPBACK_DISABLE});

    std::cout << "SPI Configuration Successful!" << std::endl;
}

//
// ############################################################################
//

void SerialConnection::run_comms_check() const
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
}

//
// ### private methods ########################################################
//

inline bool SerialConnection::status_okay(const FT_STATUS ft_status) const
{
    if (ft_status != FT_OK)
    {
        std::cout << "ERROR: ft_status = " << ft_status << std::endl;
        return false;
    }
    return true;
}

//
// ############################################################################
//

inline void SerialConnection::check_bad_response(const ByteVector_t &recv_buffer) const
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
} // namespace serial

enum bits : unsigned char
{
    ZERO = 0xC0,
    ONE = 0xF8
};

int main()
{
    serial::SerialConnection s;
    s.configure_spi_defaults();

    serial::SerialConnection::ByteVector_t on(24 * 24, bits::ONE);
    serial::SerialConnection::ByteVector_t off(24 * 24, bits::ZERO);
    serial::SerialConnection::ByteVector_t green(24 * 24, bits::ZERO);
    for (size_t i = 0; i < 1; ++i)
    {
        for (size_t j = 0; j < 8; ++j)
        {
            green[i + j] = bits::ONE;
        }
        for (size_t j = 0; j < 16; ++j)
        {
            green[8 + j] = bits::ZERO;
        }
    }

    while (true)
    {
        s.spi_write_data(green);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        s.spi_write_data(off);
        std::rotate(green.begin(), green.begin()+24, green.end());
    }
    return 0;
}
