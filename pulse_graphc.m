clc;
clear;

port = "COM8";         % Doğru COM port
baud = 9600;           % MSP430 baud rate
s = serialport(port, baud);
flush(s);              % Portu temizle

pause(1);              % MSP430 hazır olsun

data = zeros(1, 100);  % İlk veri dizisi
figure;
title("Gerçek Zamanlı Nabız (BPM)");
xlabel("Zaman");
ylabel("BPM");
grid on;

while true
    try
        if s.NumBytesAvailable > 0
            line = readline(s);                             % Satırı oku
            bpmStr = regexp(line, '\d+', 'match');          % Sayıyı ayıkla
            if ~isempty(bpmStr)
                bpm = str2double(bpmStr{1});
                data = [data(2:end), bpm];                  % FIFO güncelle

                % EKG benzeri alan dolgulu grafik
                area(data, 'FaceColor', [1 0 0], 'EdgeColor', 'r');

                ylim([40 160]);                             % Sabit y ekseni
                xlim([1 length(data)]);                     % X eksenini sabitle
                title(sprintf("Nabız: %d BPM", bpm));
                drawnow;
            end
        end
    catch ME
        disp("Hata: " + ME.message);
        break;
    end
end
