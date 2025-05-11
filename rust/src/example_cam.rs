//! # `arducam-mega`
//!
//! `arducam-mega` is an [`embedded-hal`][hal] SPI driver for the [Arducam Mega][mega]. This driver
//! aims to provide access to all features of the camera. However, due to hardware access and time
//! constraints, not all features and hardware variants have been actively tested. We welcome any
//! and all contributions improving the support and quality of this library.
//!
//! [hal]: https://github.com/rust-embedded/embedded-hal
//! [mega]: https://www.arducam.com/camera-for-any-microcontroller/
//!
//! # Examples
//!
//! The following example shows how the camera could be used on an ESP32 using the SPI3 device.
//!
//! ```ignore
//! let peripherals = Peripherals::take();
//! let clocks = ClockControl::configure(system.clock_control, CpuClock::Clock240MHz).freeze();
//! let delay = Delay::new(&clocks);
//!
//! let io = IO::new(peripherals.GPIO, peripherals.IO_MUX);
//! let sclk = io.pins.gpio18;
//! let miso = io.pins.gpio19;
//! let mosi = io.pins.gpio23;
//! let cs = io.pins.gpio5;
//!
//! let spi_controller = SpiBusController::from_spi(Spi::new_no_cs(
//!     peripherals.SPI3,
//!     sclk,
//!     mosi,
//!     miso,
//!     8u32.MHz(),
//!     SpiMode::Mode0,
//!     &mut system.peripheral_clock_control,
//!     &clocks,
//! ));
//!
//! let spi_device_1 = spi_controller.add_device(cs);
//!
//! let mut cam = ArducamMega::new(spi_device_1, delay);
//!
//! cam.reset()?
//!     .set_format(Format::Jpeg)?
//!     .set_resolution(Resolution::Hd)?
//!     .set_white_balance_mode(WhiteBalanceMode::Home)?;
//!
//! let length = cam
//!     .capture()?
//!     .read_fifo_length()?;
//!
//! // assuming buf.len() == length
//! cam.read_fifo_full(buf)?;
//! ```

#![cfg_attr(not(test), no_std)]
// only enables the `doc_cfg` feature when
// the `docsrs` configuration attribute is defined
#![cfg_attr(docsrs, feature(doc_cfg))]

#[cfg(not(any(feature = "3mp", feature = "5mp")))]
compile_error!(
    "at least one of the `arducam-mega/3mp` or `arducam-mega/5mp` features needs to be enabled"
);

use embedded_hal::{delay::DelayUs, spi::SpiBus, spi::SpiDevice};

const WRITE_REGISTER_MASK: u8 = 0x80;
const SENSOR_STATE_MASK: u8 = 0x03;
const SENSOR_STATE_IDLE: u8 = 0x02;
const SENSOR_RESET_ENABLE: u8 = 0x40;
const CAPTURE_FINISHED_MASK: u8 = 0x04;
const FIFO_READ_BURST: u8 = 0x3c;
const FIFO_READ_SINGLE: u8 = 0x3d;

/// Represents the type of camera connected to the SPI bus
///
/// This enum is typically returned by [`ArducamMega::get_camera_type`], and indicates what kind of
/// camera was detected on the SPI bus.
///
/// **Please note**: The value `0x82` for `OV3640` is a guess based on the Arducam SDK. Please
/// submit an issue on GitHub if you have a 3MP Mega and can confirm that this works. Likewise, if
/// your camera reports the `Unknown(u8)` variant, please get in touch so we can add support for it
/// in the library.
#[derive(Debug, Clone, PartialEq)]
#[repr(u8)]
pub enum CameraType {
    /// Arducam Mega 5MP
    OV5640 = 0x81,
    /// Arducam Mega 3MP
    OV3640 = 0x82,
    Unknown(u8),
}

impl From<u8> for CameraType {
    fn from(id: u8) -> Self {
        match id {
            0x81 => CameraType::OV5640,
            0x82 => CameraType::OV3640,
            id => CameraType::Unknown(id),
        }
    }
}

#[derive(Debug, Clone)]
#[repr(u8)]
enum CameraControl {
    Gain = 0x00, // ISO
    Exposure = 0x01,
    WhiteBalance = 0x02,
}

#[derive(Debug, Clone)]
#[repr(u8)]
enum RegisterAddress {
    Power = 0x02,
    ArduchipFifo = 0x04,
    SensorReset = 0x07,
    DebugDeviceAddress = 0x0a,
    Format = 0x20,
    Resolution = 0x21,
    Brightness = 0x22,
    Contrast = 0x23,
    Saturation = 0x24,
    Exposure = 0x25,
    WhiteBalanceMode = 0x26,
    ColorEffect = 0x27,
    #[cfg(feature = "3mp")]
    Sharpness = 0x28,
    #[cfg(feature = "5mp")]
    AutoFocus = 0x29,
    GainExposureWhiteBalance = 0x2a,
    SensorId = 0x40,
    SensorState = 0x44,
    FifoSize1 = 0x45,
    FifoSize2 = 0x46,
    FifoSize3 = 0x47,
}

