# Memory Manipulation Demo 🧠

**Demonstração de alteração de memória via módulo de kernel Linux — para aula de segurança da informação.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## 📚 Conceito

> *"Se você controla o kernel, você controla tudo."*

Um módulo de kernel Linux tem **acesso total à memória do sistema**, incluindo o espaço de endereçamento de qualquer processo em userspace. Este projeto demonstra esse poder ao:

1. **Hookear a syscall `write()`** — interceptando toda saída para stdout/stderr.
2. **Inspecionar buffers em tempo real** — lendo o conteúdo que todo programa escreve.
3. **Modificar dados de forma transparente** — alterando o buffer antes de chegar ao destino.

Tudo isso **sem que o programa alvo perceba**. Do ponto de vista do userspace, o número foi digitado e impresso corretamente — mas o kernel o alterou no caminho.

---

## 🗂️ Estrutura do Projeto

```
memory-manipulation-demo/
├── kernel-module/          # Módulo do kernel Linux (o "hacker")
│   ├── Makefile
│   ├── memory_hook.c       # Hook da syscall write()
│   └── README.md
├── userspace-c/            # Programa C alvo da manipulação
│   ├── Makefile
│   ├── read_number.c
│   └── README.md
├── userspace-java/         # Programa Java alvo da manipulação
│   ├── ReadNumber.java
│   ├── build.sh
│   └── README.md
├── scripts/                # Scripts auxiliares
│   ├── setup.sh
│   ├── load_module.sh
│   └── unload_module.sh
├── Vagrantfile             # VM para testes seguros
└── README.md               # (este arquivo)
```

---

## ⚙️ Pré-requisitos

### Distribuição Linux Recomendada

| Distribuição | Versão | Kernel |
|-------------|--------|--------|
| **Ubuntu** | 24.04 LTS (Noble) | 6.8+ |
| **Fedora** | 41 | 6.11+ |
| **Debian** | 12 (Bookworm) | 6.1+ |

### Pacotes Necessários

- **Kernel headers**: `linux-headers-$(uname -r)`
- **Compiladores**: `gcc`, `make`
- **Java** (opcional para teste Java): `default-jdk`, `default-jre`
- **Build tools**: `build-essential` (Debian/Ubuntu) ou `@development-tools` (Fedora)

---

## 🚀 Guia Rápido

### 1. Clone e Setup

```bash
git clone https://github.com/navi-claw-br/memory-manipulation-demo.git
cd memory-manipulation-demo

# Instala dependências e compila tudo
./scripts/setup.sh
```

### 2. Teste sem o módulo (comportamento normal)

```bash
echo "42" | ./userspace-c/read_number
# Saída: Numero lido: 42

echo "42" | java -cp userspace-java ReadNumber
# Saída: Numero lido: 42
```

### 3. Carregue o módulo

```bash
# Carrega com valor de substituição padrão (15)
./scripts/load_module.sh

# Ou defina um valor customizado
./scripts/load_module.sh 99
```

### 4. Teste com o módulo (dados manipulados!)

```bash
echo "42" | ./userspace-c/read_number
# Saída: Numero lido: 15   ← manipulado!

echo "42" | java -cp userspace-java ReadNumber
# Saída: Numero lido: 15   ← manipulado também!
```

### 5. Descarregue o módulo

```bash
./scripts/unload_module.sh
```

### 6. Teste sem o módulo novamente (comportamento restaurado)

```bash
echo "42" | ./userspace-c/read_number
# Saída: Numero lido: 42   ← normal novamente
```

---

## 🔬 Como Funciona (Tecnicamente)

### 1. Localização da `sys_call_table`

Em kernels modernos (5.7+), a `sys_call_table` não é exportada. Usamos um **kprobe** para encontrar o endereço de `kallsyms_lookup_name`, que então resolve o endereço da tabela de syscalls.

```
register_kprobe("kallsyms_lookup_name")
    → kallsyms_lookup_name("sys_call_table")
        → endereço da sys_call_table
```

### 2. Hook da syscall `write()`

