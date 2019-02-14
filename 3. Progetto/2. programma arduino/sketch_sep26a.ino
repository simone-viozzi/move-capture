#include <Wire.h>

// accelerometro
class MPU_6050
{
  public:
    int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
    float normalized_AcX, normalized_AcY, normalized_AcZ, normalized_GyX, normalized_GyY, normalized_GyZ;
    float rescaled_AcX, rescaled_AcY, rescaled_AcZ, rescaled_GyX, rescaled_GyY, rescaled_GyZ;

    const uint8_t MPU = 0x68;  // I2C address of the MPU-6050
    
    MPU_6050()
    {
      Wire.begin();
      Wire.beginTransmission(MPU);
      Wire.write(0x6B);  // PWR_MGMT_1 register
      Wire.write(0);     // set to zero (wakes up the MPU-6050)
      Wire.endTransmission(true);
    }

    void get_data() 
    {
        Wire.beginTransmission(MPU);
        Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
        Wire.endTransmission(false);
        Wire.requestFrom(MPU,14,true);  // request a total of 14 registers
        AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
        AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
        AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
        Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
        GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
        GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
        GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    }

    float get_AcX() 
    {
      return (AcX*1.0);
    }
    
    float get_AcY() 
    {
      return (AcY*1.0);
    }
    
    float get_AcZ() 
    {
      return (AcZ*1.0);
    }

    float get_GyX() 
    {
      return (GyX*1.0);
    }
    
    float get_GyY() 
    {
      return (GyY*1.0);
    }
    
    float get_GyZ() 
    {
      return (GyZ*1.0);
    }

    float get_normalized_AcX() 
    {
      return normalized_AcX;
    }
    
    float get_normalized_AcY() 
    {
      return normalized_AcY;
    }
    
    float get_normalized_AcZ() 
    {
      return normalized_AcZ;
    }

    float get_normalized_GyX() 
    {
      return normalized_GyX;
    }
    
    float get_normalized_GyY() 
    {
      return normalized_GyY;
    }
    
    float get_normalized_GyZ() 
    {
      return normalized_GyZ;
    }

    float norma(float x, float y, float z)
    {
      float a = sqrt(x*x + y*y + z*z);
      return a;
    }

    float get_norma_Gy()
    {
      return norma(GyX, GyY, GyZ);
    }

    float get_norma_Ac()
    {
      return norma(AcX, AcY, AcZ);
    }

    float get_temp()
    {
      return Tmp;
    }

    void normalize_Ac(int Max)
    {
      float rescale;
      rescale = Max/get_norma_Ac();
      normalized_AcX = AcX*rescale;
      normalized_AcY = AcY*rescale;
      normalized_AcZ = AcZ*rescale;
    }

    void normalize_Gy(int Max)
    {
      float rescale;
      rescale = Max/get_norma_Gy();
      normalized_GyX = GyX*rescale;
      normalized_GyY = GyY*rescale;
      normalized_GyZ = GyZ*rescale;
    }

    void setting()
    {
      Wire.beginTransmission(MPU);
      Wire.write(0x1A);
      Wire.write(0x06);
      Wire.endTransmission();
      delay(50);
      Wire.beginTransmission(MPU);
      Wire.write(0x1B);
      //Wire.write(0x18);
      Wire.write(0x00);
      Wire.endTransmission();
      delay(50);
      Wire.beginTransmission(MPU);
      Wire.write(0x1C);
      Wire.write(0x04);
      Wire.endTransmission();
      delay(50);
      Wire.beginTransmission(MPU);
      Wire.write(0x6B);
      Wire.write(0x00);
      Wire.endTransmission();
      delay(50);
    }

    
};


// magnetometro
class QMC5883
{
  public:
    uint8_t add = 0x0D;
    int nowX = 0;
    int nowY = 0;
    int nowZ = 0;
    float rescaled_x = 0; 
    float rescaled_y = 0;
    float rescaled_z = 0;
    
    QMC5883()
    {
      Wire.begin();
      delay(50);
      Wire.beginTransmission(add);
      Wire.write(0x0B);
      Wire.write(0x01);
      Wire.endTransmission();
      delay(50);
      Wire.beginTransmission(add);
      Wire.write(0x09);
      Wire.write(0x0D);
      Wire.endTransmission();
      delay(50);
    }
    
    float getX() 
    {
      return (nowX*1.0);
    }
    
    float getY() 
    {
      return (nowY*1.0);
    }
    
