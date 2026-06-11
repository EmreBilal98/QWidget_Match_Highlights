# ⚽ QWidget Maç Özetleri & Dinamik Skor Tabelası Üreticisi

Yerel veya uzak halı saha maç videolarını işlemek, video üzerine kronolojik ve dinamik skor tabelası (scoreboard) basmak, pozisyon kesitleri almak ve bu kesitleri otomatik olarak tek bir maç özeti (kolaj) videosu halinde birleştirmek için tasarlanmış asenkron bir gömülü-masaüstü yazılımıdır.

![Ana Uygulama Paneli](screenshots/main_screen.png)

---

## 🚀 Öne Çıkan Özellikler

* **Güvenli Kimlik Doğrulama Sistemi:** FastAPI backend mimarisiyle entegre, OAuth2 token tabanlı çalışan ve şifre maskeleme (`QLineEdit::PasswordEchoOnEdit`) özelliğine sahip Modüler Giriş (Login) ve Kayıt Ol (SignUp) arayüzleri.
* **Donanım Hızlandırmalı Video Tuvali:** Sanal makinelerde veya Linux/Ubuntu ortamlarında sıkça yaşanan donanımsal katman sızmalarını ve hayalet görüntü kırpışmalarını engellemek için `QGraphicsView` ve `QGraphicsVideoItem` mimarisiyle geliştirilmiş kararlı video alanı.
* **Dinamik Skor Basma Motoru:** Zaman tabanlı koşullu filtreleme (`enable='between(t,x,y)'` ve `gt(t,x)`) kullanarak, tablodaki gol verilerine göre skoru video piksellerine kalıcı olarak işleyen gelişmiş FFmpeg Filtergraph yapısı.
* **Asenkron Kesme Kuyruğu:** Kullanıcı arayüzünün (UI) kilitlenmesini önleyen ve arka arkaya seçilen 1 dakikalık pozisyon kliplerini sırayla kesen, durum korumalı (`FFmpegMode`) `QProcess` çoklu iş parçacığı yönetimi.
* **Tek Tıkla Anında Maç Özeti:** Kesilen tüm klipleri, kalite kaybı yaşamadan ve yeniden render (encode) gerektirmeden saliseler içinde uç uca ekleyen yüksek hızlı FFmpeg Concat Demuxer kalıbı.
* **Entegre Altyapı Otomasyonu:** Hedef bilgisayarlarda Nginx sunucusu ile entegre çalışmaya hazır, standart Unix dizinlerini (`/var/www/matchrecord`) video akış bileşenlerine bağlayan yapı.

---

## 🏗️ Sistem Mimarisi

Uygulama, masaüstü arayüzünün akıcılığını korurken veri giriş-çıkış (IO) performansını dengelemek için yerel akış yığınıyla asenkron olarak haberleşir:

```mermaid
graph TD
    %% Donanım Katmanı
    subgraph donanim ["Donanım Katmanı (Saha İçi)"]
        A["STM32 Skor Tabelası"] -->|UART - Seri İletişim| B["ESP8266 Wi-Fi Modülü"]
    end

    %% Sunucu Katmanı
    subgraph sunucu ["Sunucu & Veri Katmanı"]
        B -->|HTTP POST /api/records| C["FastAPI Sunucusu"]
        C -->|SQL Kayıt| D[("SQLite Veritabanı")]
    end

    %% Masaüstü & İşleme Katmanı
    subgraph istemci ["İstemci & Multimedya Katmanı (Masaüstü)"]
        E["Qt6 C++ Masaüstü Uygulaması"] -->|HTTP GET /api/records| C
        E -->|1. Ham Videoyu Oku| F["Nginx Medya Sunucusu"]
        E -->|2. Zaman Kodlarını Gönder| G["FFmpeg İşleme Motoru"]
        
        G -->|Bake Modu| H["Skorlu Full Maç"]
        G -->|Trim Modu| I["1 Dk. Pozisyon Klipleri"]
        G -->|Concat Modu| J["Nihai Maç Özeti Video"]
        
        H --> K["Klasör: /var/www/matchrecord/trim"]
        I --> K
        J --> K
    end

    %% Stil Ayarları
    style A fill:#2c3e50,stroke:#34495e,stroke-width:2px,color:#fff
    style B fill:#e67e22,stroke:#d35400,stroke-width:2px,color:#fff
    style C fill:#1abc9c,stroke:#16a085,stroke-width:2px,color:#fff
    style D fill:#34495e,stroke:#2c3e50,stroke-width:2px,color:#fff
    style E fill:#2980b9,stroke:#2574a9,stroke-width:2px,color:#fff
    style G fill:#c0392b,stroke:#962d22,stroke-width:2px,color:#fff
```
---

## 📸 Ekran Görüntüleri & Görsel Tur

> Arayüz elemanlarınızın çalışma akışını burada sergileyebilirsiniz. Yakaladığınız ekran görüntülerini projenizin kök dizininde `screenshots/` adlı bir klasör oluşturarak içine ekleyebilirsiniz.

### 🔐 1. Güvenli Erişim (Giriş & Kayıt Panelleri)
| Giriş Ekranı | Kullanıcı Kaydı |
|---|---|
| ![Giriş](screenshots/log_in.png) | ![Kayıt](screenshots/sign_up.png) |

### 📺 2. Video Analiz & Kesim İstasyonu
Ana panel, zaman damgalarını video konumuyla senkronize eder. Video alanının esnek yapısı ve sol üstteki dinamik skor tabelası (`3 - 2`) tam uyumla çalışır.
![Ana Panel Ekran Görüntüsü](screenshots/main_screen.png)

### ⏳ 3. Bloklamayan İşlem Penceresi (Modal Pop-up)
Arka planda uzun H.264 maç videoları işlenirken, kullanıcının arayüze dokunmasını engelleyen ancak asenkron akışı bozmayan özel kilitli yükleniyor penceresi.
![İşlem Durumu](screenshots/video_render.png)

---

## 🛠️ Kurulum ve Bağımlılıklar

### Gereksinimler
* **Qt 6.x** (Widgets, Multimedia ve Network modülleri)
* **FFmpeg 6.x veya üzeri** (Sistem ortam değişkenlerine/PATH'e eklenmiş olmalıdır)
* **Nginx** (Port `8085` üzerinde `/var/www/matchrecord` dizinini sunacak şekilde yapılandırılmış)

### Linux (Ubuntu/Debian) İçin Hızlı Kurulum
```bash
# 1. Medya bağımlılıklarını kurun
sudo apt update && sudo apt install ffmpeg nginx -y

# 2. Depoyu klonlayın
git clone [https://github.com/EmreBilal98/QWidget_Match_Highlights.git](https://github.com/EmreBilal98/QWidget_Match_Highlights.git)
cd QWidget_Match_Highlights

# 3. Nginx motoru için gerekli yerel dizinleri hazırlayın
sudo mkdir -p /var/www/matchrecord/matches
sudo chown -R www-data:www-data /var/www/matchrecord
sudo chmod -R 755 /var/www/matchrecord
