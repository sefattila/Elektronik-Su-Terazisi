# Elektronik Su Terazisi
 
## Özet
Bu proje Arduino kullanarak Elektronik Su Terazisi yapmak amaçlanmıştır. Kod olarak C++ ve kodlama ortamı olarak da Arduino IDE kullanılmıştır.

## Proje Bileşenleri ve Çalışma Mantığı
Projenin ana bileşeni olan MPU 6050 ile basit bir Elektronik Su Terazisi yapılmıştır. MPU 6050 6 Eksen İvme ve Gyro Sensörüdür. Bunun yanında ek bileşenler olarak sistemin + ve - eksende dengesini gösterebilmek için ekran eklenmiş ve aynı şekilde beş adet led eklenmiştir. Eklenen ledler sistemin + ve - eksendeki değerine göre sağa veya sola doğru aktiflerşir eğer sistem dengedeyse sadece ortadaki led aktifleşir. 

Bunun yanında buton ve buzzer da var. Sistem aktifken butona basıldığında sistemin durumuna göre buzzerdan belirli sesler üretilmiştir. Bu sesler dengedeyken sabit bir ses sistem +1 veya -1 durumundayken kesik şiddetli ses ve en son olarak +2 veya -2 durumundayken kesik daha şiddetli bir ses üretir.

Sistem kalibrasyonu için kullanılan Arduino üzerindeki restart tuşunu basıldıktan hemen sonra butona basılır ise sistem kendini kalibre edecektir. Ve bu yaşanan her durumu ekranda görebileceksiniz.

## Kullanılan Malzemeler
- Arduino Mikro(UNO,NANO vs)
- MPU 6050
- Buzzer
- Button
- Led 
- Direnç
- Jumper Kablo
- 16x2 LCD Ekran

## Proje Şeması
![Alt text](https://content.instructables.com/ORIG/F43/YTUT/KFTOR7ZL/F43YTUTKFTOR7ZL.png?auto=webp&frame=1&width=1024&fit=bounds&md=d1a8fbdcf43abe64e048f1f23b96f87d)

## Gereklilikler
Kodu çalıştırabilmek için LCD ekran için gerekli olan LiquidCrystal_I2C kütüphanesini Arduino IDE programına indirilmesi gereklidir. Bu işlemide Arduino IDE programının içinde 'Taslak' kısmından 'library ekle' dedikten sonra 'Kütüphaneleri Yönet' kısmına tıklıyoruz. Gelen ekran gerekli kütüphaneyi arama çubuğuna yazdıktan sonra kütüphaneyi kuruyoruz.

## Proje Lisansı
Proje şahsım adına ait bir proje değildir. Sadece kod kısmında ufak değişikliler yapılarak farklı bir siteden alınmıştır.
Asıl projeye [buraya](https://www.instructables.com/Arduino-MPU6050-Based-Digital-Spirit-Level/) tıklayarak ulaşabilirsiniz.
