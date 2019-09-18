# NB-Fi STM32L0 Quickstart
VisualGDB project with minimal peripherals configured and NB-Fi radio based on Axsem AX5043
* STM32L0 clocked from internal 4 MHz MSI oscillator
* LSI for timekeeping and feeding LPTIM timer

# AX5043 Connections
AX5043 | IRQ | SEL | CLK | MISO | MOSI 
--- | --- | --- | --- | --- | ---
STM32L0 | PA3 | PA4 | PA5 | PA6 | PA7 