#include <msp430.h>
#include <string.h>
#include <stdio.h>

// Sabitler
#define THRESHOLD 820          // ADC esik degeri
#define LED_ON_TIME 100        // LED'in yanik kalma s�resi (ms)
#define DEBOUNCE_MS 800        // Minimum nabiz araligi (ms)
#define TOUCH_LOST_COUNT 10    // El �ekildikten sonra LED'in kapanmasi i�in saya� (10x20ms = 200ms)

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
    
    // P1.0 pinini (LED) �ikis olarak ayarla
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;  // LED baslangi�ta kapali
    
    // ADC ve UART baslat
    adc_init();
    uart_init();
    
    // Degiskenler
    unsigned long millis = 0;          // Ge�en zaman (ms)
    unsigned long last_beat_time = 0;  // Son nabiz zamani
    unsigned long led_on_time = 0;     // LED yanma baslangi� zamani
    unsigned char led_state = 0;       // LED durumu (0=kapali, 1=a�ik)
    unsigned char pulse_ready = 0;     // Nabiz algilama hazirlik durumu
    unsigned char low_adc_counter = 0; // D�s�k ADC degeri sayaci (el �ekildigini algilamak i�in)
    unsigned int heart_rate = 0;       // Hesaplanan nabiz (BPM)
    
    // Ana d�ng�
    while (1) {
        // ADC degerini oku
        unsigned int adc = read_adc();
        
        // El �ekilme kontrol� - �ncelikli kontrol
        if (adc < THRESHOLD) {
            // ADC d�s�k ise sayaci artir
            if (low_adc_counter < TOUCH_LOST_COUNT)
                low_adc_counter++;
        } else {
            // ADC y�ksek ise sayaci sifirla
            low_adc_counter = 0;
        }
        
        // El �ekildiyse LED'i kapat ve islemi durdur
        if (low_adc_counter >= TOUCH_LOST_COUNT) {
            P1OUT &= ~BIT0;  // LED'i kapat
            led_state = 0;
            pulse_ready = 0;
            
            // Diger kontrolleri atla ve d�ng�n�n basina d�n
            delay_ms(20);
            millis += 20;
            continue;
        }
        
        // Nabiz ge�isi algilama - sadece el �ekilmediyse
        if (adc < THRESHOLD && low_adc_counter < TOUCH_LOST_COUNT) {
            // ADC d�s�kse, nabiz algilamaya hazir ol
            pulse_ready = 1;
        }
        
        // Nabiz algilandi - sadece el �ekilmediyse
        if ((adc > THRESHOLD) && pulse_ready && 
            (millis - last_beat_time > DEBOUNCE_MS) && 
            low_adc_counter < TOUCH_LOST_COUNT) {
            
            // Nabiz zamanini hesapla
            unsigned long beat_time = millis - last_beat_time;
            
            pulse_ready = 0;             // Hazirlik durumunu sifirla
            last_beat_time = millis;     // Son nabiz zamanini g�ncelle
            led_on_time = millis;        // LED yanma zamanini g�ncelle
            led_state = 1;               // LED durumunu a�ik yap
            P1OUT |= BIT0;               // LED'i yak
            
            // Nabizi BPM olarak hesapla (60000 ms / beat_time ms)
            if (beat_time > 0) {
                heart_rate = 60000 / beat_time;
                
                // Makul araligi kontrol et (40-200 BPM)
                if (heart_rate >= 40 && heart_rate <= 200) {
                    // Nabiz degerini BPM ile birlikte g�nder
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
        
        // 20ms bekle ve zaman sayacini g�ncelle
        delay_ms(20);
        millis += 20;
    }
}

// Milisaniye cinsinden gecikme fonksiyonu
void delay_ms(unsigned int ms) {
    while (ms--) {
        __delay_cycles(1000); // 1 MHz saat i�in 1ms
    }
}

// ADC baslatma fonksiyonu
void adc_init(void) {
    ADC10CTL1 = INCH_3;                  // P1.3 pinini ADC girisi olarak kullan
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;  // ADC'yi yapilandir ve a�
}

// ADC �l��m fonksiyonu
unsigned int read_adc(void) {
    ADC10CTL0 |= ENC + ADC10SC;          // �rnekleme ve d�n�s�m� baslat
    while (ADC10CTL1 & ADC10BUSY);       // D�n�s�m tamamlanana kadar bekle
    return ADC10MEM;                     // ADC sonucunu d�nd�r
}

// UART baslatma fonksiyonu
void uart_init(void) {
    // UART pinlerini yapilandir
    P1SEL = BIT1 + BIT2;                 // P1.1 = RXD, P1.2 = TXD
    P1SEL2 = BIT1 + BIT2;
    
    // UART mod�l�n� yapilandir
    UCA0CTL1 |= UCSSEL_2;                // SMCLK kaynakli
    
    // 1MHz i�in 9600 Baud hizi ayarlari
    UCA0BR0 = 104;                       // 1MHz 9600 (see User's Guide)
    UCA0BR1 = 0;                         // 1MHz 9600
    UCA0MCTL = UCBRS0;                   // Modulation UCBRSx = 1
    
    UCA0CTL1 &= ~UCSWRST;                // USCI mod�l�n� baslat
}

// UART �zerinden bir byte g�nder
void uart_send_byte(unsigned char data) {
    while (!(IFG2 & UCA0TXIFG));         // TXBUF hazir olana kadar bekle
    UCA0TXBUF = data;                    // Veriyi g�nder
}

// UART �zerinden string g�nder
void uart_send_string(char* str) {
    unsigned int i = 0;
    while (str[i] != '\0') {
        uart_send_byte(str[i++]);
    }
}