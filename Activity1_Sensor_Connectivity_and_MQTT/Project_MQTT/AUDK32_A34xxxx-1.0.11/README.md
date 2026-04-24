# Sensör Bağlanabilirliği ve MQTT

Empa Electronics tarafından düzenlenen Accelerators Workshops University etkinliğimize hoş geldiniz. Bu kılavuz, ABOV A34G43x mikrodenetleyicisi üzerinde çalışan "Uçtan Uca MQTT Veri Toplayıcı" uygulamasının geliştirme adımlarında size rehberlik edecektir.

## Geliştirme Ortamı Kurulumu

Projeyi derleyip yükleyebilmek için aşağıdaki kurulum kılavuzunu takip ediniz:

### ↳ [VS Code + CMake ile Kurulum](SETUP_VSCODE.md)
Visual Studio Code, CMake ve ARM GCC kullanarak açık kaynaklı araçlarla geliştirme ortamı kurulumunu açıklar.

---

## Donanım

Bu projede kullanılan mikrodenetleyici ve sensörler:

| Bileşen | Açıklama | Arayüz |
|---|---|---|
| **ABOV A34G43x** | Cortex-M4F MCU, 256 KB Flash, 128 KB RAM | — |
| **SHT40** | Sıcaklık ve nem sensörü | I2C |
| **LIS2DE12** | 3 eksen ivmeölçer | I2C |
| **MP23ABS1** | Analog MEMS mikrofon | ADC |
| **ESP32 (AT)** | Wi-Fi modülü | UART |

---

## Proje Hakkında

**Sensör Bağlanabilirliği**  
Bu projede ABOV A34G43x MCU'nun HAL (Hardware Abstraction Layer) kütüphaneleri kullanılarak SHT40 (sıcaklık/nem), LIS2DE12 (ivme) ve MP23ABS1 (ses) sensörlerinden veri okunmaktadır. Sensörler I2C ve ADC arayüzleri üzerinden mikrodenetleyiciye bağlanmış olup ABOV HAL sürücüleri aracılığıyla yapılandırılmaktadır.

**MQTT Protokolü & AWS**  
MQTT (Message Queuing Telemetry Transport), IoT uygulamaları için geliştirilmiş, hafif yapısı ve düşük bant genişliği gereksinimi ile öne çıkan bir mesajlaşma protokolüdür. Bu protokol, sensör verilerinin bulut tabanlı platformlara hızlı ve güvenli bir şekilde iletilmesini sağlar. Bu projede ESP32 Wi-Fi modülü üzerinden AWS IoT Core'a MQTT bağlantısı kurulmakta ve sensörlerden okunan veriler buluta aktarılmaktadır.

**Kaynaklar & Okuma Önerileri**

1. [What is MQTT? - AWS](https://aws.amazon.com/what-is/mqtt/)
2. [ABOV A34G43x Veri Sayfası](Document/Chipset_Description.md)
3. [ABOV HAL Doxygen Dokümantasyonu](Document/Doxygen/html/index.html)
