%macro and_eax 1
      dw 0xE081
      dd %1
%endmacro
%macro add_eax 1
      dw 0xC081
      dd %1
%endmacro
%macro adc_eax 1
      dw 0xD081
      dd %1
%endmacro
%macro sub_eax 1
      dw 0xE881
      dd %1
%endmacro
%macro cmp_al 1
      dw 0xF880
      db %1
%endmacro
%macro test_eax 1
      dw 0xC0F7
      dd %1
%endmacro
%macro inc_ebx 0
      dw 0xC3FF
%endmacro