/// Values to set the sharpness of the camera
#[cfg_attr(docsrs, doc(cfg(feature = "3mp")))]
#[cfg(feature = "3mp")]
#[derive(Debug, Clone, Default)]
#[repr(u8)]
pub enum SharpnessLevel {
    #[default]
    Auto = 0x00,
    One = 0x01,
    Two = 0x02,
    Three = 0x03,
    Four = 0x04,
    Five = 0x05,
    Six = 0x06,
    Seven = 0x07,
    Eight = 0x08,
}

/// Values to set the color effect of the camera
#[derive(Debug, Clone, Default)]
#[repr(u8)]
pub enum ColorEffect {
    #[default]
    None = 0x00,
    Blueish = 0x01,
    Redish = 0x02,
    BlackWhite = 0x03,
    Sepia = 0x04,
    Negative = 0x05,
    GrassGreen = 0x06,
    OverExposure = 0x07,
    Solarize = 0x08,
}

/// Values to set the level of some configuration settings
#[derive(Debug, Clone, Default)]
#[repr(u8)]
pub enum Level {
    #[default]
    Default = 0x00,
    PlusOne = 0x01,
    MinusOne = 0x02,
    PlusTwo = 0x03,
    MinusTwo = 0x04,
    PlusThree = 0x05,
    MinusThree = 0x06,
}

/// Values to set the brightness bias of the camera
#[derive(Debug, Clone, Default)]
#[repr(u8)]
pub enum BrightnessLevel {
    #[default]
    Default = 0x00,
    PlusOne = 0x01,
    MinusOne = 0x02,
    PlusTwo = 0x03,
    MinusTwo = 0x04,
    PlusThree = 0x05,
    MinusThree = 0x06,
    PlusFour = 0x07,
    MinusFour = 0x08,
}

#[derive(Debug, Clone)]
#[repr(u8)]
enum PowerMode {
    LowPower = 0x07,
    Normal = 0x05,
}

#[derive(Debug, Clone)]
#[repr(u8)]
enum ControlValue {
    Disable = 0x00,
    Enable = 0x80,
}

/// The format in which the camera data should be formatted
///
/// You will most likely want to use `Jpeg`, as `Rgb` generates images that are much greater in
/// size than `Jpeg`, and could therefore data blobs that are too big for your MCU to handle.
#[derive(Debug, Clone, Default)]
#[repr(u8)]
pub enum Format {
    #[default]
    /// Default. Offers a good mix between image quality and data size
    Jpeg = 0x01,
    /// Untested
    Rgb = 0x02,
    /// Untested
    Yuv = 0x03,
}

/// The resolution of the image captured by the camera
///
/// `Qvga` and `Vga` are good test values. `Qqvga` is listed in the SDK, however is also not
/// explicitly added to the camera's supported resolutions. Based on testing with a 5MP camera,
/// some resolutions have been feature-gated as only working on 3MP as they did not work on 5MP.
/// However, this hasn't been tested yet.
///
/// If both the `3mp` and `5mp` features are enabled, this enum will have a `Fhd` (1080p) default,
/// as that is the highest resolution compatible with both cameras. When only the `3mp` feature is
/// enabled, the enum will default to the camera's maximum supported resolution: `Qxga`. When only
/// the `5mp` feature is enabled, the enum will default to `Wqxga2`.
#[derive(Debug, Clone, Default)]
#[repr(u8)]
pub enum Resolution {
    #[cfg_attr(docsrs, doc(cfg(feature = "3mp")))]
    #[cfg(feature = "3mp")]
    /// QQVGA resolution (160x120). Tested to not work on 5MP. Untested on 3MP.
    Qqvga = 0x00,

    /// QVGA resolution (320x240). Untested on 3MP.
    Qvga = 0x01,
    /// VGA resolution (640x480). Untested on 3MP.
    Vga = 0x02,

    #[cfg_attr(docsrs, doc(cfg(feature = "3mp")))]
    #[cfg(feature = "3mp")]
    /// SVGA resolution (800x600). Tested to not work on 5MP. Untested on 3MP.
    Svga = 0x03,

    /// HD resolution (1280x720). Untested on 3MP.
    Hd = 0x04,

    #[cfg_attr(docsrs, doc(cfg(feature = "3mp")))]
    #[cfg(feature = "3mp")]
    /// SXGAM resolution (1280x960). Tested to not work on 5MP. Untested on 3MP.
    Sxgam = 0x05,

    /// UXGA resolution (1600x1200). Untested on 3MP.
    Uxga = 0x06,

    #[cfg_attr(all(feature = "3mp", feature = "5mp"), default)]
    /// FHD resolution (1920x1080). This is the default choice for this enum when both the `3mp`
    /// and `5mp` features are enabled. Untested on 3MP.
    Fhd = 0x07,

    #[cfg_attr(docsrs, doc(cfg(feature = "3mp")))]
    #[cfg(feature = "3mp")]
    #[cfg_attr(not(feature = "5mp"), default)]
    /// QXGA resolution (2048x1536). This is the default choice for this enum when the `3mp`
    /// feature (only) is enabled. Untested.
    Qxga = 0x08,

