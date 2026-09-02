# hw/ps7_init.tcl написан под XSCT и зовёт mwr, mrd, mask_write, configparams,
# которых в OpenOCD нет. Реализуем их через read_memory/write_memory.
# Вызовов там около 600, каждый - обмен по JTAG

# Путь считаем здесь: внутри proc `info script` вернёт не этот файл
set zynq_shim_dir [file dirname [file normalize [info script]]]

# У XSCT флаги перед операндами; -force тут смысла не имеет
proc mwr {args} {
    set operands {}
    foreach tok $args {
        if {[string index $tok 0] ne "-"} {
            lappend operands $tok
        }
    }
    write_memory [lindex $operands 0] 32 [list [lindex $operands 1]]
}

proc mask_write {addr mask val} {
    # Полная маска - обычная запись, вдвое меньше обмена по JTAG
    if {$mask == 0xFFFFFFFF} {
        write_memory $addr 32 [list $val]
    } else {
        set cur [lindex [read_memory $addr 32 1] 0]
        write_memory $addr 32 [list [expr {($cur & ~$mask) | ($val & $mask)}]]
    }
}

# ps7_init.tcl режет результат по 9 символов, отсюда ведущий "0"
proc mrd {addr} {
    return [format "0%08X" [lindex [read_memory $addr 32 1] 0]]
}

# Аналога в OpenOCD нет, переключать нечего
proc configparams {args} {
    return 0
}

source [file join [file dirname [info script]] .. hw ps7_init.tcl]

# Замены после подключения: свои версии завязаны на строковый вывод mrd

proc mask_poll {addr mask} {
    for {set i 0} {$i < 100000} {incr i} {
        if {([lindex [read_memory $addr 32 1] 0] & $mask) != 0} {
            return
        }
    }
    error [format "ps7_init: mask_poll timed out at 0x%08X mask 0x%08X" $addr $mask]
}

proc ps_version {} {
    return [expr {([lindex [read_memory 0xF8007080 32 1] 0] >> 28) & 0xF}]
}

# Тактовые частоты, PLL, MIO, DDR. До загрузки образа: он линкуется
# по 0x100000, а это DDR, которой до сих пор нет
proc zynq_ps7_init {} {
    halt
    ps7_init
    echo "ps7_init: PS/DDR ready"
}

# Преобразователи уровней PS-PL. Отдельно: SDK делает это после матрицы
proc zynq_ps7_post_config {} {
    ps7_post_config
    echo "ps7_post_config: PS-PL level shifters enabled"
}

# Без матрицы нет Ethernet: MDIO идёт по MIO, а RGMII через PL. ~50 с на 6 МБ;
# ZYNQ_SKIP_PL=1 пропускает - конфигурация держится до снятия питания
proc zynq_load_bitstream {} {
    if {[info exists ::env(ZYNQ_SKIP_PL)] && $::env(ZYNQ_SKIP_PL) ne "0"} {
        echo "pl: skipped (ZYNQ_SKIP_PL set)"
        return
    }
    global zynq_shim_dir
    set bit [file join $zynq_shim_dir .. hw design_1_wrapper.bit]
    echo "pl: clearing fabric"
    zynqpl_program zynq_pl.bs
    echo "pl: loading [file tail $bit] (~50 s)"
    pld load 0 $bit
    echo "pl: fabric configured"
}
