#include <msp430.h>
#include <string.h>
#include <stdio.h>

// Sabitler
#define THRESHOLD 820          // ADC esik degeri
#define LED_ON_TIME 100        // LED'in yanik kalma süresi (ms)
#define DEBOUNCE_MS 800        // Minimum nabiz araligi (ms)
#define TOUCH_LOST_COUNT 10    // El çekildikten sonra LED'in kapanmasi için sayaç (10x20ms = 200ms)

// Fonksiyon prototipleri
void delay_ms(unsigned int ms);
void adc_init(void);
void uart_init(void);
void uart_send_string(char *str);
void uart_send_byte(unsigned char data);
unsigned int read_adc(void);

// UART tampon boyutu
#define UART_BUFFER_SIZE 32
char uart_buffer[UART_BUFFER_SIZE];

int main(void) {
    // Watchdog timer'i durdur
    WDTCTL = WDTPW | WDTHOLD;
    
    // Saat ayarlari (1MHz)
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    
    // P1.0 pinini (LED) çikis olarak ayarla
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;  // LED baslangiçta kapali
    
    // ADC ve UART baslat
    adc_init();
    uart_init();
    
    // Degiskenler
    unsigned long millis = 0;          // Geçen zaman (ms)
    unsigned long last_beat_time = 0;  // Son nabiz zamani
    unsigned long led_on_time = 0;     // LED yanma baslangiç zamani
    unsigned char led_state = 0;       // LED durumu (0=kapali, 1=açik)
    unsigned char pulse_ready = 0;     // Nabiz algilama hazirlik durumu
    unsigned char low_adc_counter = 0; // Düsük ADC degeri sayaci (el çekildigini algilamak için)
    unsigned int heart_rate = 0;       // Hesaplanan nabiz (BPM)
    
    // Ana döngü
    while (1) {
        // ADC degerini oku
        unsigned int adc = read_adc();
        
        // El çekilme kontrolü - öncelikli kontrol
        if (adc < THRESHOLD) {
            // ADC düsük ise sayaci artir
            if (low_adc_counter < TOUCH_LOST_COUNT)
                low_adc_counter++;
        } else {
            // ADC yüksek ise sayaci sifirla
            low_adc_counter = 0;
        }
        
        // El çekildiyse LED'i kapat ve islemi durdur
        if (low_adc_counter >= TOUCH_LOST_COUNT) {
            P1OUT &= ~BIT0;  // LED'i kapat
            led_state = 0;
            pulse_ready = 0;
            
            // Diger kontrolleri atla ve döngünün basina dön
            delay_ms(20);
            millis += 20;
            continue;
        }
        
        // Nabiz geçisi algilama - sadece el çekilmediyse
        if (adc < THRESHOLD && low_adc_counter < TOUCH_LOST_COUNT) {
            // ADC düsükse, nabiz algilamaya hazir ol
            pulse_ready = 1;
        }
        
        // Nabiz algilandi - sadece el çekilmediyse
        if ((adc > THRESHOLD) && pulse_ready && 
            (millis - last_beat_time > DEBOUNCE_MS) && 
            low_adc_counter < TOUCH_LOST_COUNT) {
            
            // Nabiz zamanini hesapla
            unsigned long beat_time = millis - last_beat_time;
            
            pulse_ready = 0;             // Hazirlik durumunu sifirla
            last_beat_time = millis;     // Son nabiz zamanini güncelle
            led_on_time = millis;        // LED yanma zamanini güncelle
            led_state = 1;               // LED durumunu açik yap
            P1OUT |= BIT0;               // LED'i yak
            
            // Nabizi BPM olarak hesapla (60000 ms / beat_time ms)
            if (beat_time > 0) {
                heart_rate = 60000 / beat_time;
                
                // Makul araligi kontrol et (40-200 BPM)
                if (heart_rate >= 40 && heart_rate <= 200) {
                    // Nabiz degerini BPM ile birlikte gönder
                    sprintf(uart_buffer, "%d BPM\r\n", heart_rate);
                    uart_send_string(uart_buffer);
                }
            }
        }
        
        // LED zamanlamasi
        if (led_state && (millis - led_on_time >= LED_ON_TIME)) {
            P1OUT &= ~BIT0;  // LED'i kapat
            led_state = 0;   // LED durumunu kapali yap
        }
        
        // 20ms bekle ve zaman sayacini güncelle
        delay_ms(20);
        millis += 20;
    }
}

// Milisaniye cinsinden gecikme fonksiyonu
void delay_ms(unsigned int ms) {
    while (ms--) {
        __delay_cycles(1000); // 1 MHz saat için 1ms
    }
}

// ADC baslatma fonksiyonu
void adc_init(void) {
    ADC10CTL1 = INCH_3;                  // P1.3 pinini ADC girisi olarak kullan
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;  // ADC'yi yapilandir ve aç
}

// ADC ölçüm fonksiyonu
unsigned int read_adc(void) {
    ADC10CTL0 |= ENC + ADC10SC;          // Örnekleme ve dönüsümü baslat
    while (ADC10CTL1 & ADC10BUSY);       // Dönüsüm tamamlanana kadar bekle
    return ADC10MEM;                     // ADC sonucunu döndür
}

// UART baslatma fonksiyonu
void uart_init(void) {
    // UART pinlerini yapilandir
    P1SEL = BIT1 + BIT2;                 // P1.1 = RXD, P1.2 = TXD
    P1SEL2 = BIT1 + BIT2;
    
    // UART modülünü yapilandir
    UCA0CTL1 |= UCSSEL_2;                // SMCLK kaynakli
    
    // 1MHz için 9600 Baud hizi ayarlari
    UCA0BR0 = 104;                       // 1MHz 9600 (see User's Guide)
    UCA0BR1 = 0;                         // 1MHz 9600
    UCA0MCTL = UCBRS0;                   // Modulation UCBRSx = 1
    
    UCA0CTL1 &= ~UCSWRST;                // USCI modülünü baslat
}

// UART üzerinden bir byte gönder
void uart_send_byte(unsigned char data) {
    while (!(IFG2 & UCA0TXIFG));         // TXBUF hazir olana kadar bekle
    UCA0TXBUF = data;                    // Veriyi gönder
}

// UART üzerinden string gönder
void uart_send_string(char* str) {
    unsigned int i = 0;
    while (str[i] != '\0') {
        uart_send_byte(str[i++]);
    }
}