    #[cfg_attr(docsrs, doc(cfg(feature = "5mp")))]
    #[cfg(feature = "5mp")]
    #[cfg_attr(not(feature = "3mp"), default)]
    /// WQXGA2 resolution (2592x1944). This is the default choice for this enum when the
    /// `5mp` feature (only) is enabled.
    Wqxga2 = 0x09,

    /// 96x96 resolution. Untested on 3MP.
    Res96x96 = 0x0a,
    /// 128x128 resolution. Untested on 3MP.
    Res128x128 = 0x0b,
    /// 320x320 resolution. Untested on 3MP.
    Res320x320 = 0x0c,
}

#[derive(Debug, Clone)]
#[repr(u8)]
enum ArduchipCommand {
    Clear = 0x01,
    Start = 0x02,
}

/// The white balance mode the camera should use
///
/// Whether you can use a static value or not will depend on the conditions in which your camera
/// operates. For most consistent results, it is recommended to use a specific mode, which will
/// prevent the camera from randomly switching from one mode to another.
#[derive(Debug, Clone, Default)]
#[repr(u8)]
pub enum WhiteBalanceMode {
    #[default]
    Auto = 0x00,
    Sunny = 0x01,
    Office = 0x02,
    Cloudy = 0x03,
    Home = 0x04,
}

/// The main ArducamMega struct
///
/// This struct is the driver's main entrypoint. By providing it with a configured SPI device and
/// Delay implementation, this driver will be able to configure the Arducam Mega camera, take
/// pictures, and read the picture data back from the camera.
///
/// # Examples
///
/// The following example shows how the camera could be used on an ESP32 using the SPI3 device.
///
/// ```ignore
/// let peripherals = Peripherals::take();
/// let clocks = ClockControl::configure(system.clock_control, CpuClock::Clock240MHz).freeze();
/// let delay = Delay::new(&clocks);
///
/// let io = IO::new(peripherals.GPIO, peripherals.IO_MUX);
/// let sclk = io.pins.gpio18;
/// let miso = io.pins.gpio19;
/// let mosi = io.pins.gpio23;
/// let cs = io.pins.gpio5;
///
/// let spi_controller = SpiBusController::from_spi(Spi::new_no_cs(
///     peripherals.SPI3,
///     sclk,
///     mosi,
///     miso,
///     8u32.MHz(),
///     SpiMode::Mode0,
///     &mut system.peripheral_clock_control,
///     &clocks,
/// ));
///
/// let spi_device_1 = spi_controller.add_device(cs);
///
/// let mut cam = ArducamMega::new(spi_device_1, delay);
///
/// cam.reset()?
///     .set_format(Format::Jpeg)?
///     .set_resolution(Resolution::Hd)?
///     .set_white_balance_mode(WhiteBalanceMode::Home)?;
///
/// let length = cam
///     .capture()?
///     .read_fifo_length()?;
///
/// // assuming buf.len() == length
/// cam.read_fifo_full(buf)?;
/// ```
pub struct ArducamMega<SPI, Delay> {
    spi: SPI,
    delay: Delay,
}

