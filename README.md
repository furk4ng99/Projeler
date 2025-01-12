# Tiva C (TM4C123) Projeleri

Bu repo içerisinde **Tiva C (TM4C123)** mikrodenetleyicisiyle gerçekleştirilen çeşitli projelerin örnek kodları bulunmaktadır. Her proje, farklı bir donanım veya kütüphane kullanımını göstermektedir.

---

## Proje 1: LCD Üzerine Karakter Yazma

- **Özet**: Tiva C üzerinde LCD'ye 4-bit modda bir karakter (`'A'`) yazmayı gösterir.  
- **Donanım**:  
  - **PB0 -> RS**  
  - **PB1 -> E**  
  - **PB4 -> D4, PB5 -> D5, PB6 -> D6, PB7 -> D7**  
- **Ana Özellikler**:  
  - LCD 4-bit mod kurulumu  
  - LCD’ye komut ve veri gönderen fonksiyonlar  


---

## Proje 2: LCD’de Saat Gösterme + Karakter

- **Özet**: LCD’nin ilk satırına `'A'` karakterini yazarken, ikinci satırının son sütununa her saniye güncellenen `xx:yy:zz` formatında saat bilgisi yazdırır.  
- **Donanım**:  
  - **LCD** aynı pin tanımları  
  - **Timer Yok** (örnek, döngüde `SysCtlDelay` ile zaman oluşturulur)  
- **Ana Özellikler**:  
  - Saat bilgisi yazdırma ve otomatik artış  
  - LCD’de farklı satır-sütun konumları kullanma  


---

## Proje 3: Timer ve UART ile Saat Uygulaması

- **Özet**: Timer0 kullanarak her saniye sayacı artırır, LCD ekranın ikinci satırında zamanı gösterir ve aynı zamanda UART üzerinden bilgiyi bilgisayara yollar. UART üzerinden gelen `hh.mm.ss` formatındaki veriyi de saat olarak ayarlayabilir.  
- **Donanım**:  
  - **PB0-PB7 -> LCD**  
  - **Timer0** -> saniyelik kesme  
  - **UART0 (PA0-RX, PA1-TX)**  
- **Ana Özellikler**:  
  - Zamanın UART ile hem gönderilmesi hem de alınması  
  - Timer kesmesi kullanımı (1 saniye)  


---

## Proje 4: LM35 (ADC) + LCD + UART

- **Özet**: LM35 sıcaklık sensöründen alınan analog veriyi **ADC** kullanarak okur. Okunan sıcaklık değeri LCD’de görüntülenir ve aynı zamanda UART üzerinden bilgisayara gönderilir.  
- **Donanım**:  
  - **PB0-PB7 -> LCD**  
  - **PE3 (AIN0) -> LM35** çıkışı (ADC giriş)  
  - **PA0-RX, PA1-TX -> UART0**  
- **Ana Özellikler**:  
  - ADC Sequencer 3 ayarları (tek kanal)  
  - LM35 sıcaklık dönüşümü  
  - LCD ve UART üzerinde sıcaklık gösterimi  


---

## Proje 5: Hibernation (Uyku) Modu

- **Özet**: TM4C123’ün hibernation modülünü kullanarak 10 saniye sonra tekrar uyanacak şekilde derin uykuya geçer.  
- **Donanım**:  
  - **Hibernation modülü** (TM4C123G veya uygun türevinde)  
  - **VBAT** pinine pil veya sürekli besleme  
- **Ana Özellikler**:  
  - RTC tabanlı uyandırma (10 sn sonra)  
  - Uyandıktan sonra sistemin yeniden başlaması  


---

## Proje 6: Floating Point İşlemleri

- **Özet**: TM4C123 üzerinde **FPU** (Floating Point Unit) etkinleştirilip, çeşitli matematik fonksiyonları (`+, -, *, /, sqrt, pow`) yapılır ve sonuçlar UART üzerinden gönderilir.  
- **Donanım**:  
  - **UART0** (PA0-RX, PA1-TX)  
- **Ana Özellikler**:  
  - FPU etkinleştirme (`FPUEnable()`)  
  - Matematik fonksiyonları (`math.h`)  
  - Sonuçların UART ile aktarılması  


---

## Nasıl Çalıştırılır?

1. **TivaWare** kurulumunu yapın ve proje dosyalarını IDE'nizde (örn. CCS, Keil, IAR) veya `makefile` düzeninde derleyin.  
2. **Bağlantıları**, projedeki pin tanımlarına uygun şekilde gerçekleştirin.  
3. Her proje için `main.c` dosyasını derleyip Tiva C LaunchPad’e yükleyin.  
4. UART kullanılıyorsa bir seri terminal programı (Putty, TeraTerm vb.) açın, ilgili COM portunu seçin ve projedeki baud hızını ayarlayın (örn. 115200).

---
