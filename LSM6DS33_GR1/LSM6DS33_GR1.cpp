#include "LSM6DS33_GR1.h"
//lib gr

LSM6DS33::LSM6DS33(PinName sda, PinName scl, uint8_t xgAddr) : i2c(sda, scl)
{
    // xgAddress will store the 7-bit I2C address, if using I2C.
    xgAddress = xgAddr;

// init gyro offset
    gx_off=0;
    gy_off=0;
    gz_off=0;
    gxol=0;
    gxoh=0;
    gyol=0;
    gyoh=0;
    gzol=0;
    gzoh=0;
}

uint16_t LSM6DS33::begin(gyro_scale gScl, accel_scale aScl,
                         gyro_odr gODR, accel_odr aODR)
{

    // Store the given scales in class variables. These scale variables
    // are used throughout to calculate the actual g's, DPS,and Gs's.
    gScale = gScl;
    aScale = aScl;

    // Once we have the scale values, we can calculate the resolution
    // of each sensor. That's what these functions are for. One for each sensor
    calcgRes(); // Calculate DPS / ADC tick, stored in gRes variable
    calcaRes(); // Calculate g / ADC tick, stored in aRes variable


    // To verify communication, we can read from the WHO_AM_I register of
    // each device. Store those in a variable so we can return them.
    // The start of the addresses we want to read from
    char cmd[2] = {
        WHO_AM_I_REG,
        0
    };

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, cmd, 1, true);
    // Read in all the 8 bits of data
    i2c.read(xgAddress, cmd+1, 1);
    uint8_t xgTest = cmd[1];                    // Read the accel/gyro WHO_AM_I

    // Gyro initialization stuff:
    initGyro(); // This will "turn on" the gyro. Setting up interrupts, etc.
    setGyroODR(gODR); // Set the gyro output data rate and bandwidth.
    setGyroScale(gScale); // Set the gyro range

    // Accelerometer initialization stuff:
    initAccel(); // "Turn on" all axes of the accel. Set up interrupts, etc.
    setAccelODR(aODR); // Set the accel data rate.
    setAccelScale(aScale); // Set the accel range.

    //set high res timestamp where LSB is 25us
    cmd[0] = WAKE_UP_DUR;
    cmd[1] = 0x10;
    i2c.write(xgAddress, cmd, 2);


    // Once everything is initialized, return the WHO_AM_I registers we read:
    return xgTest;
}

void LSM6DS33::initGyro()
{
    char cmd[2] = {
        CTRL2_G,
        static_cast<char>(gScale | G_ODR_104)
    };

    // Write the data to the gyro control registers
    i2c.write(xgAddress, cmd, 2);
}

void LSM6DS33::initAccel()
{
    char cmd[4] = {
        CTRL1_XL,
        0x30
    };

    // Write the data to the accel control registers
    i2c.write(xgAddress, cmd, 2);
}