    float getZ() 
    {
      return (nowZ*1.0);
    }

    float getX_rescaled() 
    {
      return rescaled_x;
    }
    
    float getY_rescaled() 
    {
      return rescaled_y;
    }
    
    float getZ_rescaled() 
    {
      return rescaled_z;
    }
  
    void get_data() 
    {
      Wire.beginTransmission(add);
      Wire.write(0x00);
      Wire.endTransmission();
      delay(50);
      Wire.requestFrom(add, 6);
      while(Wire.available()>0)
      {
        nowX = Wire.read();
        nowX |= Wire.read()<<8;
        nowY = Wire.read();
        nowY |= Wire.read()<<8;
        nowZ = Wire.read();
        nowZ |= Wire.read()<<8;
      }
    }

    float norma(float x, float y, float z)
    {
      float a = sqrt(x*x + y*y + z*z);
      return a;
    }

    float get_norma()
    {
      return norma(nowX, nowY, nowZ);
    }

    void normalize(int Max)
    {
      float n_norma = norma(nowX, nowY, nowZ);
      float rescale;
      rescale = Max/n_norma;
      rescaled_x = nowX*rescale;
      rescaled_y = nowY*rescale;
      rescaled_z = nowZ*rescale;
    }

    
  
};

class serial
{
  public:

    int n = 0;
  
    void sinc()
    {
      char a = ' ';
      do
      {
        Serial.readBytes(&a, 1);
      }
      while (a != 'k');
      digitalWrite(13, HIGH);
      a = 'y';
      Serial.write(&a, 1);
      
    }

    union Scomp_float
    {
      float f;
      int i;
      unsigned char byte_s[4];
    };

    void send_float(float n)
    {
      Scomp_float S_float;
      S_float.f = n;
      while (Serial.availableForWrite() < 4)
      {
        delay(10);
      }
      Serial.write(S_float.byte_s, 4);
    }

    float receive_float()
    {
      Scomp_float S_float;
      while (Serial.available() < 4)
      {
        delay(10);
      }
      Serial.readBytes(S_float.byte_s, 4);
      return S_float.f;
    }

    void send_char(char ch)
    {
      while (Serial.availableForWrite() < 1)
      {
        delay(10);
      }
      Serial.write(&ch, 1);
    }

    char receive_char()
    {
      char ch;
      while (Serial.available() < 1)
      {
        delay(10);
      }
      Serial.readBytes(&ch, 1);
      return ch;
    }

    void send_data(float* acc_xyz, float* g_xyz, float* magn, float temp)
    {
      send_char('a');
      for (int i = 0; i < 3; i++)
      {
        send_float(acc_xyz[i]);
      }
      send_char('g');
      for (int i = 0; i < 3; i++)
      {
        send_float(g_xyz[i]);
      }
      send_char('m');
      for (int i = 0; i < 3; i++)
      {
        send_float(magn[i]);
      }
      send_char('t');
      send_float(temp);
      send_char(4);
    }
    
    /*
    int receive_data(float* acc_xyz, float* g_xyz, float* magn)
    {
      if (receive_char() != 'a')
      {
        return 0;
      }
      for (int i = 0; i < 3; i++)
      {
        acc_xyz[i] = receive_float();
      }
      if (receive_char() != 'g')
      {
        return 0;
      }
      for (int i = 0; i < 3; i++)
      {
        g_xyz[i] = receive_float();
      }
      if (receive_char() != 'm')
      {
        return 0;
      }
      for (int i = 0; i < 3; i++)
      {
        magn[i] = receive_float();
      }
      if (receive_char() != 4)
      {
        return 0;
      }
    }
    */
    
};


void setup() 
{
  Serial.begin(115200);
}

void loop() 
{
  QMC5883 mm;

  MPU_6050 aa;

  aa.setting();
  
  digitalWrite(13, HIGH);
  
  while (!digitalRead(3))
  {
    delay(10);
  }
  
  serial ser;
  ser.sinc();

  digitalWrite(13, LOW);
  
  while (true)
  {
    mm.get_data();
    mm.normalize(100);
    
    aa.get_data();

    float acc_xyz[] = {aa.get_AcX(), aa.get_AcY(), aa.get_AcZ()};
    float g_xyz[] = {aa.get_GyX(), aa.get_GyY(), aa.get_GyZ()};
    float magn[] = {mm.getX(), mm.getY(), mm.getZ()};

    ser.send_data(acc_xyz, g_xyz, magn, aa.get_temp());
  }
}
