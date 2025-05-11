// Lib for HM0360 camera
// TODO: Eventually move this to its own module

#![cfg_attr(not(test), no_std)] // Apply no_std ONLY when NOT testing

#[repr(u16)]
enum NamedRegister {
    // Read only registers
    ModelIdH = 0x0000,
    ModelIdL = 0x0001,
    SiliconRev = 0x0002,
    FrameCountH = 0x0005,
    FrameCountL = 0x0006,
    PixelOrder = 0x0007,
    // Sensor mode control
    ModeSelect = 0x0100,
    ImgOrientation = 0x0101,
    EmbeddedLineEn = 0x0102,
    SwReset = 0x0103,
    CommandUpdate = 0x0104,
    // Sensor exposure gain control
    IntegrationH = 0x0202,
    IntegrationL = 0x0203,
    AnalogGain = 0x0205,
    DigitalGainH = 0x020E,
    DigitalGainL = 0x020F,
    // Clock control
    Pll1Config = 0x0300,
    Pll2Config = 0x0301,
    Pll3Config = 0x0302,
    // Frame timing control
    FrameLenLinesH = 0x0340,
    FrameLenLinesL = 0x0341,
    LineLenPckH = 0x0342,
    LineLenPckL = 0x0343,
    // Monochrome programming
    MonoMode = 0x0370,
    MonoModeIsp = 0x0371,
    MonoModeSel = 0x0372,
    // Binning mode control
    HSubsample = 0x0380,
    VSubsample = 0x0381,
    BinningMode = 0x0382,
    // Test pattern control
    TestPatternMode = 0x0601,
    // Black level control
    BlcTgt = 0x1004,
    Blc2Tgt = 0x1009,
    MonoCtrl = 0x100A,
    // VSYNC / HSYNC / pixel shift registers
    OpfmCtrl = 0x1014,
    // Tone mapping registers
    CmprsCtrl = 0x102F,
    Cmprs01 = 0x1030,
    Cmprs02 = 0x1031,
    Cmprs03 = 0x1032,
    Cmprs04 = 0x1033,
    Cmprs05 = 0x1034,
    Cmprs06 = 0x1035,
    Cmprs07 = 0x1036,
    Cmprs08 = 0x1037,
    Cmprs09 = 0x1038,
    Cmprs10 = 0x1039,
    Cmprs11 = 0x103A,
    Cmprs12 = 0x103B,
    Cmprs13 = 0x103C,
    Cmprs14 = 0x103D,
    Cmprs15 = 0x103E,
    Cmprs16 = 0x103F,
    // Automatic exposure control
    AeCtrl = 0x2000,
    AeCtrl1 = 0x2001,
    CntOrghH = 0x2002,
    CntOrghL = 0x2003,
    CntOrgvH = 0x2004,
    CntOrgvL = 0x2005,
    CntSthH = 0x2006,
    CntSthL = 0x2007,
    CntStvH = 0x2008,
    CntStvL = 0x2009,
    CtrlPgSkipcnt = 0x200A,
    BvWinWeightEn = 0x200D,
    MaxIntgH = 0x2029,
    MaxIntgL = 0x202A,
    MaxAgain = 0x202B,
    MaxDgainH = 0x202C,
    MaxDgainL = 0x202D,
    MinIntg = 0x202E,
    MinAgain = 0x202F,
    MinDgain = 0x2030,
    TDamping = 0x2031,
    NDamping = 0x2032,
    AlcTh = 0x2033,
    AeTargetMean = 0x2034,
    AeMinMean = 0x2035,
    AeTargetZone = 0x2036,
    ConvergeInTh = 0x2037,
    ConvergeOutTh = 0x2038,
    FsCtrl = 0x203B,
    Fs60HzH = 0x203C,
    Fs60HzL = 0x203D,
    Fs50HzH = 0x203E,
    Fs50HzL = 0x203F,
    FrameCntTh = 0x205B,
    AeMean = 0x205D,
    AeConverge = 0x2060,
    AeBliTgt = 0x2070,
    // Interrupt control
    PulseMode = 0x2061,
    PulseThH = 0x2062,
    PulseThL = 0x2063,
    IntIndic = 0x2064,
    IntClear = 0x2065,
    // Motion detection control
    MdCtrl = 0x2080,
    RoiStartEndV = 0x2081,
    RoiStartEndH = 0x2082,
    MdThMin = 0x2083,
    MdThStrL = 0x2084,
    MdThStrH = 0x2085,
    MdLightCoef = 0x2099,
    MdBlockNumTh = 0x209B,
    MdLatency = 0x209C,
    MdLatencyTh = 0x209D,
    MdCtrl1 = 0x209E,
    // Context switch control registers
    PmuCfg3 = 0x3024,
    PmuCfg4 = 0x3025,
    // Operation mode control
    WinMode = 0x3030,
    // IO and clock control
    PadRegister07 = 0x3112,
}

