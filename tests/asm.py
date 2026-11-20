#!/usr/bin/env python3
"""Ensamblador RISC-V minimo para generar los binarios de prueba."""
import re, struct, sys

REGS = {f'x{i}': i for i in range(32)}
REGS.update({'zero':0,'ra':1,'sp':2,'t0':5,'t1':6,'t2':7,
             's0':8,'s1':9,'a0':10,'a1':11,'a2':12,'a3':13,'a4':14,'a5':15,
             's2':18,'s3':19,'t3':28,'t4':29,'t5':30,'t6':31})

def rtype(f7, rs2, rs1, f3, rd, op):
    return (f7<<25)|(rs2<<20)|(rs1<<15)|(f3<<12)|(rd<<7)|op
def itype(imm, rs1, f3, rd, op):
    return (imm<<20)|(rs1<<15)|(f3<<12)|(rd<<7)|op
def stype(imm, rs2, rs1, f3, op):
    return (((imm>>5)&0x3F)<<25)|(rs2<<20)|(rs1<<15)|(f3<<12)|((imm&0x1F)<<7)|op
def btype(imm, rs2, rs1, f3, op):
    return (((imm>>12)&1)<<31)|(((imm>>11)&1)<<7)|(((imm>>5)&0x3F)<<25)|(((imm>>1)&0xF)<<8)|(rs2<<20)|(rs1<<15)|(f3<<12)|op
def jtype(imm, rd, op):
    return (((imm>>20)&1)<<31)|(((imm>>12)&0xFF)<<12)|(((imm>>11)&1)<<20)|(((imm>>1)&0x3FF)<<21)|(rd<<7)|op
def utype(imm, rd, op):
    return (imm<<12)|(rd<<7)|op

def num(s):
    s = s.strip()
    if s.startswith('-'): return -num(s[1:])
    return int(s, 0)

def parse(lines):
    prog, labels, addr = [], {}, 0
    for raw in lines:
        line = raw.split('#')[0].strip()
        if not line or line.startswith('.'): continue
        m = re.match(r'^(\w+):', line)
        if m:
            labels[m.group(1)] = addr
            line = line[m.end():].strip()
            if not line: continue
        prog.append((addr, line)); addr += 4
    return prog, labels

def assemble(prog, labels):
    out = []
    for addr, line in prog:
        p = [x.strip() for x in line.replace(',', ' ').split()]
        op = p[0].lower(); args = p[1:]
        R = lambda x: REGS[x.lower()]
        def imm_of(s):
            if s == '.': return -4
            return labels[s]-(addr+4) if s in labels else num(s)
        if op == 'li':
            rd, v = R(args[0]), num(args[1])
            if -2048 <= v < 2048: out.append(itype(v & 0xFFF, 0, 0, rd, 0x13))
            else:
                out.append(utype((v >> 12) & 0xFFFFF, rd, 0x37))
                out.append(itype(v & 0xFFF, rd, 0, rd, 0x13))
        elif op == 'mv':
            out.append(itype(0, R(args[1]), 0, R(args[0]), 0x13))
        elif op == 'addi':
            out.append(itype(num(args[2]) & 0xFFF, R(args[1]), 0, R(args[0]), 0x13))
        elif op in ('add','sub','and','or','xor','sll','srl','sra','slt','sltu'):
            f3 = dict(add=0,sub=0,sll=1,slt=2,sltu=3,xor=4,srl=5,sra=5,orr=6,andd=7)
            key = {'or':'orr','and':'andd'}.get(op, op)
            f7 = 0x20 if op in ('sub','sra') else 0
            out.append(rtype(f7, R(args[2]), R(args[1]), f3[key], R(args[0]), 0x33))
        elif op == 'slli':
            out.append(itype(num(args[2]) & 0x1F, R(args[1]), 1, R(args[0]), 0x13))
        elif op in ('lw','lh','lb','lbu','lhu'):
            f3 = dict(lb=0,lh=1,lw=2,lbu=4,lhu=5)[op]
            m = re.match(r'(-?\d+)\((\w+)\)', args[1])
            out.append(itype(num(m.group(1)) & 0xFFF, R(m.group(2)), f3, R(args[0]), 0x03))
        elif op in ('sw','sh','sb'):
            f3 = dict(sb=0,sh=1,sw=2)[op]
            m = re.match(r'(-?\d+)\((\w+)\)', args[1])
            out.append(stype(num(m.group(1)), R(args[0]), R(m.group(2)), f3, 0x23))
        elif op in ('beq','bne','blt','bge','bltu','bgeu'):
            f3 = dict(beq=0,bne=1,blt=4,bge=5,bltu=6,bgeu=7)[op]
            out.append(btype(imm_of(args[2]), R(args[1]), R(args[0]), f3, 0x63))
        elif op == 'j':
            out.append(jtype(imm_of(args[0]), 0, 0x6F))
        elif op == 'jal':
            out.append(jtype(imm_of(args[1]), R(args[0]), 0x6F))
        elif op == 'call':
            out.append(jtype(imm_of(args[0]), 1, 0x6F))
        elif op == 'ret':
            out.append(itype(0, 1, 0, 0, 0x67))
        else:
            raise ValueError(f'No soportada: {line}')
    return out

def main():
    prog, labels = parse(open(sys.argv[1]).read().splitlines())
    code = assemble(prog, labels)
    with open(sys.argv[2], 'wb') as f:
        f.write(b''.join(struct.pack('<I', c & 0xFFFFFFFF) for c in code))
    print(f'{len(code)} instrucciones -> {sys.argv[2]}')

if __name__ == '__main__':
    main()