void LSM6DS33::initIntr()
{

}
//modif gr1
void LSM6DS33::readAllraw()
{
    // The data we are going to read from the temp/gyr/acc/timestamp
    //char data[14];//from 0x20 to 0x42

    char data[14];
    char tsdata[3];

    i2c.start();
    i2c.write(xgAddress);
    i2c.write(OUT_TEMP_L);
    i2c.start();
    i2c.write(xgAddress | 0x1);
    for(int i =0; i<13; i++) {
        data[i]=i2c.read(1);
    }
    data[13]=i2c.read(0);
    i2c.stop();

    // Temperature is a 12-bit signed integer
    temperature_raw = data[0] | (data[1] << 8);
    gxl = data[2] ;
    gxh =data[3] ;
    gyl = data[4] ;
    gyh= data[5] ;
    gzl = data[6] ;
    gzh=data[7] ;
    axl= data[8] ;
    axh=data[9] ;
    ayl = data[10];
    ayh=data[11] ;
    azl = data[12] ;
    azh=data[13] ;

    //i2c.start();
    // i2c.write(xgAddress);
    // i2c.write(TIMESTAMP0_REG);
    // i2c.start();
    // i2c.write(xgAddress | 0x1);
    // for(int i =0; i<3; i++) {
    //     tsdata[i]=i2c.read(1);
    // }
    // tsdata[3]=i2c.read(0);
    //i2c.stop();

    // time_raw = tsdata[0] | (tsdata[1] << 8) | (tsdata[2] << 16);


    temperature_c = (float)temperature_raw / 16.0 + 25.0;
    // gx = gx_raw * gRes;
    //  gy = gy_raw * gRes;
    // gz = gz_raw * gRes;
    //  ax = ax_raw * aRes;
    // ay = ay_raw * aRes;
    // az = az_raw * aRes;
    //  time = time_raw*(0.000025);


}
//fin modif gr1
void LSM6DS33::readAll()
{
    // The data we are going to read from the temp/gyr/acc/timestamp
    //char data[14];//from 0x20 to 0x42

    char data[14];
    char tsdata[4];

    i2c.start();
    i2c.write(xgAddress);
    i2c.write(OUT_TEMP_L);
    i2c.start();
    i2c.write(xgAddress | 0x1);
    for(int i =0; i<13; i++) {
        data[i]=i2c.read(1);
    }
    data[13]=i2c.read(0);
    i2c.stop();

    // Temperature is a 12-bit signed integer
    temperature_raw = data[0] | (data[1] << 8);
    gx_raw = data[2] | (data[3] << 8);
    gy_raw = data[4] | (data[5] << 8);
    gz_raw = data[6] | (data[7] << 8);
    ax_raw = data[8] | (data[9] << 8);
    ay_raw = data[10] | (data[11] << 8);
    az_raw = data[12] | (data[13] << 8);

    i2c.start();
    i2c.write(xgAddress);
    i2c.write(TIMESTAMP0_REG);
    i2c.start();
    i2c.write(xgAddress | 0x1);
    for(int i =0; i<3; i++) {
        tsdata[i]=i2c.read(1);
    }
    tsdata[3]=i2c.read(0);
    i2c.stop();

    time_raw = tsdata[0] | (tsdata[1] << 8) | (tsdata[2] << 16);


    temperature_c = (float)temperature_raw / 16.0 + 25.0;
    gx = gx_raw * gRes;
    gy = gy_raw * gRes;
    gz = gz_raw * gRes;
    ax = ax_raw * aRes;
    ay = ay_raw * aRes;
    az = az_raw * aRes;
    time = time_raw*(0.000025);


}


void LSM6DS33::readAccel()
{
    // The data we are going to read from the accel
    char data[6];

    // Set addresses
    char subAddressXL = OUTX_L_XL;
    char subAddressXH = OUTX_H_XL;
    char subAddressYL = OUTY_L_XL;
    char subAddressYH = OUTY_H_XL;
    char subAddressZL = OUTZ_L_XL;
    char subAddressZH = OUTZ_H_XL;

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, &subAddressXL, 1, true);
    // Read in register containing the axes data and alocated to the correct index
    i2c.read(xgAddress, data, 1);

    i2c.write(xgAddress, &subAddressXH, 1, true);
    i2c.read(xgAddress, (data + 1), 1);
    i2c.write(xgAddress, &subAddressYL, 1, true);
    i2c.read(xgAddress, (data + 2), 1);
    i2c.write(xgAddress, &subAddressYH, 1, true);
    i2c.read(xgAddress, (data + 3), 1);
    i2c.write(xgAddress, &subAddressZL, 1, true);
    i2c.read(xgAddress, (data + 4), 1);
    i2c.write(xgAddress, &subAddressZH, 1, true);
    i2c.read(xgAddress, (data + 5), 1);

    // Reassemble the data and convert to g
    ax_raw = data[0] | (data[1] << 8);
    ay_raw = data[2] | (data[3] << 8);
    az_raw = data[4] | (data[5] << 8);
    ax = ax_raw * aRes;
    ay = ay_raw * aRes;
    az = az_raw * aRes;
    //gr
    axl= data[0] ;
    axh=data[1] ;
    ayl = data[2];
    ayh=data[3] ;
    azl = data[4] ;
    azh=data[5] ;
}

void LSM6DS33::readIntr()
{
    char data[1];
    char subAddress = TAP_SRC;

    i2c.write(xgAddress, &subAddress, 1, true);
    i2c.read(xgAddress, data, 1);

    intr = (float)data[0];
}

