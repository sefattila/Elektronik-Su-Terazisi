#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <EEPROM.h>

//LCD adresini ve konfigürasyonunu tanımladık.
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define switchPin 2     //Calibrasyon ve buzzer pin için
#define LED_2 3         //Led -2 
#define LED_1 4         //Led -1
#define LED_0 5         //Led 0
#define LED_P1 6        //Led +1
#define LED_P2 7        //Led +2
#define BUZZER 8        //Buzzer
#define HORIZONTAL 0    //Yatay için sabit
#define VERTICAL 1      //Dikey için sabit


///////////////////////
//Değişken tanımlama //
///////////////////////

float gyro_x, gyro_y, gyro_z;                 // MPU 6050 den alacağımız veri değişkenleri
float gyro_x_cal, gyro_y_cal, gyro_z_cal = 0; //gyro için ofsetler
float acc_x, acc_y, acc_z;                    //MPU 6050 için hızlanma ayarı
float acc_x_cal, acc_y_cal, acc_z_cal = 0;    //accelometre ayarları
float thetaM;                                 //theta ve  thetaM ilgili açıda döndürme
float theta;
float phiM;                                   //phi ve  phiM ilgili yükseklik
float phi; 
int LEDCount = 0;                             //Kalibre ederken, kurulum döngüsünde LED'lerin kolay yinelenmesini sağlar
long loop_timer;                              //Çok hızlı veya çok yavaş çalışmadığından emin olmak için ana döngü için zamanlayıcı
int temp;                                     //MPU6050'den kazanılan sıcaklık
int displaycount = 0;                         //Ana döngü her yürütüldüğünde, displaycount artar. Yalnızca her 100 sayımda bir ekran güncellenir
float dt;                                     //Zaman sabiti - eski ve yeni zaman arasındaki fark (açı hesaplamaları için)
unsigned long millisOld;                      //Eski zamanı saklamak için değişken
boolean orientation;                          //MPU6050 modülünün yönüne bağlı olarak YATAY veya DİKEY
int eeprom_address = 0;                       //Güncel eeprom adresini takip etmek için
int horizonalCalibration = 0;                 //0 kalibre edilmemişse, kalibre edilmişse 255
int verticalCalibration = 0;                  //0 kalibre edilmemişse, kalibre edilmişse 255
boolean horizonalCalibrationNotice = 0;       //Döngünün ilk yinelemesinde, kalibre edilmemişse 'Kalibrasyon yapılmadı' uyarısının görüntülenmesine izin verilir!
boolean verticalCalibrationNotice = 0;        //Döngünün ilk yinelemesinde, kalibre edilmemişse 'Kalibrasyon yapılmadı' uyarısının görüntülenmesine izin verilir!
boolean playBuzzer = 0;

//////////////////
// Ana kurulum //
/////////////////