impl<SPI, Delay> ArducamMega<SPI, Delay>
where
    SPI: SpiDevice,
    SPI::Bus: SpiBus,
    Delay: DelayUs,
{
    pub fn new(spi: SPI, delay: Delay) -> Self {
        Self { spi, delay }
    }

    fn read_reg(&mut self, addr: RegisterAddress) -> Result<u8, Error<SPI::Error, Delay::Error>> {
        let out: [u8; 1] = [addr as u8];
        let mut buf = [0; 3];

        self.spi
            .transfer(&mut buf[..], &out[..])
            .map_err(Error::Spi)?;

        Ok(buf[2])
    }

    fn write_reg(
        &mut self,
        addr: RegisterAddress,
        value: u8,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        let out: [u8; 2] = [addr as u8 | WRITE_REGISTER_MASK, value];
        self.spi.write(&out[..]).map_err(Error::Spi)?;

        Ok(self)
    }

    fn wait_idle(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        while (self.read_reg(RegisterAddress::SensorState)? & SENSOR_STATE_MASK)
            != SENSOR_STATE_IDLE
        {
            self.delay.delay_us(500u32).map_err(Error::Delay)?;
        }

        Ok(self)
    }

    /// Resets the camera sensor sensor
    ///
    /// This command sends a reset command to the camera. The Arducam SDK uses this after
    /// initialisation to ensure that the sensor is in a known-good state.
    pub fn reset(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::SensorReset, SENSOR_RESET_ENABLE)?;
        self.wait_idle()
    }

    /// Reads the camera model from the camera
    ///
    /// This function uses the `SensorId` register in the camera to obtain information about the
    /// sensor type. See [`CameraType`](CameraType) for more information.
    pub fn get_camera_type(&mut self) -> Result<CameraType, Error<SPI::Error, Delay::Error>> {
        let id = self.read_reg(RegisterAddress::SensorId)?;

        Ok(id.into())
    }

    /// Sets the auto-focus of the camera
    ///
    /// It is not clear how this feature should be used. As of current testing, only sending `0x00`
    /// actually produces successful captures. Sending `0x01` makes the camera produce invalid
    /// image data. More testing welcome.
    #[cfg_attr(docsrs, doc(cfg(feature = "5mp")))]
    #[cfg(feature = "5mp")]
    pub fn set_auto_focus(
        &mut self,
        value: u8,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::AutoFocus, value)?;
        self.wait_idle()
    }

    /// Sets the capture format of the camera
    ///
    /// This function allows you to control what format the camera captures pictures in.
    /// `Format::Jpeg` provides a good mix between image size and quality, and is the
    /// default.
    pub fn set_format(
        &mut self,
        format: Format,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::Format, format as u8)?;
        self.wait_idle()
    }

    /// Sets the capture resolution of the camera
    ///
    /// This function allows you to control the resolution of the pictures that are captured by the
    /// camera. Both the 3MP and 5MP cameras have two different default resolutions. See
    /// [`Resolution`](Resolution) for more details.
    pub fn set_resolution(
        &mut self,
        resolution: Resolution,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(
            RegisterAddress::Resolution,
            resolution as u8 | WRITE_REGISTER_MASK,
        )?;
        self.wait_idle()
    }

    /// Sets the camera's debug device address
    ///
    /// The Arducam SDK uses this command as part of the camera initialisation with the address
    /// `0x78`, however this does not appear to be necessary for the camera to function properly.
    pub fn set_debug_device_address(
        &mut self,
        addr: u8,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::DebugDeviceAddress, addr)?;
        self.wait_idle()
    }

    /// Empties the camera's FIFO buffer
    ///
    /// This command empties the contents of the camera's FIFO buffer. This is used as part of the
    /// [`capture()`](ArducamMega::capture) function.
    fn clear_fifo(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::ArduchipFifo, ArduchipCommand::Clear as u8)
    }

    /// Checks whether the sensor has finished capturing an image
    ///
    /// This command reads a register in the camera to check if the sensor has finished taking a
    /// picture. You should not attempt to read the FIFO buffer length nor read any FIFO buffer
    /// data prior to this returning `true`.
    pub fn capture_finished(&mut self) -> Result<bool, Error<SPI::Error, Delay::Error>> {
        let sensor_state = self.read_reg(RegisterAddress::SensorState)?;
        Ok((sensor_state & CAPTURE_FINISHED_MASK) != 0)
    }

    /// Takes a picture using the currently-configured settings
    ///
    /// This command starts by emptying the camera's FIFO buffer and subsequently tells the sensor
    /// to capture an image. The image will be captured using the currently-configured settings
    /// (white balance, gain, exposure, colour filters, etc). This command blocks until the sensor
    /// has finished capturing the image.
    pub fn capture(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.capture_noblock()?;

        while !self.capture_finished()? {
            self.delay.delay_us(500u32).map_err(Error::Delay)?;
        }

        Ok(self)
    }

    /// Non-blocking version of [`capture()`](ArducamMega::capture)
    pub fn capture_noblock(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.clear_fifo()?;
        self.write_reg(RegisterAddress::ArduchipFifo, ArduchipCommand::Start as u8)
    }

    /// Reads the size of the camera's FIFO buffer length
    ///
    /// This function reads out the size of the FIFO buffer length. This (roughly) represents the
    /// size of the picture. It appears that in some cases, the reported buffer is larger than the
    /// actual picture data.
    pub fn read_fifo_length(&mut self) -> Result<usize, Error<SPI::Error, Delay::Error>> {
        let size1 = self.read_reg(RegisterAddress::FifoSize1)? as usize;
        let size2 = self.read_reg(RegisterAddress::FifoSize2)? as usize;
        let size3 = self.read_reg(RegisterAddress::FifoSize3)? as usize;

        let length = (size3 << 16) | (size2 << 8) | size1;

        Ok(length)
    }

    /// Reads a single byte out of the FIFO buffer
    ///
    /// Returns the first byte out of the FIFO buffer. After it has been read, the FIFO buffer will
    /// advance and the byte will be gone from the camera. In theory, calling this function
    /// [`fifo_length`](ArducamMega::read_fifo_length) times should provide you with the full
    /// picture data.
    ///
    /// Reading out the entire image data like this will be relatively slow, as each byte transfer
    /// will require an SPI transaction to be setup and ended. For faster transfers, please see
    /// [`read_fifo_full`](ArducamMega::read_fifo_full).
    pub fn read_fifo_byte(&mut self) -> Result<u8, Error<SPI::Error, Delay::Error>> {
        let output: [u8; 1] = [FIFO_READ_SINGLE];
        let mut data: [u8; 3] = [0; 3];
        self.spi
            .transfer(&mut data[..], &output[..])
            .map_err(Error::Spi)?;

        Ok(data[2])
    }

    /// Reads out the entire FIFO buffer
    ///
    /// Reads out the camera's entire FIFO buffer into `data`. `data` must be large enough to
    /// accomodate the entire data transfer (as indicated by
    /// [`read_fifo_length()`](ArducamMega::read_fifo_length)), or some data loss may occur. The
    /// data is currently read in chunks of 63 bytes, as this appears to be how the camera responds
    /// to burst read commands (however, this may be a side-effect of the ESP32's SPI controller
    /// used for testing). If the provided `data` buffer is longer than the FIFO contents, the
    /// final bytes will be `0x00`.
    ///
    /// **Please note**: It appears the length reported by the camera through
    /// [`read_fifo_length()`](ArducamMega::read_fifo_length) for JPEG pictures is bigger than the
    /// actual picture. Previous versions of this function made attempts at finding the EOF marker
    /// in the JPEG stream, however this is no longer the case. Please see
    /// [`find_jpeg_eof()`](find_jpeg_eof) to help trim the data stream.
    ///
    /// This function is not currently tested with other data formats (YUV, RGB).
    pub fn read_fifo_full<T>(
        &mut self,
        data: &mut T,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>>
    where
        T: AsMut<[u8]>,
    {
        let data = data.as_mut();
        let length = data.len();
        let output: [u8; 1] = [FIFO_READ_BURST];
        let mut buffer: [u8; 65] = [0; 65];

        let mut i = 0;

        self.spi
            .transaction(|bus| {
                // there are more than 63 bytes to be read
                while i + 63 < length {
                    // Send the read burst command and read 65 bytes
                    bus.transfer(&mut buffer, &output)?;
                    // Copy buffer contents into data array, skipping first two bytes
                    data[i..i + 63].copy_from_slice(&buffer[2..]);
                    i += 63;
                }

                // Send another read burst command and read remaining bytes
                bus.transfer(&mut buffer[..(length - i) + 2], &output)?;
                // Copy buffer contents into data array, skipping first two bytes
                data[i..].copy_from_slice(&buffer[2..(length - i) + 2]);

                Ok(())
            })
            .map_err(Error::Spi)?;

        Ok(self)
    }

    fn set_auto_camera_control(
        &mut self,
        cc: CameraControl,
        value: ControlValue,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(
            RegisterAddress::GainExposureWhiteBalance,
            cc as u8 | value as u8,
        )?;
        self.wait_idle()
    }

    /// Enables the camera's auto white balance
    ///
    /// **Note**: This appears to not work on the ArducamMega 5MP, where pictures are severely
    /// green-tinted when using AWB.
    #[inline]
    pub fn enable_auto_white_balance(
        &mut self,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_auto_camera_control(CameraControl::WhiteBalance, ControlValue::Enable)
    }

    /// Disables the camera's auto white balance
    ///
    /// This function is automatically called by
    /// [`set_white_balance_mode()`](ArducamMega::set_white_balance_mode).
    #[inline]
    pub fn disable_auto_white_balance(
        &mut self,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_auto_camera_control(CameraControl::WhiteBalance, ControlValue::Disable)
    }

    /// Enables the camera's automatic gain adjustment
    #[inline]
    pub fn enable_auto_iso(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_auto_camera_control(CameraControl::Gain, ControlValue::Enable)
    }

    /// Disables the camera's automatic gain adjustment
    #[inline]
    pub fn disable_auto_iso(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_auto_camera_control(CameraControl::Gain, ControlValue::Disable)
    }

    /// Enables the camera's automatic exposure control
    #[inline]
    pub fn enable_auto_exposure(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_auto_camera_control(CameraControl::Exposure, ControlValue::Enable)
    }

    /// Disables the camera's automatic exposure control
    #[inline]
    pub fn disable_auto_exposure(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_auto_camera_control(CameraControl::Exposure, ControlValue::Disable)
    }

    /// Fixes the white balance of the camera to `mode`
    ///
    /// This will set the white balance mode to the fixed value described by `mode`. The `auto`
    /// mode has not been tested yet, and is copied straight from the SDK provided by Arducam.
    /// This function ensures that auto white balance is disabled in the sensor by calling
    /// [`disable_auto_white_balance()`](ArducamMega::disable_auto_white_balance) first.
    pub fn set_white_balance_mode(
        &mut self,
        mode: WhiteBalanceMode,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.disable_auto_white_balance()?;
        self.write_reg(RegisterAddress::WhiteBalanceMode, mode as u8)?;
        self.wait_idle()
    }

    #[inline]
    fn set_power_mode(
        &mut self,
        mode: PowerMode,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::Power, mode as u8)
    }

    /// Turns on the camera's low power mode
    #[inline]
    pub fn enable_low_power_mode(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_power_mode(PowerMode::LowPower)
    }

    /// Turns off the camera's low power mode
    #[inline]
    pub fn disable_low_power_mode(&mut self) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.set_power_mode(PowerMode::Normal)
    }

    /// Sets the camera's brightness bias
    pub fn set_brightness_bias(
        &mut self,
        level: BrightnessLevel,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::Brightness, level as u8)?;
        self.wait_idle()
    }

    /// Sets the camera's contrast
    pub fn set_contrast(
        &mut self,
        level: Level,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::Contrast, level as u8)?;
        self.wait_idle()
    }

    /// Sets the camera's saturation
    pub fn set_saturation(
        &mut self,
        level: Level,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::Saturation, level as u8)?;
        self.wait_idle()
    }

    /// Sets the camera's exposure
    pub fn set_exposure(
        &mut self,
        level: Level,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::Exposure, level as u8)?;
        self.wait_idle()
    }

    /// Sets the camera's color effect
    pub fn set_color_effect(
        &mut self,
        effect: ColorEffect,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::ColorEffect, effect as u8)?;
        self.wait_idle()
    }

    /// Sets the camera's sharpness
    #[cfg_attr(docsrs, doc(cfg(feature = "3mp")))]
    #[cfg(feature = "3mp")]
    pub fn set_sharpness(
        &mut self,
        level: SharpnessLevel,
    ) -> Result<&mut Self, Error<SPI::Error, Delay::Error>> {
        self.write_reg(RegisterAddress::Sharpness, level as u8)?;
        self.wait_idle()
    }
}