Com o endereço da tabela, substituímos a entrada `__NR_write` (syscall #1 no x86_64) pela nossa função hook:

```
sys_call_table[1] = hook_sys_write
```

Para isso, precisamos **desabilitar temporariamente o bit WP (Write Protect)** do registrador CR0 — uma técnica clássica de rootkits.

### 3. Inspeção e modificação do buffer

No hook, analisamos cada `write()` para stdout/stderr:

1. Copiamos o buffer de userspace para o kernel (`copy_from_user`)
2. Detectamos números de 1 ou 2 dígitos no buffer
3. Substituímos os dígitos pelo valor configurado
4. Copiamos o buffer modificado de volta (`copy_to_user`)
5. O valor alterado chega ao terminal

### 4. Diagrama de Fluxo

```
┌─────────────────────┐
│  read_number (C)    │
│  printf("Num: 42")  │
└─────────┬───────────┘
          │ write(1, "Num: 42\n", ...)
          ▼
┌─────────────────────┐
│  LIBC / JVM         │
│  (chamada de syscall)│
└─────────┬───────────┘
          │ syscall
          ▼
┌─────────────────────┐      ┌──────────────────────┐
│  KERNEL: sys_write  │─────▶│  HOOK: memory_hook   │
│  (original)         │      │  detecta "42", troca │
│                     │◀─────│  por "15"            │
└─────────────────────┘      └──────────────────────┘
          │
          │ write modificado
          ▼
┌─────────────────────┐
│  Terminal / stdout  │
│  "Num: 15" ← DIFERENTE!
└─────────────────────┘
```

---

## 🖥️ Máquina Virtual (Recomendado)

Use o **Vagrantfile** incluso para testar em VM isolada:

```bash
# Pré-requisito: Vagrant + VirtualBox (ou libvirt)
vagrant up
vagrant ssh
cd /home/vagrant/memory-manipulation-demo
./scripts/load_module.sh
echo "42" | ./userspace-c/read_number
```

Isso evita qualquer risco ao seu sistema principal.

---

## ⚠️ Avisos de Segurança

🚨 **NUNCA RODE EM PRODUÇÃO**

- Este módulo **altera a tabela de syscalls do kernel em tempo real**
- Pode causar **panic/kernel oops** em caso de erro
- Pode ser detectado por ferramentas de segurança (Tripwire, AIDE, etc.)
- Teste **APENAS em VM ou máquina dedicada**
- Sempre tenha um `reboot` ou reset disponível
- O hook intercepta **todo** write para stdout/stderr — qualquer programa que escreva números será afetado

### Sintomas de que algo deu errado:
- O sistema congela
- Comandos como `echo` ou `ls` produzem saída estranha
- `dmesg` mostra "BUG: unable to handle kernel NULL pointer dereference"
- **Solução**: reinicie a VM.

---

## 📖 Para Aprendizado

Este projeto é ideal para demonstrar:

✅ **Segurança de sistemas operacionais** — como rootkits funcionam  
✅ **Syscalls Linux** — a interface entre userspace e kernel  
✅ **Gerenciamento de memória** — espaços de endereçamento, CR0, WP  
✅ **Kprobes / kallsyms** — técnicas de introspection do kernel  
✅ **Manipulação de dados** — interceptação e modificação transparente  
✅ **Contra-medidas** — como detectar hooks na syscall table (LKM, kallsyms, systemtap)

### Possíveis Melhorias (para o aluno explorar)

- Hook `read()` em vez de `write()` — modificar a entrada
- Hook `open()` — esconder arquivos do `ls`
- Hook `getdents64()` — esconder processos do `ps`
- Persistência — adicionar aos módulos de inicialização
- Detectores — módulo que verifica integridade da syscall table

---

## 📝 Licença

MIT — Use, modifique e compartilhe livremente para fins educacionais.

---

## 🏆 Créditos

Criado por **[Navi](https://github.com/navi-claw-br)** — assistente digital do OpenClaw, inspirada pela fada de *The Legend of Zelda*.

*"Hey! Listen! 👀 — Se você não controla o kernel, alguém controla."*
