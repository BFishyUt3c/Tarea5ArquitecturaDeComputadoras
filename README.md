# Simulador RISC-V (CS3051 - HW5)

Simulador del ISA base RV32I de 32 bits, implementado en C++17.
Carga un programa binario crudo en la direccion `0x00000000`, lo ejecuta
paso a paso y permite inspeccionar el PC, los registros y la memoria.

## Compilacion

```bash
cmake -B build
cmake --build build
```

O directamente con g++:

```bash
g++ -std=c++17 -O2 -I src src/main.cpp src/cpu/cpu.cpp src/mem/mem.cpp src/loader/loader.cpp -o riscv-sim
```

## Arquitectura del codigo

- **`src/cpu/`** — Estado arquitectural: PC de 32 bits y 32 registros (`x0`
  fijado en cero). Implementa el ciclo fetch-decode-execute en `step()`:
  lee la instruccion apuntada por el PC, la decodifica (opcode, funct3,
  funct7, registros e inmediatos), ejecuta su comportamiento y actualiza
  el PC, los registros y la memoria. Incluye las 37 instrucciones del
  Anexo A y reporta un error explicito ante un opcode desconocido.
- **`src/mem/`** — Memoria plana *byte-addressable* con espacio de
  direcciones de 32 bits, en formato little-endian. Provee lecturas y
  escrituras de 1, 2 y 4 bytes (`read_byte`, `read_half`, `read_word`, etc.).
- **`src/loader/`** — Lee un archivo binario crudo y copia sus bytes a
  memoria a partir de la direccion `0x00000000`.

## Ejecucion

```bash
./riscv-sim tests/riscvtest.bin
```

El simulador muestra un prompt `>` listo para recibir comandos.

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

## Formato little-endian

RISC-V guarda el byte menos significativo en la direccion mas baja. Por
eso el valor `25` (`0x00000019`) aparece en memoria como:

```
0x64: 19 00 00 00
```

Es decir: `0x19` (byte bajo) en `0x64`, y los tres bytes altos en cero.

## Inspeccion en distintos puntos de ejecucion

Ademas del resultado final, se puede seguir como cambia la memoria
intercalando `step` y `mem`:

```
$ ./riscv-sim tests/quicksort.bin
> step
pc=0x4
> step
pc=0x8
> mem 0x1000 0x1003
0x1000: 00 00 00 00
0x1004: 00 00 00 00
> step
pc=0xc
> mem 0x1000 0x1003
0x1000: 06 00 00 00
0x1004: 00 00 00 00
> exit
```

Aqui se ve como el primer `sw` escribe el valor `6` en `0x1000`.

## Requisitos obligatorios cumplidos

- ISA base RV32I de 32 bits (las 37 instrucciones del Anexo A): cargas
  (`lb`, `lh`, `lw`, `lbu`, `lhu`), ALU inmediata y de registro, stores
  (`sb`, `sh`, `sw`), branches, jumps (`jal`, `jalr`) y `lui`/`auipc`.
- Memoria little-endian de 32 bits.
- Carga de binarios crudos en `0x00000000`.
- Ejecucion paso a paso (`step`).
- Inspeccion de PC, registros y memoria.
- Reporte explicito de instrucciones invalidas.

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

Nota: los programas terminan en un bucle infinito, por eso `run` se detiene
por el limite de pasos; el resultado ya esta calculado antes de eso.

`tests/asm.py` es un ensamblador minimo que genera los binarios a partir
de archivos `.s` (no es parte del simulador; solo sirve para regenerar los
binarios de prueba).
