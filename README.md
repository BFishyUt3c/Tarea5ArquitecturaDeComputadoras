# Simulador RISC-V (CS3051 - HW5)

Simulador del ISA base RV32I de 32 bits, implementado en C++17.

## Compilacion

```bash
cmake -B build
cmake --build build
```

O directamente con g++:

```bash
g++ -std=c++17 -O2 -I src src/main.cpp src/cpu/cpu.cpp src/mem/mem.cpp src/loader/loader.cpp -o riscv-sim
```

## Ejecucion

```bash
./riscv-sim tests/riscvtest.bin
```

El simulador carga el archivo binario crudo en la direccion `0x00000000` y muestra un prompt `>`.

## Comandos

| Comando | Descripcion |
|---|---|
| `pc` | Muestra el contador de programa. |
| `step` | Ejecuta una instruccion. |
| `regs` | Muestra los 32 registros. |
| `regs x5 x14` | Muestra registros especificos. |
| `mem <ini> <fin>` | Muestra memoria (direcciones en hexadecimal). |
| `run` | Ejecuta hasta detenerse (maximo 100000 pasos). |
| `exit` | Sale del simulador. |

## Programas de prueba

En `tests/` se incluyen tres binarios:

| Archivo | Resultado esperado |
|---|---|
| `riscvtest.bin` | `mem 0x64 0x67` -> `19 00 00 00` (25) |
| `quicksort.bin` | `mem 0x1000 0x101b` -> `01 02 03 04 06 08 09` |
| `symmetric.bin` | `regs x10` -> `0x1` |

Ejemplo de sesion:

```
$ ./riscv-sim tests/riscvtest.bin
"tests/riscvtest.bin" cargado a memoria.
> run
Ejecutadas 100000 instrucciones. pc=0x50
Limite de pasos alcanzado.
> mem 0x64 0x67
0x64: 19 00 00 00
> exit
See you next time...
```

Nota: los programas terminan en un bucle infinito, por eso `run` se detiene por el limite de pasos; el resultado ya esta calculado antes de eso.

`tests/asm.py` es un ensamblador minimo que genera los binarios a partir de archivos `.s` (no es parte del simulador; solo sirve para regenerar los binarios de prueba).