void LSM6DS33::readTemp()
{
    // The data we are going to read from the temp
    char data[2];

    // Set addresses
    char subAddressL = OUT_TEMP_L;
    char subAddressH = OUT_TEMP_H;

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, &subAddressL, 1, true);
    // Read in register containing the temperature data and alocated to the correct index
    i2c.read(xgAddress, data, 1);

    i2c.write(xgAddress, &subAddressH, 1, true);
    i2c.read(xgAddress, (data + 1), 1);

    // Temperature is a 12-bit signed integer
    temperature_raw = data[0] | (data[1] << 8);

    temperature_c = (float)temperature_raw / 16.0 + 25.0;
    temperature_f = temperature_c * 1.8 + 32.0;
}


void LSM6DS33::readGyro()
{
    // The data we are going to read from the gyro
    char data[6];

    // Set addresses
    char subAddressXL = OUTX_L_G;
    char subAddressXH = OUTX_H_G;
    char subAddressYL = OUTY_L_G;
    char subAddressYH = OUTY_H_G;
    char subAddressZL = OUTZ_L_G;
    char subAddressZH = OUTZ_H_G;

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, &subAddressXL, 1, true);
    // Read in register containing the axes data and alocated to the correct index
    i2c.read(xgAddress, data, 1);

    i2c.write(xgAddress, &subAddressXH, 1, true);
    i2c.read(xgAddress, (data + 1), 1);
    i2c.write(xgAddress, &subAddressYL, 1, true);
    i2c.read(xgAddress, (data + 2), 1);
    i2c.write(xgAddress, &subAddressYH, 1, true);
    i2c.read(xgAddress, (data + 3), 1);
    i2c.write(xgAddress, &subAddressZL, 1, true);
    i2c.read(xgAddress, (data + 4), 1);
    i2c.write(xgAddress, &subAddressZH, 1, true);
    i2c.read(xgAddress, (data + 5), 1);

    // Reassemble the data and convert to degrees/sec
    gx_raw = data[0] | (data[1] << 8);
    gy_raw = data[2] | (data[3] << 8);
    gz_raw = data[4] | (data[5] << 8);
    gx = gx_raw * gRes;
    gy = gy_raw * gRes;
    gz = gz_raw * gRes;
    // gr
    gxl = data[0] ;
    gxh =data[1] ;
    gyl = data[2] ;
    gyh= data[3] ;
    gzl = data[4] ;
    gzh=data[5] ;
}

void LSM6DS33::setGyroScale(gyro_scale gScl)
{
    // The start of the addresses we want to read from
    char cmd[2] = {
        CTRL2_G,
        0
    };

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, cmd, 1, true);
    // Read in all the 8 bits of data
    i2c.read(xgAddress, cmd+1, 1);

    // Then mask out the gyro scale bits:
    cmd[1] &= 0xFF^(0x7 << 1);  //// << 2 au lieu de 3
    // Then shift in our new scale bits:
    cmd[1] |= gScl << 1;    //// << 0 au lieu de 3

    // Write the gyroscale out to the gyro
    i2c.write(xgAddress, cmd, 2);

    // We've updated the sensor, but we also need to update our class variables
    // First update gScale:
    gScale = gScl;
    // Then calculate a new gRes, which relies on gScale being set correctly:
    calcgRes();
}

void LSM6DS33::setAccelScale(accel_scale aScl)
{
    // The start of the addresses we want to read from
    char cmd[2] = {
        CTRL1_XL,
        0
    };

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, cmd, 1, true);
    // Read in all the 8 bits of data
    i2c.read(xgAddress, cmd+1, 1);

    // Then mask out the accel scale bits:
    cmd[1] &= 0xFF^(0x3 << 2);  //// gr 2 au lieu de 3  mise a zero des bits 3 et 4
    // Then shift in our new scale bits:
    cmd[1] |= aScl << 2;   //// gr 2 au lieu de 3

    // Write the accelscale out to the accel
    i2c.write(xgAddress, cmd, 2);

    // We've updated the sensor, but we also need to update our class variables
    // First update aScale:
    aScale = aScl;
    // Then calculate a new aRes, which relies on aScale being set correctly:
    calcaRes();
}