#[derive(Copy, Clone, Debug)]
pub enum Error<SPI, Delay> {
    Spi(SPI),
    Delay(Delay),
}

/// Locates the JPEG end-of-file marker in a data array
///
/// It appears that the Arducam Mega over-reports the size of the FIFO buffer length. This means
/// that in some cases, you can end up with a bunch of `0x00` bytes at the end of your JPEG file.
/// This function can be used to detect the end-of-file marker in the JPEG data and thus trim the
/// file. This function would typically be used after retrieving data using
/// [`read_fifo_full`](ArducamMega::read_fifo_full).
///
/// # Examples
///
/// ```
/// use arducam_mega::find_jpeg_eof;
///
/// //                                  v- extra bytes
/// let data = [0xff, 0xff, 0xff, 0xd9, 0x00, 0x00, 0x00];
/// //                      ^- end of file marker
///
/// let eof = find_jpeg_eof(&data).unwrap();
/// assert_eq!(data[..eof].len(), 4);
/// ```
pub fn find_jpeg_eof(data: &[u8]) -> Option<usize> {
    data.windows(2)
        .enumerate()
        .filter_map(|(i, p)| match p {
            [0xff, 0xd9] => Some(i + 2),
            _ => None,
        })
        .last()
}

#[cfg(test)]
mod tests {
    use super::*;
    use embedded_hal_mock::{
        delay,
        spi::{self, Transaction},
    };

