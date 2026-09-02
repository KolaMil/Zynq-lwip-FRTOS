# Прошивка из CLion под Linux

Замена «Run As → Launch on Hardware» из Xilinx SDK. XSCT здесь нет поэтому
загрузка идёт через OpenOCD + GDB конфигурацией CLion *OpenOCD Download & Run*.

## Установка

```
sudo apt install openocd gdb-multiarch
sudo cp openocd/99-digilent-jtag.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

После правила переткнуть кабель JTAG. `/dev/ttyUSB0` должен исчезнуть,
консоль CP210x остаётся.

В CLion: *Embedded Development* → OpenOCD `/usr/bin/openocd`, *Toolchains* →
Debugger `/usr/bin/gdb-multiarch`. Родной `arm-none-eabi-gdb` требует
`libncurses.so.5` которого в Ubuntu 26.04 нет.

Готовая конфигурация запуска лежит в `.idea/runConfigurations/`. Если править
руками — атрибуты там в kebab-case, camelCase игнорируется молча.

## Что делает program

Сброс периферии PS → очистка и загрузка битстрима → `ps7_init` →
`ps7_post_config` → приведение CPU0 в состояние после включения → загрузка
образа в DDR, проверка, PC, парковка CPU1 → `resume`.

`ZYNQ_SKIP_PL=1` пропускает загрузку матрицы (~50 с), `ZYNQ_NO_PERIPH_RESET=1` —
сброс периферии.

Почему сделано именно так — в комментариях `zynq7000.cfg`, `zynq_7030.cfg`
и `ps7_init_shim.tcl`.