void LSM6DS33::setGyroODR(gyro_odr gRate)
{
    // The start of the addresses we want to read from
    char cmd[2] = {
        CTRL2_G,
        0
    };

    // Set low power based on ODR, else keep sensor on high performance
    if(gRate == G_ODR_13_BW_0 | gRate == G_ODR_26_BW_2 | gRate == G_ODR_52_BW_16) {
        char cmdLow[2] = {
            CTRL7_G,
            1
        };

        i2c.write(xgAddress, cmdLow, 2);
    } else {
        char cmdLow[2] = {
            CTRL7_G,
            0
        };

        i2c.write(xgAddress, cmdLow, 2);
    }

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, cmd, 1, true);
    // Read in all the 8 bits of data
    i2c.read(xgAddress, cmd+1, 1);

    // Then mask out the gyro odr bits:
    cmd[1]  &= 0xFF^(0xF << 4);
    // Then shift in our new odr bits:
    cmd[1] |= gRate;

    // Write the gyroodr out to the gyro
    i2c.write(xgAddress, cmd, 2);
}

void LSM6DS33::setAccelODR(accel_odr aRate)
{
    // The start of the addresses we want to read from
    char cmd[2] = {
        CTRL1_XL,
        0
    };

    // Set low power based on ODR, else keep sensor on high performance
    if(aRate == A_ODR_13 | aRate == A_ODR_26 | aRate == A_ODR_52) {
        char cmdLow[2] = {
            CTRL6_C,
            1
        };

        i2c.write(xgAddress, cmdLow, 2);
    } else {
        char cmdLow[2] = {
            CTRL6_C,
            0
        };

        i2c.write(xgAddress, cmdLow, 2);
    }

    // Write the address we are going to read from and don't end the transaction
    i2c.write(xgAddress, cmd, 1, true);
    // Read in all the 8 bits of data
    i2c.read(xgAddress, cmd+1, 1);

    // Then mask out the accel odr bits:
    cmd[1] &= 0xFF^(0xF << 4);  // gr erreur ??
    // Then shift in our new odr bits:
    cmd[1] |= aRate << 4;     //  gr erreur

    // Write the accelodr out to the accel
    i2c.write(xgAddress, cmd, 2);
}

void LSM6DS33::calcgRes()
{
    // Possible gyro scales (and their register bit settings) are:
    // 125 DPS , 245 DPS (00), 500 DPS (01), 2000 DPS (10).
    switch (gScale) {
        case G_SCALE_125DPS:
            gRes = 125.0 / 32768.0;
            break;
        case G_SCALE_250DPS:
            gRes = 250.0 / 32768.0;
            break;
        case G_SCALE_500DPS:
            gRes = 500.0 / 32768.0;
            break;
        case G_SCALE_1000DPS:
            gRes = 1000.0 / 32768.0;
            break;
        case G_SCALE_2000DPS:
            gRes = 2000.0 / 32768.0;
            break;
    }
}

void LSM6DS33::calcaRes()
{
    // Possible accelerometer scales (and their register bit settings) are:
    // 2 g (000), 4g (001), 6g (010) 8g (011), 16g (100).
    switch (aScale) {
        case A_SCALE_2G:
            aRes = 2.0 / 32768.0;
            break;
        case A_SCALE_4G:
            aRes = 4.0 / 32768.0;
            break;
        case A_SCALE_8G:
            aRes = 8.0 / 32768.0;
            break;
        case A_SCALE_16G:
            aRes = 16.0 / 32768.0;
            break;
    }
}
void LSM6DS33::calibration( int16_t iter)
{
    int32_t gxoll=0,gyoll=0,gzoll=0;
    for(int ii=0; ii<iter; ii++) {
        this->readGyro();
        gx_off=gx_off+gx;
        gy_off=gy_off+gy;
        gz_off=gz_off+gz;
        //
        
        gxoll=gxoll+(int32_t)gx_raw;
        gyoll=gyoll+(int32_t)gy_raw;
        gzoll=gzoll+(int32_t)gz_raw;
        
        
        //wait(0.01);
    }
    gx_off=gx_off/iter;
    gy_off=gy_off/iter;
    gz_off=gz_off/iter;
    //
    gxoll=gxoll/iter;
    gyoll=gyoll/iter;
    gzoll=gzoll/iter;
    
    //
    gxol=(gxoll&0x00FF);
    gxoh=(gxoll>>8);
    gyol=(gyoll&0x00FF);
    gyoh=(gyoll>>8);
    gzol=(gzoll&0x00FF);
    gzoh=(gzoll>>8);
    
}