    #[test]
    fn find_jpeg_eof_finds_first_byte() {
        assert_eq!(find_jpeg_eof(&[0xff, 0xd9]).unwrap(), 2);
    }

    #[test]
    fn find_jpeg_eof_finds_not_2_aligned_byte() {
        assert_eq!(find_jpeg_eof(&[0x00, 0xff, 0xd9]).unwrap(), 3);
    }

    #[test]
    fn find_jpeg_eof_returns_last_match() {
        assert_eq!(find_jpeg_eof(&[0xff, 0xd9, 0xff, 0xd9]).unwrap(), 4);
    }

    #[test]
    fn find_jpeg_eof_identifies_missing_tag() {
        assert!(find_jpeg_eof(&[]).is_none());
    }

    macro_rules! harness {
        ($e:ident, $s:ident, $c:ident) => {
            let mut $s = spi::Mock::new(&$e);
            let mut $c = ArducamMega::new(&mut $s, delay::MockNoop::new());
        };
    }

    macro_rules! expect {
        (send $send:expr, receive $recv:expr) => {
            [
                Transaction::transaction_start(),
                Transaction::transfer($send, $recv),
                Transaction::transaction_end(),
            ]
        };

        (send $send:expr, receive $recv:expr, wait_idle) => {
            [
                Transaction::transaction_start(),
                Transaction::transfer($send, $recv),
                Transaction::transaction_end(),
                Transaction::transaction_start(),
                Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x02]),
                Transaction::transaction_end(),
            ]
        };

        (send $send:expr) => {
            [
                Transaction::transaction_start(),
                Transaction::write_vec($send),
                Transaction::transaction_end(),
            ]
        };

        (send $send:expr, wait_idle) => {
            [
                Transaction::transaction_start(),
                Transaction::write_vec($send),
                Transaction::transaction_end(),
                Transaction::transaction_start(),
                Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x02]),
                Transaction::transaction_end(),
            ]
        };
    }

    #[test]
    fn read_reg_returns_third_byte() {
        let expectations = expect!(send vec![0x45], receive vec![0x00, 0x00, 0x01]);
        harness!(expectations, spi, cam);
        assert_eq!(cam.read_reg(RegisterAddress::FifoSize1).unwrap(), 0x01);
        spi.done();
    }

    #[test]
    fn write_reg_transforms_addr() {
        let expectations = expect!(send vec![0xc5, 0x02]);
        harness!(expectations, spi, cam);
        cam.write_reg(RegisterAddress::FifoSize1, 0x02).unwrap();
        spi.done();
    }

    #[test]
    fn reset_sends_correct_data() {
        let expectations = expect!(send vec![0x87, 0x40], wait_idle);
        harness!(expectations, spi, cam);
        cam.reset().unwrap();
        spi.done();
    }

    #[test]
    fn wait_idle_returns_immediately_if_idle() {
        let expectations = expect!(send vec![0x44], receive vec![0x00, 0x00, 0x02]);
        harness!(expectations, spi, cam);
        cam.wait_idle().unwrap();
        spi.done();
    }

    #[test]
    fn wait_idle_waits_until_idle() {
        let expectations = [
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x01]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x01]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x01]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x01, 0x01, 0x02]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        cam.wait_idle().unwrap();
        spi.done();
    }

    #[test]
    fn get_camera_type_reads_a_register_and_returns_an_enum() {
        let expectations = [
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x40], vec![0x00, 0x00, 0x81]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x40], vec![0x00, 0x00, 0x82]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x40], vec![0x00, 0x00, 0x33]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        assert_eq!(cam.get_camera_type().unwrap(), CameraType::OV5640);
        assert_eq!(cam.get_camera_type().unwrap(), CameraType::OV3640);
        assert_eq!(cam.get_camera_type().unwrap(), CameraType::Unknown(0x33));
        spi.done();
    }

    #[cfg(feature = "5mp")]
    #[test]
    fn set_auto_focus_writes_a_register() {
        let expectations = expect!(send vec![0xa9, 0x33], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_auto_focus(0x33).unwrap();
        spi.done();
    }

    #[test]
    fn set_format_writes_a_register() {
        let expectations = expect!(send vec![0xa0, 0x01], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_format(Format::default()).unwrap();
        spi.done();
    }

    #[test]
    fn set_resolution_writes_a_register() {
        let expectations = expect!(send vec![0xa1, 0x84], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_resolution(Resolution::Hd).unwrap();
        spi.done();
    }

    #[test]
    fn set_debug_device_address_writes_a_register() {
        let expectations = expect!(send vec![0x8a, 0x33], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_debug_device_address(0x33).unwrap();
        spi.done();
    }

    #[test]
    fn clear_fifo_writes_a_register() {
        let expectations = expect!(send vec![0x84, 0x01]);
        harness!(expectations, spi, cam);
        cam.clear_fifo().unwrap();
        spi.done();
    }

    #[test]
    fn capture_finished_detects_unfinished() {
        let expectations = expect!(send vec![0x44], receive vec![0x00, 0x00, 0xfb]);
        harness!(expectations, spi, cam);
        assert!(!cam.capture_finished().unwrap());
        spi.done();
    }

    #[test]
    fn capture_finished_detects_finished() {
        let expectations = expect!(send vec![0x44], receive vec![0x00, 0x00, 0x04]);
        harness!(expectations, spi, cam);
        assert!(cam.capture_finished().unwrap());
        spi.done();
    }

    #[test]
    fn capture_no_block_clears_fifo_and_writes_reg() {
        let expectations = [
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0x84, 0x01]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0x84, 0x02]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        cam.capture_noblock().unwrap();
        spi.done();
    }

    #[test]
    fn capture_returns_instantly_after_capture_finishes() {
        let expectations = [
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0x84, 0x01]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0x84, 0x02]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x04]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        cam.capture().unwrap();
        spi.done();
    }

    #[test]
    fn capture_blocks_until_capture_finished() {
        let expectations = [
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0x84, 0x01]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0x84, 0x02]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x00]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x00]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x00]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x04]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        cam.capture().unwrap();
        spi.done();
    }

    #[test]
    fn read_fifo_length_reads_three_regs() {
        let expectations = [
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x45], vec![0x00, 0x00, 0xab]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x46], vec![0x00, 0x00, 0xbc]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x47], vec![0x00, 0x00, 0xcd]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        assert_eq!(cam.read_fifo_length().unwrap(), 13483179);
        spi.done();
    }

    #[test]
    fn read_fifo_byte_reads_a_register() {
        let expectations = expect!(send vec![0x3d], receive vec![0x00, 0x00, 0x33]);
        harness!(expectations, spi, cam);
        assert_eq!(cam.read_fifo_byte().unwrap(), 0x33);
        spi.done();
    }

    #[test]
    fn read_fifo_full_skips_first_two_bytes() {
        let mut buffer = [0; 2];
        let expectations = expect!(send vec![0x3c], receive vec![0x00, 0x00, 0x33, 0x01]);
        harness!(expectations, spi, cam);
        cam.read_fifo_full(&mut buffer).unwrap();
        assert_eq!(buffer, [0x33, 0x01]);
        spi.done();
    }

    #[test]
    fn read_fifo_full_skips_first_two_bytes_on_long_transfers() {
        let mut buffer = [0; 126];
        let expectations = [
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x3c], vec![0x11; 65]),
            Transaction::transfer(vec![0x3c], vec![0x22; 65]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        cam.read_fifo_full(&mut buffer).unwrap();
        assert_eq!(
            buffer.iter().map(|i| *i as u32).sum::<u32>(),
            63 * 0x11 + 63 * 0x22
        );
        spi.done();
    }

    #[test]
    fn read_fifo_full_hands_partial_reads() {
        let mut buffer = [0; 100];
        let expectations = [
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x3c], vec![0x11; 65]),
            Transaction::transfer(vec![0x3c], vec![0x22; 39]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        cam.read_fifo_full(&mut buffer).unwrap();
        assert_eq!(
            buffer.iter().map(|i| *i as u32).sum::<u32>(),
            63 * 0x11 + 37 * 0x22
        );
        spi.done();
    }

    #[test]
    fn set_auto_camera_control_writes_a_register() {
        let expectations = expect!(send vec![0xaa, 0x81], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_auto_camera_control(CameraControl::Exposure, ControlValue::Enable)
            .unwrap();
        spi.done();
    }

    #[test]
    fn enable_auto_white_balance_writes_a_register() {
        let expectations = expect!(send vec![0xaa, 0x82], wait_idle);
        harness!(expectations, spi, cam);
        cam.enable_auto_white_balance().unwrap();
        spi.done();
    }

    #[test]
    fn disable_auto_white_balance_writes_a_register() {
        let expectations = expect!(send vec![0xaa, 0x02], wait_idle);
        harness!(expectations, spi, cam);
        cam.disable_auto_white_balance().unwrap();
        spi.done();
    }

    #[test]
    fn enable_auto_iso_writes_a_register() {
        let expectations = expect!(send vec![0xaa, 0x80], wait_idle);
        harness!(expectations, spi, cam);
        cam.enable_auto_iso().unwrap();
        spi.done();
    }

    #[test]
    fn disable_auto_iso_writes_a_register() {
        let expectations = expect!(send vec![0xaa, 0x00], wait_idle);
        harness!(expectations, spi, cam);
        cam.disable_auto_iso().unwrap();
        spi.done();
    }

    #[test]
    fn enable_auto_exposure_writes_a_register() {
        let expectations = expect!(send vec![0xaa, 0x81], wait_idle);
        harness!(expectations, spi, cam);
        cam.enable_auto_exposure().unwrap();
        spi.done();
    }

    #[test]
    fn disable_auto_exposure_writes_a_register() {
        let expectations = expect!(send vec![0xaa, 0x01], wait_idle);
        harness!(expectations, spi, cam);
        cam.disable_auto_exposure().unwrap();
        spi.done();
    }

    #[test]
    fn set_white_balance_mode_disables_autowb_and_writes_a_reg() {
        let expectations = [
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0xaa, 0x02]),
            Transaction::transaction_end(),
            // wait_idle
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x02]),
            Transaction::transaction_end(),
            Transaction::transaction_start(),
            Transaction::write_vec(vec![0xa6, 0x04]),
            Transaction::transaction_end(),
            // wait_idle
            Transaction::transaction_start(),
            Transaction::transfer(vec![0x44], vec![0x00, 0x00, 0x02]),
            Transaction::transaction_end(),
        ];
        harness!(expectations, spi, cam);
        cam.set_white_balance_mode(WhiteBalanceMode::Home).unwrap();
        spi.done();
    }

    #[test]
    fn enable_low_power_mode_writes_a_reg() {
        let expectations = expect!(send vec![0x82, 0x07]);
        harness!(expectations, spi, cam);
        cam.enable_low_power_mode().unwrap();
        spi.done();
    }

    #[test]
    fn disable_low_power_mode_writes_a_reg() {
        let expectations = expect!(send vec![0x82, 0x05]);
        harness!(expectations, spi, cam);
        cam.disable_low_power_mode().unwrap();
        spi.done();
    }

    #[test]
    fn set_brightness_bias_writes_a_reg() {
        let expectations = expect!(send vec![0xa2, 0x03], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_brightness_bias(BrightnessLevel::PlusTwo).unwrap();
        spi.done();
    }

    #[test]
    fn set_contrast_writes_a_reg() {
        let expectations = expect!(send vec![0xa3, 0x03], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_contrast(Level::PlusTwo).unwrap();
        spi.done();
    }

    #[test]
    fn set_saturation_writes_a_reg() {
        let expectations = expect!(send vec![0xa4, 0x03], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_saturation(Level::PlusTwo).unwrap();
        spi.done();
    }

    #[test]
    fn set_exposure_writes_a_reg() {
        let expectations = expect!(send vec![0xa5, 0x03], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_exposure(Level::PlusTwo).unwrap();
        spi.done();
    }

    #[test]
    fn set_color_effect_writes_a_reg() {
        let expectations = expect!(send vec![0xa7, 0x03], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_color_effect(ColorEffect::BlackWhite).unwrap();
        spi.done();
    }

    #[cfg(feature = "3mp")]
    #[test]
    fn set_sharpness_writes_a_reg() {
        let expectations = expect!(send vec![0xa8, 0x03], wait_idle);
        harness!(expectations, spi, cam);
        cam.set_sharpness(SharpnessLevel::Three).unwrap();
        spi.done();
    }
}