void setup() 
{

  //Assigning all the pins as required.
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_0, OUTPUT);
  pinMode(LED_P1, OUTPUT);
  pinMode(LED_P2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  // Seri Port                                                
  Serial.begin(115200);

  //I2C
  Wire.begin();
  
  //LCD
  lcd.begin(16,2);//lcd.init(); //lcd.begin()
  lcd.print("Setting up"); 
  //MPU-6050'nin kayıtlarını ayarlayalım                                                  
  setup_mpu_6050_registers(); 

  delay(500);   //Kurulum bildirimi için.

  // Başlatma zamanlayıcısı
  loop_timer = micros();

  //SwitchPin düşük (Basılmış) ise, kalibrasyon modu başlar. Bu, 5 saniyenin yerleşmesine izin verir, ardından mevcut yönelimi 0'a kalibre eder.
  if(digitalRead(switchPin)==0)
  {
    delay(500);
    lcd.clear();
    lcd.print("Calibrating in ");
    for(int i = 5; i > 0; i--)
    {
      lcd.setCursor(15,0);
      lcd.print(i);
      LEDCount = i + 2;
      digitalWrite(LEDCount, HIGH);
      delay(1000);
      digitalWrite(LEDCount, LOW);
    }
    lcd.setCursor(11,0);
    lcd.print("-----");
    delay(500);
    read_mpu_6050_data();         //Yönü belirlemek için önce MPU6050'den gelen verilere ihtiyacımız var.
    if(acc_x < acc_z)             //Yatay ise acc_x acc_z'den küçük olacak, dikey ise tersine dönecek (yerçekimi etkisinden dolayı)
    {
      lcd.setCursor(0,1);
      lcd.print("Horizontal!");
      orientation = HORIZONTAL;
      //Read the raw acc and gyro data from the MPU-6050 1000 times                                          
      for (int cal_int = 0; cal_int < 1000 ; cal_int ++)
      {                  
        read_mpu_6050_data();                                         
        gyro_x_cal += gyro_x;                                            
        gyro_y_cal += gyro_y;                                              
        gyro_z_cal += gyro_z;                                           
        acc_x_cal += acc_x;                                          
        acc_y_cal += acc_y;                                                          
      }
      // Ortalama ofset elde etmek için tüm sonuçları 1000'e bölün
      gyro_x_cal /= 1000.0;                                                 
      gyro_y_cal /= 1000.0;                                                 
      gyro_z_cal /= 1000.0;
      acc_x_cal /= 1000.0;
      acc_y_cal /= 1000.0;

      horizonalCalibration = 255;
      eeprom_address = 0;
      EEPROM.put(eeprom_address, horizonalCalibration);
      eeprom_address += sizeof(int);
      EEPROM.put(eeprom_address, gyro_x_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, gyro_y_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, gyro_z_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, acc_x_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, acc_y_cal);
      eeprom_address += sizeof(float);
      delay(500);
    }
    else {
      lcd.setCursor(0,1);
      lcd.print("Vertical!");
      orientation = VERTICAL;
      //Read the raw acc and gyro data from the MPU-6050 1000 times                                          
      for (int cal_int = 0; cal_int < 1000 ; cal_int ++) {                  
        read_mpu_6050_data(); 
        //Add the gyro x offset to the gyro_x_cal variable                                            
        gyro_x_cal += gyro_x;
        //Add the gyro y offset to the gyro_y_cal variable                                              
        gyro_y_cal += gyro_y; 
        //Add the gyro z offset to the gyro_z_cal variable                                             
        gyro_z_cal += gyro_z; 
        //Add the acc x offset to the acc_x_cal variable                                            
        acc_y_cal += acc_y;
        //Add the acc y offset to the acc_y_cal variable                                            
        acc_z_cal += acc_z;                                                         
      }
      gyro_x_cal /= 1000.0;                                                 
      gyro_y_cal /= 1000.0;                                                 
      gyro_z_cal /= 1000.0;
      acc_y_cal /= 1000.0;
      acc_z_cal /= 1000.0;
      verticalCalibration = 255;
      eeprom_address = 24;
      EEPROM.put(eeprom_address, verticalCalibration);
      eeprom_address += sizeof(int);
      EEPROM.put(eeprom_address, gyro_x_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, gyro_y_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, gyro_z_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, acc_y_cal);
      eeprom_address += sizeof(float);
      EEPROM.put(eeprom_address, acc_z_cal);
      delay(500);
    }
  }
  //Ana döngüye girmek üzereyiz, bu yüzden ekranı verileri göstermeye hazır hale getiriyoruz
  setLcdBaseline();                                                
}

///////////////
// Ana Döngü //
///////////////