// Wraps a raw register address
#[derive(Copy, Clone)]
pub struct RegAddr(pub u16);

// Describe how to convert from NamedRegister to RegAddr
impl From<NamedRegister> for RegAddr {
    fn from(val: NamedRegister) -> Self {
        RegAddr(val as u16)
    }
}


// TODO: Totally halucinated values, need to double-check this and above named registers >.<
// Also, don't really need to do the below for all regs. Just set 0 or 1 to SwReset

use NamedRegister::*;
pub const HIMAX_DEFAULT_REGS: &[(RegAddr, u8)] = &[
    (SwReset.into(), 0x00),
    (MonoMode.into(), 0x00),
    (MonoModeIsp.into(), 0x01),
    (MonoModeSel.into(), 0x01),
    // BLC control
    (RegAddr(0x1000), 0x01),
    (RegAddr(0x1003), 0x04),
    (BlcTgt.into(), 0x04),
    (RegAddr(0x1007), 0x01),
    (RegAddr(0x1008), 0x04),
    (Blc2Tgt.into(), 0x04),
    (MonoCtrl.into(), 0x01),
    // Output format control
    (OpfmCtrl.into(), 0x0C),
    // Reserved regs
    (RegAddr(0x101D), 0x00),
    (RegAddr(0x101E), 0x01),
    (RegAddr(0x101F), 0x01),
    // Clock settings
    (PLL_CTRL, 0x04),
    (CLK_SEL1, 0x00),
    (CLK_SEL2, 0x20),
    // PCLK control
    (PCLK_POL, 0x01),
    // Crop window
    (RegAddr(0x2000), 0x02),
    (RegAddr(0x2001), 0x80), // X_START = 640
    (RegAddr(0x2002), 0x01),
    (RegAddr(0x2003), 0xE0), // Y_START = 480
    (RegAddr(0x2004), 0x05),
    (RegAddr(0x2005), 0x00), // X_END = 1280
    (RegAddr(0x2006), 0x03),
    (RegAddr(0x2007), 0xC0), // Y_END = 960
    // Scaling
    (RegAddr(0x2010), 0x01),
    (RegAddr(0x2011), 0x00), // SCALE_M = 256 (1x)
    (RegAddr(0x2012), 0x01),
    (RegAddr(0x2013), 0x00), // SCALE_N = 256 (1x)
    // Image orientation
    (ImgOrientation.into(), 0x00),
    // AE/AGC
    (AeCtrl.into(), 0x01),
    (AeTargetMean.into(), 0x3C),
    (AeMinMean.into(), 0x00),
    //(AeMaxMean.into(), 0xFF),
    // Frame rate
    (FLL, 0x08),
    (FLL + 1, 0x98),
    (FLL + 2, 0x00),
    (FLL + 3, 0x00),
    // Exposure
    (RegAddr(0x0202), 0x01),
    (RegAddr(0x0203), 0x00),
    // Gain
    (RegAddr(0x0205), 0x00),
    // Test pattern
    (RegAddr(0x0601), 0x00),
];

// Example function to test
pub fn add_one(x: u32) -> u32 {
    x.wrapping_add(1)
}

// This whole module only gets compiled when running `cargo test`
#[cfg(test)]
mod tests {
    // Import the items you want to test from the parent module
    use super::*;

    // Standard Rust test function
    #[test]
    fn test_add_one() {
        assert_eq!(add_one(5), 6);
        assert_eq!(add_one(0), 1);
        assert_eq!(add_one(u32::MAX), = 0); // Test wrapping behavior
    }
}