void loop()
{

  if(digitalRead(switchPin) == 0)     //Anahtarın etkin olup olmadığını kontrol ettik
  {
    playBuzzer = !playBuzzer;
    delay(200);                       //Saniyenin 5 de 1 i kadar kullanıcıya zaman verdik kalibrasyon için
  }
  // MPU 6050 den veri alma
  read_mpu_6050_data();
  //modülün doğru yönünü belirlemek için kullanılan acc_x ve acc_z verileri
  if(acc_x < acc_z)
  {
    orientation = HORIZONTAL;
  }
  else
  {
    orientation = VERTICAL;
  }

  if(orientation == HORIZONTAL)   //Yatay ise, eeprom kalibrasyon verileri yüklenir (mevcut değilse, 'Kalibrasyon yapılmadı' mesajı görüntülenir)
  {
    if(horizonalCalibrationNotice == 0)   //Bu bölüm yalnızca bir kez çalıştırılır ve ardından yoksayılır
    {
      eeprom_address = 0;                 //Set EEPROM address to 0 (start of horizontal calibration offsets
      if(EEPROM.get(eeprom_address, horizonalCalibration) == 255)   //Verilere yalnızca gerçekten varsa erişilir!
      {
         eeprom_address += sizeof(int);
         EEPROM.get(eeprom_address, gyro_x_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, gyro_y_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, gyro_z_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, acc_x_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, acc_y_cal);
         eeprom_address += sizeof(float);
      }
      else   //Kalibrasyon verisi yoksa, size söyler!
      {
        lcd.clear();
        lcd.print("Not calibrated!");
        delay(1000);
        setLcdBaseline();
      }
      horizonalCalibrationNotice = 1;
    }
    
    //Ham veriden ofset verisini çıkardık
    gyro_x -= gyro_x_cal;                                                
    gyro_y -= gyro_y_cal;                                                
    gyro_z -= gyro_z_cal; 
    acc_x -= acc_x_cal;
    acc_y -= acc_y_cal;
  
    /*
     * Sonraki birkaç satır, ham verileri, LCD ve LED'lere gönderilebilecek açılara dönüştürmek için işler.
     * Hızlanma verilerinin bölündüğü 4096 değeri MPU6050 veri sayfasından alınır ve örnekleme hızına dayanır.
     * 9.8'in değeri yerçekimidir
     * atan2 işlevi matematik modülündendir ve verilen verilerden açıları hesaplamak için kullanılır.
     */
    thetaM =-atan2((acc_x/4096.0)/9.8 , (acc_z/4096.0)/9.8)/2/3.141592656 * 360;  //Raw data
    phiM =-atan2((acc_y/4096.0)/9.8 , (acc_z/4096.0)/9.8)/2/3.141592656 * 360;  //Raw data
  
    dt=(millis()-millisOld)/1000.;
    millisOld=millis();
    /*
     * Bu bölüm, sistemi daha duyarlı hale getirmek için gyro verilerini kullanır.
     * jiroskop verilerinin bölündüğü 65.5 değeri MPU6050 veri sayfasından alınmıştır ve örnekleme hızına dayanmaktadır.
     */
    theta=(theta+(gyro_y/65.5)*dt)*.96 + thetaM*.04;  //Low pass filter
    phi=(phi+(gyro_x/65.5)*dt)*.96 + phiM*.04;  //Low pass filter
  
    
  }
  else      //Dikey ise, eeprom kalibrasyon verileri yüklenir (mevcut değilse, 'Kalibrasyon yapılmadı' mesajı görüntülenir)
  {
    if(verticalCalibrationNotice == 0)    //Bu bölüm yalnızca bir kez çalıştırılır ve ardından yoksayılır
    {
      eeprom_address = 24;                //EEPROM adresini 24'e ayarlayın (dikey kalibrasyon ofsetlerinin başlangıcı
      if(EEPROM.get(eeprom_address, verticalCalibration) == 255)      //Verilere yalnızca gerçekten varsa erişilir!
      {
         eeprom_address += sizeof(int);
         EEPROM.get(eeprom_address, gyro_x_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, gyro_y_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, gyro_z_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, acc_y_cal);
         eeprom_address += sizeof(float);
         EEPROM.get(eeprom_address, acc_z_cal);
         eeprom_address += sizeof(float);
      }
      else    //Kalibrasyon verisi yoksa uyarır
      {
        lcd.clear();
        lcd.print("Not calibrated!");
        delay(1000);
        setLcdBaseline();
      }
      verticalCalibrationNotice = 1;
    }
    
      //Ham gyro değerlerinden ofset değerlerini çıkarın
    gyro_x -= gyro_x_cal;                                                
    gyro_y -= gyro_y_cal;                                                
    gyro_z -= gyro_z_cal; 
    acc_y -= acc_y_cal;
    acc_z -= acc_z_cal;


    /*
     * Sonraki birkaç satır, ham verileri, LCD ve LED'lere gönderilebilecek açılara dönüştürmek için işler.
     * Hızlanma verilerinin bölündüğü 4096 değeri MPU6050 veri sayfasından alınır ve örnekleme hızına dayanır.
     * 9.8'in değeri yerçekimidir
     * atan2 işlevi matematik modülündendir ve verilen verilerden açıları hesaplamak için kullanılır.
     */
     
    thetaM =-atan2((acc_z/4096.0)/9.8 , (acc_x/4096.0)/9.8)/2/3.141592656 * 360;  //Raw data
    phiM =-atan2((acc_y/4096.0)/9.8 , (acc_x/4096.0)/9.8)/2/3.141592656 * 360;  //Raw data
  
    dt=(millis()-millisOld)/1000.;
    millisOld=millis();
    /*
     * Bu bölüm, sistemi daha duyarlı hale getirmek için gyro verilerini kullanır.
     * jiroskop verilerinin bölündüğü 65.5 değeri MPU6050 veri sayfasından alınmıştır ve örnekleme hızına dayanmaktadır.
     */
    theta=(theta+(gyro_y/65.5)*dt)*.96 + thetaM*.04;  //Low pass filter
    phi=(phi+(gyro_z/65.5)*dt)*.96 + phiM*.04;  //Low pass filter
  }

  /*
   * Yönlendirmeden bağımsız olarak, bu bölüm LCD'yi, LED'leri ve sesli uyarıyı günceller
   */
  // Ekran sayacı artır
  displaycount = displaycount +1;
  
  if (displaycount > 100) 
  {
    
    // Seviye LED'leri için Açıyı Kontrol Edin
    if (phi < -160.01)  //-2.01
    {
      // Turn on Level LED
      digitalWrite(LED_2, HIGH);
      digitalWrite(LED_1, LOW);
      digitalWrite(LED_0, LOW);
      digitalWrite(LED_P1, LOW);
      digitalWrite(LED_P2, LOW);
      if(playBuzzer)
      {
        tone(8,200,200);
      }
      
    } 
    else if ((phi > -160.00) && (phi < -60.01))  //-2.00 -1.01
    {
      // Turn on Level LED
      digitalWrite(LED_2, LOW);
      digitalWrite(LED_1, HIGH);
      digitalWrite(LED_0, LOW);
      digitalWrite(LED_P1, LOW);
      digitalWrite(LED_P2, LOW);
      if(playBuzzer)
      {
        tone(8,1000,400);
      }
      
    } 
    else if ((phi < 60.0) && (phi > -60.0)) //1.0 -1.0
    {
      // Turn on Level LED
      digitalWrite(LED_2, LOW);
      digitalWrite(LED_1, LOW);
      digitalWrite(LED_0, HIGH);
      digitalWrite(LED_P1, LOW);
      digitalWrite(LED_P2, LOW);
      if(playBuzzer)
      {
        tone(8,2000,600);
      }
      
    } 
    else if ((phi > 60.01) && (phi < 160.00)) //1.01 2.00
    {
      // Turn on Level LED
      digitalWrite(LED_2, LOW);
      digitalWrite(LED_1, LOW);
      digitalWrite(LED_0, LOW);
      digitalWrite(LED_P1, HIGH);
      digitalWrite(LED_P2, LOW);
      if(playBuzzer)
      {
        tone(8,1000,400);
      }
      
    } 
    else if (phi > 160.01) //2.01
    {
      // Turn on Level LED
      digitalWrite(LED_2, LOW);
      digitalWrite(LED_1, LOW);
      digitalWrite(LED_0, LOW);
      digitalWrite(LED_P1, LOW);
      digitalWrite(LED_P2, HIGH);
      if(playBuzzer)
      {
        tone(8,200,200);
      }
    }

  //Before printing the angle to the display, it makes any negative numbers positive, so it always shows 0 degrees +.
  //If you want to show negative angles, omit this section
  if(phi<0)
  {
    phi = phi - (2*phi);
  }
  if(theta<0)
  {
    theta = theta - (2*theta);
  }

  //End of section

  lcd.setCursor(9,0);
  lcd.print(phi,1);
  lcd.print("   ");
  lcd.setCursor(9,1);
  lcd.print(theta,1);
  lcd.print("   ");

  Serial.print("Pitch = ");
  Serial.print(phi);
  Serial.print(", Roll = ");
  Serial.println(theta);
    
    
  displaycount = 0;
  
  }
  

 while(micros() - loop_timer < 4000);  //4000
 //Reset the loop timer                                
 loop_timer = micros();
 //Back to the top of the loop to start again!
}

//////////////////////
//alt Program Bölümü//
//////////////////////

// Bu rutin sadece lcd'yi temizler ve Açı ve Dönmeyi değerleri göstermeye hazır olarak yazdırır.
void setLcdBaseline()
{
  lcd.clear();
  lcd.print("Angle = ");
  lcd.setCursor(0,1);
  lcd.print("Roll = ");
}


//MPU6050'yi ilk kullanım için ayarlamak için gereklidir.
void setup_mpu_6050_registers(){

  Wire.beginTransmission(0b1101000); //Bu, MPU'nun I2C adresidir (AC0 düşük/yüksek veri sayfası için b1101000/b1101001 sn. 9.2)
  Wire.write(0x6B); //6B kaydına erişim - Güç Yönetimi (Böl. 4.28)
  Wire.write(0b00000000); //UYKU kaydının 0'a ayarlanması. (Gerekli; s. 9'daki Not'a bakın)
  Wire.endTransmission();  
                                              
  Wire.beginTransmission(0x68);                                   
  Wire.write(0x1C);   
  //İstenen başlangıç ​​kaydını ayarlayın                                           
  Wire.write(0x10); 
  //İletimi sonlandır                                                   
  Wire.endTransmission();                     
  Wire.beginTransmission(0x68);
  //İstenen başlangıç ​​kaydını gönder                                       
  Wire.write(0x1B);
  //İstenen başlangıç ​​kaydını ayarlayın                                                
  Wire.write(0x08); 
  //İletimi sonlandır                                                  
  Wire.endTransmission();                                     
}


//MPU6050'den jiroskop ve hızlanma değerlerini almak için bu rutin gereklidir.
void read_mpu_6050_data(){ 
                                       
  Wire.beginTransmission(0x68);                                     
  Wire.write(0x3B);                                           
  Wire.endTransmission();                                
  Wire.requestFrom(0x68,14);                                       
  while(Wire.available() < 14);                                     
  acc_x = Wire.read()<<8|Wire.read();                                  
  acc_y = Wire.read()<<8|Wire.read();                                  
  acc_z = Wire.read()<<8|Wire.read();                                  
  temp = Wire.read()<<8|Wire.read();                                   
  gyro_x = Wire.read()<<8|Wire.read();                                 
  gyro_y = Wire.read()<<8|Wire.read();                                 
  gyro_z = Wire.read()<<8|Wire.read();                 
}
